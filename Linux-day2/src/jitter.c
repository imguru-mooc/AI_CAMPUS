/* jitter.c — 주기 태스크의 깨어남 지연(latency) 측정기
 * 빌드: make jitter
 * 사용법: ./jitter [-p 주기us] [-n 횟수] [-f FIFO우선순위] [-c CPU] [-o 출력.csv]
 *   ./jitter                         SCHED_OTHER, 1000us × 5000번
 *   sudo ./jitter -f 80              SCHED_FIFO 80
 *   sudo ./jitter -f 80 -c 1         SCHED_FIFO 80 + CPU 1 고정
 *   stress-ng --cpu 2 &   을 켜고 끄며 비교
 * 출력: 지연의 최소·평균·최대·99% 값(us), CSV 에는 회차별 지연
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <time.h>
#include <unistd.h>

static long long ns(struct timespec t) { return t.tv_sec * 1000000000LL + t.tv_nsec; }

static int cmp_ll(const void *a, const void *b)
{
    long long x = *(const long long *)a, y = *(const long long *)b;
    return (x > y) - (x < y);
}

int main(int argc, char *argv[])
{
    long period_us = 1000;
    int loops = 5000, fifo = 0, cpu = -1;
    const char *out = NULL;
    int opt;
    while ((opt = getopt(argc, argv, "p:n:f:c:o:")) != -1) {
        switch (opt) {
        case 'p': period_us = atol(optarg); break;
        case 'n': loops = atoi(optarg); break;
        case 'f': fifo = atoi(optarg); break;
        case 'c': cpu = atoi(optarg); break;
        case 'o': out = optarg; break;
        default:
            fprintf(stderr, "사용법: %s [-p us] [-n 횟수] [-f prio] [-c cpu] [-o csv]\n", argv[0]);
            return 1;
        }
    }
    if (cpu >= 0) {
        cpu_set_t set;
        CPU_ZERO(&set);
        CPU_SET(cpu, &set);
        if (sched_setaffinity(0, sizeof(set), &set) == -1) {
            perror("sched_setaffinity");
            return 1;
        }
    }
    if (fifo > 0) {
        struct sched_param sp = { .sched_priority = fifo };
        if (sched_setscheduler(0, SCHED_FIFO, &sp) == -1) {
            perror("sched_setscheduler (sudo 필요)");
            return 1;
        }
    }
    long long *lat = malloc(sizeof(long long) * loops);
    if (lat == NULL)
        return 1;

    struct timespec next, now;
    clock_gettime(CLOCK_MONOTONIC, &next);
    for (int i = 0; i < loops; i++) {
        next.tv_nsec += period_us * 1000;
        while (next.tv_nsec >= 1000000000L) {
            next.tv_nsec -= 1000000000L;
            next.tv_sec++;
        }
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);
        clock_gettime(CLOCK_MONOTONIC, &now);
        lat[i] = ns(now) - ns(next);             /* 깨어나야 할 시각보다 늦은 만큼 */
    }

    if (out) {
        FILE *fp = fopen(out, "w");
        if (fp) {
            fprintf(fp, "i,latency_us\n");
            for (int i = 0; i < loops; i++)
                fprintf(fp, "%d,%.1f\n", i, lat[i] / 1000.0);
            fclose(fp);
        }
    }
    long long sum = 0;
    for (int i = 0; i < loops; i++)
        sum += lat[i];
    qsort(lat, loops, sizeof(long long), cmp_ll);
    printf("정책=%s", fifo ? "SCHED_FIFO" : "SCHED_OTHER");
    if (fifo)
        printf("(%d)", fifo);
    if (cpu >= 0)
        printf("  CPU=%d", cpu);
    else
        printf("  CPU=자유");
    printf("  주기=%ldus  횟수=%d\n", period_us, loops);
    printf("지연(us)  최소 %7.1f  평균 %7.1f  99%% %7.1f  최대 %7.1f\n",
           lat[0] / 1000.0, sum / 1000.0 / loops, lat[loops * 99 / 100] / 1000.0, lat[loops - 1] / 1000.0);
    free(lat);
    return 0;
}
