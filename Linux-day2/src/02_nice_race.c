/* 02_nice_race.c — 같은 CPU 에서 nice 값이 다른 두 프로세스의 계산량 비교
 * 빌드: make 02_nice_race
 * 실행: ./02_nice_race 10      (자식 A: nice 0, 자식 B: nice 10)
 *       ./02_nice_race 5 / 0 / 19 로 바꿔 보기
 * 두 자식을 모두 CPU 0 에 고정해 반드시 경쟁하게 만든다.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/wait.h>

/* CFS 의 nice → weight 표 (커널 sched_prio_to_weight, nice -20 ~ 19) */
static const int WEIGHT[40] = {
 88761, 71755, 56483, 46273, 36291, 29154, 23254, 18705, 14949, 11916,
  9548,  7620,  6100,  4904,  3906,  3121,  2501,  1991,  1586,  1277,
  1024,   820,   655,   526,   423,   335,   272,   215,   172,   137,
   110,    87,    70,    56,    45,    36,    29,    23,    18,    15 };

static double now_sec(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec / 1e9;
}

int main(int argc, char *argv[])
{
    int nice_b = (argc > 1) ? atoi(argv[1]) : 10;
    if (nice_b < 0 || nice_b > 19) {
        fprintf(stderr, "nice 값은 0~19\n");
        return 1;
    }
    /* 자식 둘이 결과를 써 둘 공유 메모리 (Day 3 에서 자세히) */
    unsigned long *count = mmap(NULL, 2 * sizeof(unsigned long), PROT_READ | PROT_WRITE,
                                MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (count == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    double start = now_sec() + 0.2;              /* 둘이 동시에 출발 */

    for (int k = 0; k < 2; k++) {
        if (fork() == 0) {
            cpu_set_t set;
            CPU_ZERO(&set);
            CPU_SET(0, &set);
            sched_setaffinity(0, sizeof(set), &set);       /* 둘 다 CPU 0 */
            if (k == 1)
                setpriority(PRIO_PROCESS, 0, nice_b);      /* B 만 nice 올림 */
            while (now_sec() < start)
                ;
            unsigned long n = 0;
            while (now_sec() < start + 3.0)
                n++;
            count[k] = n;
            _exit(0);
        }
    }
    while (wait(NULL) > 0)
        ;
    double tot = (double)count[0] + count[1];
    double wa = WEIGHT[20], wb = WEIGHT[20 + nice_b];
    printf("A (nice 0 ) : %12lu 회  %5.1f %%   예상 %5.1f %%\n", count[0], 100 * count[0] / tot, 100 * wa / (wa + wb));
    printf("B (nice %-2d) : %12lu 회  %5.1f %%   예상 %5.1f %%\n", nice_b, count[1], 100 * count[1] / tot, 100 * wb / (wa + wb));
    printf("weight      : %.0f vs %.0f\n", wa, wb);
    return 0;
}
