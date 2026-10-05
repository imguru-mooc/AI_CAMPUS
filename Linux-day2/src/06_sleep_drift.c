/* 06_sleep_drift.c — 상대 sleep 과 절대 시각 sleep 의 누적 오차 비교
 * 빌드: make 06_sleep_drift
 * 실행: ./06_sleep_drift        (1ms 주기 × 1000번, 매 주기 0.2ms 일한다고 가정)
 */
#include <stdio.h>
#include <time.h>

#define PERIOD_NS 1000000L      /* 1 ms */
#define LOOPS     1000

static long long ns(struct timespec t) { return t.tv_sec * 1000000000LL + t.tv_nsec; }

static void work(void)           /* 0.2ms 동안 바쁘게 일하는 척 */
{
    struct timespec a, b;
    clock_gettime(CLOCK_MONOTONIC, &a);
    do {
        clock_gettime(CLOCK_MONOTONIC, &b);
    } while (ns(b) - ns(a) < 200000);
}

static void add_ns(struct timespec *t, long d)
{
    t->tv_nsec += d;
    while (t->tv_nsec >= 1000000000L) {
        t->tv_nsec -= 1000000000L;
        t->tv_sec++;
    }
}

int main(void)
{
    struct timespec start, end, next;
    struct timespec rel = { 0, PERIOD_NS };

    /* 1) 상대 sleep: "지금부터 1ms" — 일한 시간 + 깨어나는 지연이 매번 더해진다 */
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < LOOPS; i++) {
        work();
        nanosleep(&rel, NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("상대 nanosleep        : %8.1f ms (목표 %d ms, 오차 %+.1f ms)\n",
           (ns(end) - ns(start)) / 1e6, LOOPS, (ns(end) - ns(start)) / 1e6 - LOOPS);

    /* 2) 절대 sleep: "start + i×1ms 시각까지" — 오차가 쌓이지 않는다 */
    clock_gettime(CLOCK_MONOTONIC, &start);
    next = start;
    for (int i = 0; i < LOOPS; i++) {
        work();
        add_ns(&next, PERIOD_NS);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("절대 clock_nanosleep  : %8.1f ms (목표 %d ms, 오차 %+.1f ms)\n",
           (ns(end) - ns(start)) / 1e6, LOOPS, (ns(end) - ns(start)) / 1e6 - LOOPS);
    return 0;
}
