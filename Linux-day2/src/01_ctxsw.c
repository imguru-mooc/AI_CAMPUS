/* 01_ctxsw.c — 잠드는 프로그램 vs 계속 도는 프로그램의 컨텍스트 스위치 비교
 * 빌드: make 01_ctxsw
 * 실행: ./01_ctxsw sleep      (1ms씩 잠들기 1000번)
 *       ./01_ctxsw busy       (2초 동안 계속 계산)
 *       stress-ng --cpu 2 &  후 다시 busy 로 실행해 비자발적 전환 증가 확인
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* /proc/self/status 에서 key 로 시작하는 줄의 숫자 값을 돌려준다 */
static long status_value(const char *key)
{
    FILE *fp = fopen("/proc/self/status", "r");
    if (fp == NULL) {
        perror("fopen");
        return -1;
    }
    char line[256];
    long v = -1;
    size_t n = strlen(key);
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strncmp(line, key, n) == 0) {
            v = atol(line + n + 1);
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

int main(int argc, char *argv[])
{
    const char *mode = (argc > 1) ? argv[1] : "sleep";
    long v0 = status_value("voluntary_ctxt_switches");
    long n0 = status_value("nonvoluntary_ctxt_switches");

    if (strcmp(mode, "sleep") == 0) {
        for (int i = 0; i < 1000; i++)
            usleep(1000);                        /* 스스로 CPU 를 내려놓음 */
    } else {
        volatile unsigned long x = 0;
        double end = now_sec() + 2.0;
        while (now_sec() < end)
            x++;                                 /* 빼앗길 때까지 계속 계산 */
    }

    long v1 = status_value("voluntary_ctxt_switches");
    long n1 = status_value("nonvoluntary_ctxt_switches");
    printf("모드=%s\n", mode);
    printf("  자발적   전환 (스스로 잠듦)   : %ld\n", v1 - v0);
    printf("  비자발적 전환 (선점당함)      : %ld\n", n1 - n0);
    return 0;
}
