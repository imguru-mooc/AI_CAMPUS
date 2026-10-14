/* 02_rgb.c — RGB LED 3채널 소프트웨어 PWM (쓰레드 하나가 세 채널을 처리)
 * 배선: RGB LED(공통 음극) R → 220Ω → GPIO17, G → 220Ω → GPIO27, B → 220Ω → GPIO22, 공통 → GND
 * 실행: ./02_rgb               무지개 페이드
 *       ./02_rgb 255 80 0      주황 고정 (R G B, 0~255)
 *       ./02_rgb -f            SCHED_FIFO (setcap 필요)
 *
 * 방법: 주기(10 ms) 시작에 켤 채널을 모두 켜고, 꺼야 할 시각이 빠른 채널부터
 *       그 절대 시각까지 잠들었다 끈다 → 주기당 깨어남은 최대 4번
 */
#include "gpio_common.h"
#include <math.h>
#include <pthread.h>
#include <sched.h>
#include <stdlib.h>
#include <sys/mman.h>

#define PERIOD_NS 10000000L                /* 10 ms = 100 Hz */
static const unsigned int LINES[3] = { 17, 27, 22 };
static struct gpiod_line_request *g_req;
static volatile int g_rgb[3] = { 0, 0, 0 };   /* 0~255 */

static void ts_add(struct timespec *t, long ns)
{
    t->tv_nsec += ns;
    while (t->tv_nsec >= 1000000000L) { t->tv_nsec -= 1000000000L; t->tv_sec++; }
}

static void *rgb_thread(void *arg)
{
    (void)arg;
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    while (!g_stop) {
        long on[3]; int idx[3] = { 0, 1, 2 };
        for (int c = 0; c < 3; c++) on[c] = PERIOD_NS / 255 * g_rgb[c];
        /* 꺼질 시각이 빠른 순서로 정렬 (원소 3개라 단순 교환) */
        for (int a = 0; a < 2; a++)
            for (int b = a + 1; b < 3; b++)
                if (on[idx[b]] < on[idx[a]]) { int t = idx[a]; idx[a] = idx[b]; idx[b] = t; }

        enum gpiod_line_value v[3];
        for (int c = 0; c < 3; c++) v[c] = on[c] > 0 ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE;
        gpiod_line_request_set_values(g_req, v);           /* 주기 시작: 한꺼번에 켜기 */

        for (int k = 0; k < 3; k++) {
            int c = idx[k];
            if (on[c] <= 0 || on[c] >= PERIOD_NS) continue;
            struct timespec t = start;
            ts_add(&t, on[c]);
            clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
            gpiod_line_request_set_value(g_req, LINES[c], GPIOD_LINE_VALUE_INACTIVE);
        }
        ts_add(&start, PERIOD_NS);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &start, NULL);
    }
    return NULL;
}

/* 색상환(0~359°) → RGB */
static void hue_to_rgb(int h, int rgb[3])
{
    double x = 1 - fabs(fmod(h / 60.0, 2) - 1);
    double r = 0, g = 0, b = 0;
    if (h < 60) { r = 1; g = x; } else if (h < 120) { r = x; g = 1; }
    else if (h < 180) { g = 1; b = x; } else if (h < 240) { g = x; b = 1; }
    else if (h < 300) { r = x; b = 1; } else { r = 1; b = x; }
    rgb[0] = (int)(r * 255); rgb[1] = (int)(g * 255); rgb[2] = (int)(b * 255);
}

int main(int argc, char **argv)
{
    int fifo = 0, fixed = 0, n = 0, vals[3] = { 0 };
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-f")) fifo = 1;
        else if (n < 3) { vals[n++] = atoi(argv[i]); fixed = (n == 3); }
    }
    gpio_install_sigint();
    g_req = gpio_request_output("rgb", LINES, 3);
    if (!g_req) return EXIT_FAILURE;

    pthread_attr_t at;
    pthread_attr_init(&at);
    if (fifo) {
        if (mlockall(MCL_CURRENT | MCL_FUTURE) < 0) perror("mlockall (계속 진행)");
        struct sched_param sp = { .sched_priority = 50 };
        pthread_attr_setinheritsched(&at, PTHREAD_EXPLICIT_SCHED);
        pthread_attr_setschedpolicy(&at, SCHED_FIFO);
        pthread_attr_setschedparam(&at, &sp);
    }
    pthread_t th;
    int e = pthread_create(&th, &at, rgb_thread, NULL);
    if (e) { fprintf(stderr, "pthread_create: %s\n", strerror(e)); return EXIT_FAILURE; }

    if (fixed) {
        for (int c = 0; c < 3; c++) g_rgb[c] = vals[c] < 0 ? 0 : (vals[c] > 255 ? 255 : vals[c]);
        printf("RGB = (%d, %d, %d) — Ctrl+C 로 종료\n", g_rgb[0], g_rgb[1], g_rgb[2]);
        while (!g_stop) sleep_ms(100);
    } else {
        printf("무지개 페이드%s — Ctrl+C 로 종료\n", fifo ? " (SCHED_FIFO)" : "");
        for (int h = 0; !g_stop; h = (h + 1) % 360) {
            int c[3]; hue_to_rgb(h, c);
            g_rgb[0] = c[0]; g_rgb[1] = c[1]; g_rgb[2] = c[2];
            sleep_ms(15);
        }
    }

    pthread_join(th, NULL);
    enum gpiod_line_value off[3] = { 0, 0, 0 };
    gpiod_line_request_set_values(g_req, off);
    gpiod_line_request_release(g_req);
    printf("\n종료\n");
    return EXIT_SUCCESS;
}
