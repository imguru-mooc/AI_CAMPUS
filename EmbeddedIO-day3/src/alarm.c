/* alarm.c — 미니 프로젝트: 초음파 거리 경보기
 *
 *   측정 쓰레드 : HC-SR04 를 3번 재서 중앙값 → 공유 변수 g_cm (mutex)
 *   메인 루프   : 20 ms 마다 g_cm 을 읽어
 *                 LED  — 50 cm 이상 초록 · 20~50 노랑 · 20 미만 빨강
 *                 부저 — 50 cm 미만이면 삑삑, 가까울수록 간격이 짧게 / 10 cm 미만 연속음
 *                 서보 — 10 cm 미만이면 차단기 내림(0°), 아니면 올림(90°)
 *
 * 배선: HC-SR04 Trig GPIO23 · Echo GPIO24(분압) / LED GPIO17 빨강 · 27 노랑 · 22 초록
 *       서보 신호 GPIO18 (PWM0) / 패시브 부저 GPIO19 (PWM1)
 * 실행: ./alarm
 */
#include "gpio_common.h"
#include "pwm_sysfs.h"
#include <poll.h>
#include <pthread.h>
#include <stdlib.h>

#define TRIG 23
#define ECHO 24
static const unsigned int LEDS[3] = { 17, 27, 22 };      /* R Y G */

static struct gpiod_line_request *g_trig, *g_echo, *g_led;
static struct gpiod_edge_event_buffer *g_buf;
static double g_cm = -1;                                /* 공유: 측정 쓰레드가 쓰고 메인이 읽는다 */
static pthread_mutex_t g_mx = PTHREAD_MUTEX_INITIALIZER;

static double measure_cm(void)                          /* 09_sonar.c 와 같은 함수 */
{
    struct pollfd p = { .fd = gpiod_line_request_get_fd(g_echo), .events = POLLIN };
    while (poll(&p, 1, 0) > 0) gpiod_line_request_read_edge_events(g_echo, g_buf, 8);
    gpiod_line_request_set_value(g_trig, TRIG, GPIOD_LINE_VALUE_ACTIVE);
    struct timespec ts = { 0, 10000 };
    nanosleep(&ts, NULL);
    gpiod_line_request_set_value(g_trig, TRIG, GPIOD_LINE_VALUE_INACTIVE);
    long long rise = 0, fall = 0, deadline = now_ns() + 60000000LL;
    while (!fall) {
        int left = (int)((deadline - now_ns()) / 1000000);
        if (left <= 0 || poll(&p, 1, left) <= 0) return -1;
        int n = gpiod_line_request_read_edge_events(g_echo, g_buf, 8);
        for (int i = 0; i < n; i++) {
            struct gpiod_edge_event *ev = gpiod_edge_event_buffer_get_event(g_buf, i);
            long long t = (long long)gpiod_edge_event_get_timestamp_ns(ev);
            if (gpiod_edge_event_get_event_type(ev) == GPIOD_EDGE_EVENT_RISING_EDGE) rise = t;
            else if (rise) { fall = t; break; }
        }
    }
    double us = (fall - rise) / 1000.0;
    return us > 30000 ? -1 : us * 0.0343 / 2;
}

static void *sensor_thread(void *arg)
{
    (void)arg;
    while (!g_stop) {
        double v[3]; int n = 0;
        for (int i = 0; i < 3 && !g_stop; i++) { double d = measure_cm(); if (d > 0) v[n++] = d; sleep_ms(60); }
        double m = -1;
        if (n == 3) {                                   /* 3개 중앙값 */
            double a = v[0], b = v[1], c = v[2];
            m = (a > b) ? ((b > c) ? b : (a > c ? c : a)) : ((a > c) ? a : (b > c ? c : b));
        } else if (n > 0) m = v[0];
        pthread_mutex_lock(&g_mx);
        g_cm = m;
        pthread_mutex_unlock(&g_mx);
    }
    return NULL;
}

static long servo_ns(int deg) { return (500 + 2000L * deg / 180) * 1000; }

int main(void)
{
    unsigned int trig = TRIG, echo = ECHO;
    gpio_install_sigint();
    g_trig = gpio_request_output("alarm", &trig, 1);
    g_led  = gpio_request_output("alarm", LEDS, 3);
    struct gpiod_line_settings *s = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(s, GPIOD_LINE_DIRECTION_INPUT);
    gpiod_line_settings_set_edge_detection(s, GPIOD_LINE_EDGE_BOTH);
    gpiod_line_settings_set_bias(s, GPIOD_LINE_BIAS_DISABLED);
    g_echo = gpio_request("alarm", &echo, 1, s);
    gpiod_line_settings_free(s);
    g_buf = gpiod_edge_event_buffer_new(8);
    pwm_t servo, buz;
    if (!g_trig || !g_led || !g_echo || !g_buf || pwm_open(&servo, 0, 0) < 0 || pwm_open(&buz, 0, 1) < 0)
        return EXIT_FAILURE;
    pwm_set(&servo, 20000000L, servo_ns(90)); pwm_enable(&servo, 1);
    pwm_set(&buz, 1000000000L / 2000, 1000000000L / 4000);   /* 2 kHz, 듀티 50% */

    pthread_t th;
    pthread_create(&th, NULL, sensor_thread, NULL);
    printf("거리 경보기 — Ctrl+C 로 종료\n");

    long long last_beep = 0, last_print = 0;
    int buz_on = 0, gate_down = 0;
    while (!g_stop) {
        pthread_mutex_lock(&g_mx);
        double d = g_cm;
        pthread_mutex_unlock(&g_mx);
        long long now = now_ns();

        /* LED 세 단계 */
        int zone = (d < 0 || d >= 50) ? 2 : (d >= 20 ? 1 : 0);    /* 2 초록 · 1 노랑 · 0 빨강 */
        enum gpiod_line_value v[3] = { zone == 0, zone == 1, zone == 2 };
        gpiod_line_request_set_values(g_led, v);

        /* 부저: 간격 = 거리 × 10 ms (50 cm → 0.5초, 20 cm → 0.2초), 삑 길이 50 ms */
        int want;
        if (d > 0 && d < 10) want = 1;
        else if (d > 0 && d < 50) {
            long gap = (long)(d * 10) * 1000000LL;
            if (now - last_beep >= gap) last_beep = now;
            want = (now - last_beep) < 50000000LL;
        } else want = 0;
        if (want != buz_on) { pwm_enable(&buz, want); buz_on = want; }

        /* 서보 차단기 */
        int down = d > 0 && d < 10;
        if (down != gate_down) { pwm_set_duty(&servo, servo_ns(down ? 0 : 90)); gate_down = down; }

        if (now - last_print > 500000000LL) {
            last_print = now;
            if (d < 0) printf("  ---.- cm  측정 없음\n");
            else printf("  %5.1f cm  %s%s\n", d, zone == 0 ? "빨강" : zone == 1 ? "노랑" : "초록",
                        down ? "  ▼ 차단기 내림" : "");
        }
        sleep_ms(20);
    }

    pthread_join(th, NULL);
    enum gpiod_line_value off[3] = { 0, 0, 0 };
    gpiod_line_request_set_values(g_led, off);
    pwm_close(&buz);
    pwm_close(&servo);
    gpiod_edge_event_buffer_free(g_buf);
    gpiod_line_request_release(g_echo);
    gpiod_line_request_release(g_trig);
    gpiod_line_request_release(g_led);
    printf("\n종료\n");
    return EXIT_SUCCESS;
}
