/* 01_swpwm.c — 쓰레드로 만드는 소프트웨어 PWM + 깨어나는 시각의 흔들림(jitter) 측정
 * 배선: GPIO17 → 220Ω → LED → GND (Day 1 그대로)
 * 실행: ./01_swpwm          듀티 20% 고정, 2초마다 jitter 히스토그램
 *       ./01_swpwm 70       듀티 70%
 *       ./01_swpwm -b       숨쉬기(듀티 0→100→0)
 *       ./01_swpwm -f       SCHED_FIFO 실시간 우선순위로 (2교시, setcap 필요)
 * 부하: 다른 창에서  yes > /dev/null &  를 코어 수만큼
 */
#include "gpio_common.h"
#include <pthread.h>
#include <sched.h>
#include <stdlib.h>
#include <sys/mman.h>

#define LED       17
#define PERIOD_NS 5000000L                 /* 5 ms = 200 Hz */

static struct gpiod_line_request *g_req;
static volatile int g_duty = 20;           /* % */
static unsigned int g_off = LED;

/* jitter 히스토그램: 목표 시각보다 얼마나 늦게 깨어났나 */
static const long EDGE_US[] = { 10, 50, 100, 500, 1000 };
static long g_hist[6], g_max_ns, g_cnt;
static long long g_sum_ns;
static pthread_mutex_t g_mx = PTHREAD_MUTEX_INITIALIZER;

static void record(long late_ns)
{
    int b = 0;
    while (b < 5 && late_ns >= EDGE_US[b] * 1000) b++;
    pthread_mutex_lock(&g_mx);
    g_hist[b]++; g_cnt++; g_sum_ns += late_ns;
    if (late_ns > g_max_ns) g_max_ns = late_ns;
    pthread_mutex_unlock(&g_mx);
}

static void ts_add(struct timespec *t, long ns)
{
    t->tv_nsec += ns;
    while (t->tv_nsec >= 1000000000L) { t->tv_nsec -= 1000000000L; t->tv_sec++; }
}

/* 절대 시각까지 잠들고, 얼마나 늦게 깨어났는지 기록 */
static void sleep_until(const struct timespec *t)
{
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, t, NULL);
    long long target = (long long)t->tv_sec * 1000000000LL + t->tv_nsec;
    record((long)(now_ns() - target));
}

static void *pwm_thread(void *arg)
{
    (void)arg;
    struct timespec next;
    clock_gettime(CLOCK_MONOTONIC, &next);
    while (!g_stop) {
        long on_ns = PERIOD_NS / 100 * g_duty;
        struct timespec start = next;
        if (on_ns > 0) gpiod_line_request_set_value(g_req, g_off, GPIOD_LINE_VALUE_ACTIVE);
        if (on_ns < PERIOD_NS) {
            struct timespec t_off = start;
            ts_add(&t_off, on_ns);
            if (on_ns > 0) sleep_until(&t_off);
            gpiod_line_request_set_value(g_req, g_off, GPIOD_LINE_VALUE_INACTIVE);
        }
        ts_add(&next, PERIOD_NS);           /* 다음 주기 시작 = 이전 시작 + 주기 (누적 오차 없음) */
        sleep_until(&next);
    }
    return NULL;
}

int main(int argc, char **argv)
{
    int breathe = 0, fifo = 0;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-b")) breathe = 1;
        else if (!strcmp(argv[i], "-f")) fifo = 1;
        else g_duty = atoi(argv[i]) < 0 ? 0 : (atoi(argv[i]) > 100 ? 100 : atoi(argv[i]));
    }
    gpio_install_sigint();
    g_req = gpio_request_output("swpwm", &g_off, 1);
    if (!g_req) return EXIT_FAILURE;

    pthread_attr_t at;
    pthread_attr_init(&at);
    if (fifo) {
        /* 페이지 폴트로 멈추지 않게 메모리를 묶고, 실시간 우선순위 50 으로 */
        if (mlockall(MCL_CURRENT | MCL_FUTURE) < 0) perror("mlockall (계속 진행)");
        struct sched_param sp = { .sched_priority = 50 };
        pthread_attr_setinheritsched(&at, PTHREAD_EXPLICIT_SCHED);
        pthread_attr_setschedpolicy(&at, SCHED_FIFO);
        pthread_attr_setschedparam(&at, &sp);
    }
    pthread_t th;
    int e = pthread_create(&th, &at, pwm_thread, NULL);
    if (e) {
        fprintf(stderr, "pthread_create: %s%s\n", strerror(e),
                e == EPERM ? " — sudo setcap cap_sys_nice,cap_ipc_lock+ep ./01_swpwm" : "");
        return EXIT_FAILURE;
    }
    printf("SW PWM 200 Hz, %s%s — Ctrl+C 로 종료\n",
           breathe ? "숨쉬기" : "듀티 고정", fifo ? ", SCHED_FIFO 50" : ", SCHED_OTHER");

    int dir = 1;
    for (int t = 0; !g_stop; t++) {
        if (breathe) {                       /* 20 ms 마다 1% 씩 */
            g_duty += dir;
            if (g_duty >= 100 || g_duty <= 0) dir = -dir;
            sleep_ms(20);
        } else sleep_ms(20);
        if (t % 100 == 99) {                 /* 2초마다 히스토그램 */
            pthread_mutex_lock(&g_mx);
            printf("\n깨어난 지연 (%ld 회, 평균 %lld µs, 최대 %ld µs)\n", g_cnt,
                   g_cnt ? g_sum_ns / g_cnt / 1000 : 0, g_max_ns / 1000);
            const char *lab[6] = { "<10µs", "<50µs", "<100µs", "<500µs", "<1ms", "≥1ms" };
            for (int b = 0; b < 6; b++) {
                int bar = g_cnt ? (int)(g_hist[b] * 40 / g_cnt) : 0;
                printf("  %-7s %6ld |", lab[b], g_hist[b]);
                for (int k = 0; k < bar; k++) putchar('#');
                putchar('\n');
            }
            memset(g_hist, 0, sizeof g_hist); g_cnt = 0; g_sum_ns = 0; g_max_ns = 0;
            pthread_mutex_unlock(&g_mx);
        }
    }

    pthread_join(th, NULL);
    gpiod_line_request_set_value(g_req, g_off, GPIOD_LINE_VALUE_INACTIVE);
    gpiod_line_request_release(g_req);
    printf("\n종료\n");
    return EXIT_SUCCESS;
}
