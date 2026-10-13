/* 01_blink.c — libgpiod v2 로 LED 깜빡이기 (객체 흐름을 한 단계씩 직접 작성)
 * 배선: GPIO17 → 220Ω → LED → GND
 * 빌드: make 01_blink      실행: ./01_blink      종료: Ctrl+C
 */
#include <gpiod.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CHIP_PATH "/dev/gpiochip0"
#define LED_LINE  17

static volatile sig_atomic_t g_stop = 0;
static void on_sigint(int sig) { (void)sig; g_stop = 1; }

int main(void)
{
    unsigned int offset = LED_LINE;
    int ret = EXIT_FAILURE;

    /* Ctrl+C → g_stop = 1 → 루프 종료 → 아래에서 라인 반납 */
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = on_sigint;
    sigaction(SIGINT, &sa, NULL);

    /* ① 칩 열기 */
    struct gpiod_chip *chip = gpiod_chip_open(CHIP_PATH);
    if (!chip) { perror("gpiod_chip_open"); return EXIT_FAILURE; }

    /* ② 라인 하나에 줄 설정: 출력, 처음 값 0 */
    struct gpiod_line_settings *settings = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);
    gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_INACTIVE);

    /* ③ 어떤 라인(offset)에 그 설정을 쓸지 묶기 */
    struct gpiod_line_config *line_cfg = gpiod_line_config_new();
    gpiod_line_config_add_line_settings(line_cfg, &offset, 1, settings);

    /* ④ 요청 전체에 대한 설정: consumer 이름 (gpioinfo 에 보임) */
    struct gpiod_request_config *req_cfg = gpiod_request_config_new();
    gpiod_request_config_set_consumer(req_cfg, "blink");

    /* ⑤ 커널에 요청 → 성공하면 이 프로세스가 라인의 소유자 */
    struct gpiod_line_request *req = gpiod_chip_request_lines(chip, req_cfg, line_cfg);
    if (!req) { perror("gpiod_chip_request_lines"); goto cleanup; }

    printf("GPIO%d 깜빡임 시작 (Ctrl+C 로 종료)\n", LED_LINE);
    int on = 0;
    while (!g_stop) {
        on = !on;
        gpiod_line_request_set_value(req, offset,
            on ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE);
        struct timespec ts = { 0, 500 * 1000000L };      /* 0.5초 */
        nanosleep(&ts, NULL);
    }

    gpiod_line_request_set_value(req, offset, GPIOD_LINE_VALUE_INACTIVE);   /* 끄고 나가기 */
    gpiod_line_request_release(req);                                       /* ⑥ 반납 */
    printf("\n라인 반납 완료\n");
    ret = EXIT_SUCCESS;

cleanup:
    gpiod_request_config_free(req_cfg);
    gpiod_line_config_free(line_cfg);
    gpiod_line_settings_free(settings);
    gpiod_chip_close(chip);
    return ret;
}
