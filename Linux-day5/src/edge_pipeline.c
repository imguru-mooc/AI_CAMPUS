/* edge_pipeline.c — 5일 종합: 센서 → 처리 → 기록 3단 쓰레드 파이프라인
 *
 *   [센서 쓰레드]  10ms 절대 주기로 가상 센서 값 생성 (Day 2: clock_nanosleep, FIFO, CPU 고정)
 *        │  링 버퍼 64칸 (Day 5: mutex + 조건 변수)
 *   [처리 쓰레드]  이동 평균 10개, 임계치 넘으면 알람 (Day 5: cond_wait)
 *        │  통계 구조체 (mutex)
 *   [기록 쓰레드]  1초마다 CSV 한 줄 + fsync (Day 4)
 *   [main]         mlockall (Day 3), SIGINT 로 정상 종료 (Day 1)
 *
 * 빌드: make edge_pipeline
 * 실행: ./edge_pipeline -d 10                 (일반 정책, 10초)
 *       sudo ./edge_pipeline -d 10 -r         (센서 SCHED_FIFO 80 + CPU 1 + mlockall)
 *       stress-ng --cpu 2 & 을 켜고 두 실행의 max_lat_us 를 비교
 *       Ctrl+C 로 언제든 정상 종료 (CSV 는 끝까지 저장)
 * 출력: edge.csv — sec,samples,avg,max_lat_us,alarms,dropped
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <sys/mman.h>

#define CAP       64
#define PERIOD_NS 10000000L          /* 10 ms */
#define WIN       10                 /* 이동 평균 창 */

static volatile sig_atomic_t g_stop = 0;
static void on_sigint(int sig) { (void)sig; g_stop = 1; }

/* ── 링 버퍼 ── */
static double ring[CAP];
static int head = 0, tail = 0, count = 0;
static pthread_mutex_t rmx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  not_empty = PTHREAD_COND_INITIALIZER;

/* ── 1초 통계 (처리 · 센서 → 기록) ── */
static struct { long samples, alarms, dropped; double sum; long max_lat_ns; } st;
static pthread_mutex_t smx = PTHREAD_MUTEX_INITIALIZER;

static double threshold = 0.8;
static int use_rt = 0;
static const char *outpath = "edge.csv";

static long long ns(struct timespec t) { return t.tv_sec * 1000000000LL + t.tv_nsec; }
static void add_ns(struct timespec *t, long d)
{
    t->tv_nsec += d;
    while (t->tv_nsec >= 1000000000L) { t->tv_nsec -= 1000000000L; t->tv_sec++; }
}

static void *sensor(void *arg)
{
    (void)arg;
    if (use_rt) {                                    /* Day 2 */
        cpu_set_t set;
        CPU_ZERO(&set);
        CPU_SET(sysconf(_SC_NPROCESSORS_ONLN) > 1 ? 1 : 0, &set);
        pthread_setaffinity_np(pthread_self(), sizeof(set), &set);
        struct sched_param sp = { .sched_priority = 80 };
        if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sp) != 0)
            fprintf(stderr, "센서: SCHED_FIFO 실패 (sudo 필요) — 일반 정책으로 계속\n");
    }
    struct timespec next, now;
    clock_gettime(CLOCK_MONOTONIC, &next);
    long k = 0;
    while (!g_stop) {
        add_ns(&next, PERIOD_NS);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);
        clock_gettime(CLOCK_MONOTONIC, &now);
        long lat = (long)(ns(now) - ns(next));       /* 깨어남 지연 */

        double t = k++ * 0.01;                       /* 가상 센서: 2초 주기 사인파 + 잡음 */
        double v = sin(2 * M_PI * t / 2.0) + ((rand() % 1000) / 1000.0 - 0.5) * 0.3;

        pthread_mutex_lock(&rmx);
        if (count == CAP) {                          /* 가득 차면 가장 오래된 값을 버림 */
            head = (head + 1) % CAP;
            count--;
            pthread_mutex_lock(&smx); st.dropped++; pthread_mutex_unlock(&smx);
        }
        ring[tail] = v;
        tail = (tail + 1) % CAP;
        count++;
        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&rmx);

        pthread_mutex_lock(&smx);
        if (lat > st.max_lat_ns) st.max_lat_ns = lat;
        pthread_mutex_unlock(&smx);
    }
    pthread_mutex_lock(&rmx);
    pthread_cond_broadcast(&not_empty);              /* 처리 쓰레드를 깨워 끝내게 */
    pthread_mutex_unlock(&rmx);
    return NULL;
}

static void *process(void *arg)
{
    (void)arg;
    double win[WIN] = { 0 };
    int wi = 0, filled = 0;
    for (;;) {
        pthread_mutex_lock(&rmx);
        while (count == 0 && !g_stop)
            pthread_cond_wait(&not_empty, &rmx);     /* 값이 올 때까지 잠듦 */
        if (count == 0 && g_stop) {
            pthread_mutex_unlock(&rmx);
            break;
        }
        double v = ring[head];
        head = (head + 1) % CAP;
        count--;
        pthread_mutex_unlock(&rmx);                  /* 계산은 잠금 밖에서 */

        win[wi] = v;
        wi = (wi + 1) % WIN;
        if (filled < WIN) filled++;
        double avg = 0;
        for (int i = 0; i < filled; i++) avg += win[i];
        avg /= filled;

        pthread_mutex_lock(&smx);
        st.samples++;
        st.sum += avg;
        if (avg > threshold) st.alarms++;
        pthread_mutex_unlock(&smx);
    }
    return NULL;
}

static void *logger(void *arg)
{
    (void)arg;
    FILE *fp = fopen(outpath, "w");
    if (fp == NULL) { perror(outpath); g_stop = 1; return NULL; }
    fprintf(fp, "sec,samples,avg,max_lat_us,alarms,dropped\n");
    printf("%4s %8s %8s %11s %7s %8s\n", "초", "samples", "avg", "max_lat_us", "alarms", "dropped");
    struct timespec next;
    clock_gettime(CLOCK_MONOTONIC, &next);
    for (int sec = 1; !g_stop; sec++) {
        next.tv_sec += 1;
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);
        pthread_mutex_lock(&smx);
        long n = st.samples, a = st.alarms, d = st.dropped, lat = st.max_lat_ns;
        double avg = n ? st.sum / n : 0;
        memset(&st, 0, sizeof(st));                  /* 다음 1초를 위해 초기화 */
        pthread_mutex_unlock(&smx);
        fprintf(fp, "%d,%ld,%.3f,%.1f,%ld,%ld\n", sec, n, avg, lat / 1000.0, a, d);
        fflush(fp);
        fsync(fileno(fp));                           /* Day 4: 1초에 한 번만 디스크까지 */
        printf("%4d %8ld %8.3f %11.1f %7ld %8ld\n", sec, n, avg, lat / 1000.0, a, d);
        fflush(stdout);
    }
    fclose(fp);
    return NULL;
}

int main(int argc, char *argv[])
{
    int dur = 10, opt;
    while ((opt = getopt(argc, argv, "d:rt:o:")) != -1) {
        switch (opt) {
        case 'd': dur = atoi(optarg); break;
        case 'r': use_rt = 1; break;
        case 't': threshold = atof(optarg); break;
        case 'o': outpath = optarg; break;
        default:
            fprintf(stderr, "사용법: %s [-d 초] [-r] [-t 임계치] [-o csv]\n", argv[0]);
            return 1;
        }
    }
    if (use_rt && mlockall(MCL_CURRENT | MCL_FUTURE) == -1)   /* Day 3 */
        perror("mlockall (sudo 필요) — 잠금 없이 계속");
    signal(SIGINT, on_sigint);                                 /* Day 1 */
    printf("edge_pipeline: %d초, 정책=%s, 임계치=%.2f → %s\n", dur, use_rt ? "센서 FIFO 80 + CPU 고정 + mlockall" : "일반(OTHER)", threshold, outpath);

    pthread_t ts, tp, tl;
    pthread_create(&tp, NULL, process, NULL);
    pthread_create(&tl, NULL, logger, NULL);
    pthread_create(&ts, NULL, sensor, NULL);
    for (int i = 0; i < dur * 10 && !g_stop; i++)
        usleep(100000);
    g_stop = 1;                                                /* 모든 쓰레드에 종료 알림 */
    pthread_join(ts, NULL);
    pthread_join(tp, NULL);
    pthread_join(tl, NULL);
    printf("정상 종료 — %s 저장\n", outpath);
    return 0;
}
