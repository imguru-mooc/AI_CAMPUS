/* 09_sonar.c — HC-SR04 초음파 거리 측정 (커널 엣지 타임스탬프로 Echo 폭 재기)
 * 배선: VCC → 5V, GND → GND, Trig → GPIO23 (물리 16)
 *       Echo → 1kΩ → GPIO24 (물리 18),  GPIO24 → 1kΩ → 1kΩ → GND (2kΩ)   ← 분압 필수 (5V → 3.3V)
 * 실행: ./09_sonar
 */
#include "gpio_common.h"
#include <poll.h>
#include <stdlib.h>

#define TRIG 23
#define ECHO 24

static struct gpiod_line_request *g_trig, *g_echo;
static struct gpiod_edge_event_buffer *g_buf;

/* 한 번 측정해 cm 를 돌려준다. 실패(시간 초과)면 -1 */
static double measure_cm(void)
{
    struct pollfd p = { .fd = gpiod_line_request_get_fd(g_echo), .events = POLLIN };
    while (poll(&p, 1, 0) > 0)                     /* 지난 측정의 찌꺼기 버리기 */
        gpiod_line_request_read_edge_events(g_echo, g_buf, 8);

    unsigned int t = TRIG;
    gpiod_line_request_set_value(g_trig, t, GPIOD_LINE_VALUE_ACTIVE);
    struct timespec ts = { 0, 10000 };             /* 10 µs 이상 High (길어져도 괜찮다) */
    nanosleep(&ts, NULL);
    gpiod_line_request_set_value(g_trig, t, GPIOD_LINE_VALUE_INACTIVE);

    long long rise = 0, fall = 0;
    long long deadline = now_ns() + 60000000LL;     /* 60 ms 안에 끝나야 한다 */
    while (!fall) {
        int left = (int)((deadline - now_ns()) / 1000000);
        if (left <= 0 || poll(&p, 1, left) <= 0) return -1;
        int n = gpiod_line_request_read_edge_events(g_echo, g_buf, 8);
        for (int i = 0; i < n; i++) {
            struct gpiod_edge_event *ev = gpiod_edge_event_buffer_get_event(g_buf, i);
            long long tsn = (long long)gpiod_edge_event_get_timestamp_ns(ev);
            if (gpiod_edge_event_get_event_type(ev) == GPIOD_EDGE_EVENT_RISING_EDGE) rise = tsn;
            else if (rise) { fall = tsn; break; }
        }
    }
    double us = (fall - rise) / 1000.0;            /* Echo High 폭 = 왕복 시간 */
    if (us > 30000) return -1;                     /* 약 5 m 이상 = 감지 실패 */
    return us * 0.0343 / 2;                        /* 소리 343 m/s → 0.0343 cm/µs, 왕복이라 ÷2 */
}

static int cmp(const void *a, const void *b)
{
    double x = *(const double *)a, y = *(const double *)b;
    return (x > y) - (x < y);
}

/* 5번 재서 중앙값 — 튀는 값 하나에 흔들리지 않는다 */
static double median_cm(void)
{
    double v[5]; int n = 0;
    for (int i = 0; i < 5 && !g_stop; i++) {
        double d = measure_cm();
        if (d > 0) v[n++] = d;
        sleep_ms(60);                              /* 메아리가 사라질 시간 */
    }
    if (n == 0) return -1;
    qsort(v, (size_t)n, sizeof v[0], cmp);
    return v[n / 2];
}

int main(void)
{
    unsigned int trig = TRIG, echo = ECHO;
    gpio_install_sigint();
    g_trig = gpio_request_output("sonar", &trig, 1);
    struct gpiod_line_settings *s = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(s, GPIOD_LINE_DIRECTION_INPUT);
    gpiod_line_settings_set_edge_detection(s, GPIOD_LINE_EDGE_BOTH);
    gpiod_line_settings_set_bias(s, GPIOD_LINE_BIAS_DISABLED);   /* 센서가 직접 0/1 */
    g_echo = gpio_request("sonar", &echo, 1, s);
    gpiod_line_settings_free(s);
    g_buf = gpiod_edge_event_buffer_new(8);
    if (!g_trig || !g_echo || !g_buf) return EXIT_FAILURE;

    printf("거리 측정 (중앙값 5회) — Ctrl+C 로 종료\n");
    while (!g_stop) {
        double d = median_cm();
        if (d < 0) printf("  측정 실패 (너무 멀거나 각도가 비스듬함)\n");
        else {
            printf("  %6.1f cm  |", d);
            for (int k = 0; k < (int)(d / 5) && k < 60; k++) putchar('=');
            putchar('\n');
        }
    }
    gpiod_edge_event_buffer_free(g_buf);
    gpiod_line_request_release(g_echo);
    gpiod_line_request_release(g_trig);
    return EXIT_SUCCESS;
}
