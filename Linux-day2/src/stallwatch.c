/* stallwatch.c — VM 의 vCPU 가 호스트에 의해 멈추는지 감시
 * 빌드: gcc -O2 -Wall -o stallwatch stallwatch.c
 * 실행: ./stallwatch 30        (30초 동안 감시, 기본 20초)
 * 원리: CPU 를 쉬지 않고 돌며 clock_gettime 을 연속으로 읽는다.
 *       연속 두 번 읽은 사이가 5ms 이상 벌어졌다면 그동안 이 프로세스가 실행되지 못한 것이다.
 *       게스트 안에 경쟁자가 없는데도 자주 생기면 호스트(Windows · Hyper-V)가 vCPU 를 멈춘 것이다.
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static long long ns(struct timespec t) { return t.tv_sec * 1000000000LL + t.tv_nsec; }

int main(int argc, char *argv[])
{
    int secs = (argc > 1) ? atoi(argv[1]) : 20;
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    long long start = ns(t), prev = start, end = start + secs * 1000000000LL;
    long long worst = 0, total_lost = 0;
    int count = 0;
    printf("%d초 동안 감시합니다 (5ms 이상 멈춤만 표시)\n", secs);
    while (prev < end) {
        clock_gettime(CLOCK_MONOTONIC, &t);
        long long now = ns(t), gap = now - prev;
        if (gap >= 5000000) {
            count++;
            total_lost += gap;
            if (gap > worst) worst = gap;
            if (count <= 15)
                printf("  %6.2f 초 지점: %8.1f ms 멈춤\n", (now - start) / 1e9, gap / 1e6);
        }
        prev = now;
    }
    printf("멈춤 %d회, 최대 %.1f ms, 합계 %.1f ms (전체의 %.2f %%)\n",
           count, worst / 1e6, total_lost / 1e6, 100.0 * total_lost / (secs * 1e9));
    printf("판단: %s\n", count == 0 ? "정상 — 실습 결과를 그대로 믿어도 됩니다" :
                         worst < 20000000 ? "가벼움 — 측정값이 조금 흔들릴 수 있습니다" :
                                            "심각 — 호스트가 vCPU 를 자주 멈춥니다 (Hyper-V · 전원 설정 · 호스트 부하 확인)");
    return 0;
}
