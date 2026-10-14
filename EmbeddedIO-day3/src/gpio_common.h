/* gpio_common.h — Day 2~5 공통 도우미 (libgpiod v2)
 *
 *   gpio_install_sigint()  Ctrl+C 를 받으면 g_stop = 1 (poll·sleep 을 깨움)
 *   gpio_request_*()       라인 요청을 한 줄로 — 실패하면 NULL
 *   now_ns()               CLOCK_MONOTONIC 나노초 (edge event 타임스탬프와 같은 시계)
 *   sleep_ms()             Ctrl+C 로 깨어날 수 있는 대기
 *
 * 규칙: 요청한 라인은 프로그램이 끝나기 전에 gpiod_line_request_release() 로 반납한다.
 */
#ifndef GPIO_COMMON_H
#define GPIO_COMMON_H

#include <gpiod.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#define GPIO_CHIP "/dev/gpiochip0"

static volatile sig_atomic_t g_stop = 0;

static void on_sigint(int sig) { (void)sig; g_stop = 1; }

/* SA_RESTART 를 주지 않는다 → poll()/nanosleep() 이 EINTR 로 돌아와 루프가 끝난다 */
static inline void gpio_install_sigint(void)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = on_sigint;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

static inline long long now_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

static inline void sleep_ms(long ms)
{
    struct timespec ts = { ms / 1000, (ms % 1000) * 1000000L };
    nanosleep(&ts, NULL);            /* SIGINT 가 오면 남은 시간을 버리고 돌아온다 */
}

/* 공통 요청 함수: settings 하나를 offsets[0..n-1] 전부에 적용 */
static inline struct gpiod_line_request *
gpio_request(const char *consumer, const unsigned int *offsets, size_t n,
             struct gpiod_line_settings *settings)
{
    struct gpiod_chip *chip = NULL;
    struct gpiod_line_config *lcfg = NULL;
    struct gpiod_request_config *rcfg = NULL;
    struct gpiod_line_request *req = NULL;

    chip = gpiod_chip_open(GPIO_CHIP);
    if (!chip) { perror("gpiod_chip_open"); goto out; }

    lcfg = gpiod_line_config_new();
    if (!lcfg || gpiod_line_config_add_line_settings(lcfg, offsets, n, settings) < 0) {
        perror("line_config"); goto out;
    }
    rcfg = gpiod_request_config_new();
    if (!rcfg) { perror("request_config"); goto out; }
    gpiod_request_config_set_consumer(rcfg, consumer);

    req = gpiod_chip_request_lines(chip, rcfg, lcfg);
    if (!req)
        fprintf(stderr, "%s: 라인 요청 실패: %s\n", consumer, strerror(errno));
out:
    gpiod_request_config_free(rcfg);
    gpiod_line_config_free(lcfg);
    gpiod_chip_close(chip);          /* 요청은 칩을 닫아도 유지된다 */
    return req;
}

/* 출력 라인 요청 (초기값 inactive = 0) */
static inline struct gpiod_line_request *
gpio_request_output(const char *consumer, const unsigned int *offsets, size_t n)
{
    struct gpiod_line_settings *s = gpiod_line_settings_new();
    if (!s) return NULL;
    gpiod_line_settings_set_direction(s, GPIOD_LINE_DIRECTION_OUTPUT);
    gpiod_line_settings_set_output_value(s, GPIOD_LINE_VALUE_INACTIVE);
    struct gpiod_line_request *r = gpio_request(consumer, offsets, n, s);
    gpiod_line_settings_free(s);
    return r;
}

/* 입력 라인 요청: pull-up, edge 는 GPIOD_LINE_EDGE_NONE 이면 이벤트 없음 */
static inline struct gpiod_line_request *
gpio_request_input(const char *consumer, const unsigned int *offsets, size_t n,
                   enum gpiod_line_edge edge, unsigned long debounce_us)
{
    struct gpiod_line_settings *s = gpiod_line_settings_new();
    if (!s) return NULL;
    gpiod_line_settings_set_direction(s, GPIOD_LINE_DIRECTION_INPUT);
    gpiod_line_settings_set_bias(s, GPIOD_LINE_BIAS_PULL_UP);
    gpiod_line_settings_set_edge_detection(s, edge);
    if (debounce_us)
        gpiod_line_settings_set_debounce_period_us(s, debounce_us);
    struct gpiod_line_request *r = gpio_request(consumer, offsets, n, s);
    gpiod_line_settings_free(s);
    return r;
}

#endif
