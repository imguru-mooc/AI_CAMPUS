/* memutil.h — Day 3 예제 공용 측정 함수 (헤더만 include 하면 된다) */
#ifndef MEMUTIL_H
#define MEMUTIL_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/resource.h>

/* /proc/self/status 에서 key(예: "VmRSS") 값을 kB 로 */
static inline long status_kb(const char *key)
{
    FILE *fp = fopen("/proc/self/status", "r");
    char line[256];
    long v = -1;
    size_t n = strlen(key);
    if (fp == NULL)
        return -1;
    while (fgets(line, sizeof(line), fp) != NULL)
        if (strncmp(line, key, n) == 0 && line[n] == ':') {
            v = atol(line + n + 1);
            break;
        }
    fclose(fp);
    return v;
}

/* 지금까지의 minor page fault 수 (디스크 없이 처리된 fault) */
static inline long minflt(void)
{
    struct rusage ru;
    getrusage(RUSAGE_SELF, &ru);
    return ru.ru_minflt;
}

static inline long majflt(void)
{
    struct rusage ru;
    getrusage(RUSAGE_SELF, &ru);
    return ru.ru_majflt;
}

static inline double now_ms(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1000.0 + t.tv_nsec / 1e6;
}
#endif
