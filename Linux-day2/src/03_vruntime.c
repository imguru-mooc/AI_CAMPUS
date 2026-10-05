/* 03_vruntime.c — /proc/self/sched 의 vruntime 이 얼마나 빨리 느는지 관찰
 * 빌드: make 03_vruntime
 * 실행: ./03_vruntime          (nice 0)
 *       nice -n 10 ./03_vruntime
 * 0.5초 계산할 때마다 실제 실행 시간(sum_exec_runtime)과 vruntime 증가량을 비교한다.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/resource.h>

/* /proc/self/sched 에서 "이름 : 값" 형식의 값을 읽는다 (단위 ms) */
static double sched_value(const char *key)
{
    FILE *fp = fopen("/proc/self/sched", "r");
    if (fp == NULL) {
        perror("fopen /proc/self/sched");
        exit(1);
    }
    char line[256];
    double v = -1;
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strncmp(line, key, strlen(key)) == 0) {
            char *colon = strchr(line, ':');
            if (colon)
                v = atof(colon + 1);
            break;
        }
    }
    fclose(fp);
    return v;
}

static double now_sec(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec / 1e9;
}

int main(void)
{
    printf("nice = %d\n", getpriority(PRIO_PROCESS, 0));
    printf("%8s %14s %14s %8s\n", "구간", "실행시간(ms)", "vruntime(ms)", "비율");
    double e0 = sched_value("se.sum_exec_runtime");
    double v0 = sched_value("se.vruntime");
    for (int i = 1; i <= 4; i++) {
        double end = now_sec() + 0.5;
        volatile unsigned long x = 0;
        while (now_sec() < end)
            x++;
        double e1 = sched_value("se.sum_exec_runtime");
        double v1 = sched_value("se.vruntime");
        printf("%8d %14.1f %14.1f %8.2f\n", i, e1 - e0, v1 - v0, (v1 - v0) / (e1 - e0));
        e0 = e1;
        v0 = v1;
    }
    printf("비율 ≈ 1024 / weight(nice).  nice 0 → 1.00, nice 10 → 9.31\n");
    return 0;
}
