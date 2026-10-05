/* 07_busy.c — 1초마다 계산량을 출력하는 CPU 부하 프로그램 (cgroup 실습용)
 * 빌드: make 07_busy
 * 실행: ./07_busy                                   (제한 없음)
 *       sudo systemd-run --scope -p CPUQuota=20% ./07_busy   (CPU 20% 제한)
 *       Ctrl+C 로 종료
 */
#include <stdio.h>
#include <time.h>

static double now_sec(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec / 1e9;
}

int main(void)
{
    double base = 0;
    for (int sec = 1; ; sec++) {
        double end = now_sec() + 1.0;
        unsigned long n = 0;
        while (now_sec() < end)
            n++;
        if (sec == 1)
            base = n;
        printf("%3d초: %11lu 회 (첫 1초 대비 %5.1f %%)\n", sec, n, 100.0 * n / base);
        fflush(stdout);
    }
    return 0;
}
