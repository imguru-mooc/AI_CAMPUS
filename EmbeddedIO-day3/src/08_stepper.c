/* 08_stepper.c — 28BYJ-48 스테퍼 + ULN2003 드라이버, 하프 스텝 · 가감속
 * 배선: ULN2003 IN1 → GPIO12, IN2 → GPIO16, IN3 → GPIO20, IN4 → GPIO21, + → 5V, − → GND
 *       (6교시는 L293D 를 빼고 같은 GPIO20 · 21 에 꽂는다)
 * 실행: ./08_stepper 90        시계 방향 90°
 *       ./08_stepper -360      반시계 한 바퀴
 *       ./08_stepper 360 0     가감속 없이 (탈조 비교)
 */
#include "gpio_common.h"
#include <stdlib.h>

#define STEPS_PER_REV 4096                 /* 하프 스텝 기준 (실측 약 4076) */
#define FAST_US 1200                       /* 최고 속도: 스텝 간격 1.2 ms */
#define SLOW_US 4000                       /* 출발 · 정지 속도: 4 ms */
#define RAMP    200                        /* 가속 · 감속에 쓰는 스텝 수 */

static const unsigned int COILS[4] = { 12, 16, 20, 21 };
/* 하프 스텝 8단계: 코일 하나 → 둘 → 하나 … (A, AB, B, BC, C, CD, D, DA) */
static const int HALF[8][4] = {
    {1,0,0,0},{1,1,0,0},{0,1,0,0},{0,1,1,0},{0,0,1,0},{0,0,1,1},{0,0,0,1},{1,0,0,1},
};

static void ts_add(struct timespec *t, long ns)
{
    t->tv_nsec += ns;
    while (t->tv_nsec >= 1000000000L) { t->tv_nsec -= 1000000000L; t->tv_sec++; }
}

int main(int argc, char **argv)
{
    int deg = (argc > 1) ? atoi(argv[1]) : 90;
    int ramp = !(argc > 2 && atoi(argv[2]) == 0);
    gpio_install_sigint();
    struct gpiod_line_request *req = gpio_request_output("stepper", COILS, 4);
    if (!req) return EXIT_FAILURE;

    long steps = labs((long)deg) * STEPS_PER_REV / 360;
    int dir = deg >= 0 ? 1 : -1;
    printf("%d° = 하프 스텝 %ld 개, %s\n", deg, steps, ramp ? "가감속" : "일정 속도");

    struct timespec next;
    clock_gettime(CLOCK_MONOTONIC, &next);
    int phase = 0;
    long long t0 = now_ns();
    for (long i = 0; i < steps && !g_stop; i++) {
        phase = (phase + dir + 8) % 8;
        enum gpiod_line_value v[4];
        for (int c = 0; c < 4; c++) v[c] = HALF[phase][c] ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE;
        gpiod_line_request_set_values(req, v);

        /* 남은 스텝과 지난 스텝 중 가까운 쪽으로 속도를 정한다 (사다리꼴) */
        long us = FAST_US;
        if (ramp) {
            long edge = i < steps - 1 - i ? i : steps - 1 - i;
            if (edge < RAMP) us = SLOW_US - (SLOW_US - FAST_US) * edge / RAMP;
        }
        ts_add(&next, us * 1000);          /* 절대 시각 → 간격이 일정하게 유지된다 */
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);
    }
    printf("완료 %.2f 초\n", (now_ns() - t0) / 1e9);

    enum gpiod_line_value off[4] = { 0, 0, 0, 0 };   /* 코일 전류 끄기 — 열 · 전력 절약 */
    gpiod_line_request_set_values(req, off);
    gpiod_line_request_release(req);
    return EXIT_SUCCESS;
}
