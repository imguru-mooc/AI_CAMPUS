/* 04_affinity.c — 허용 CPU 목록 확인과 CPU 고정
 * 빌드: make 04_affinity
 * 실행: ./04_affinity          (고정 없이: 어느 CPU 에서 도는지 0.2초마다 기록)
 *       ./04_affinity 1        (CPU 1 에 고정)
 *       taskset -c 0 ./04_affinity   (셸에서 고정)
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <time.h>
#include <unistd.h>

static void print_mask(const char *title)
{
    cpu_set_t set;
    CPU_ZERO(&set);
    if (sched_getaffinity(0, sizeof(set), &set) == -1) {
        perror("sched_getaffinity");
        exit(1);
    }
    printf("%s 허용 CPU:", title);
    for (int c = 0; c < CPU_SETSIZE; c++)
        if (CPU_ISSET(c, &set))
            printf(" %d", c);
    printf("  (총 %d개)\n", CPU_COUNT(&set));
}

int main(int argc, char *argv[])
{
    printf("온라인 CPU 수 = %ld\n", sysconf(_SC_NPROCESSORS_ONLN));
    print_mask("[시작]");

    if (argc > 1) {
        int cpu = atoi(argv[1]);
        cpu_set_t set;
        CPU_ZERO(&set);
        CPU_SET(cpu, &set);
        if (sched_setaffinity(0, sizeof(set), &set) == -1) {
            perror("sched_setaffinity");          /* 없는 CPU 번호면 EINVAL */
            return 1;
        }
        print_mask("[고정]");
    }

    printf("0.2초마다 실행 중인 CPU:");
    for (int i = 0; i < 15; i++) {
        struct timespec t0, t;
        clock_gettime(CLOCK_MONOTONIC, &t0);
        do {
            clock_gettime(CLOCK_MONOTONIC, &t);
        } while ((t.tv_sec - t0.tv_sec) * 1e9 + (t.tv_nsec - t0.tv_nsec) < 2e8);
        printf(" %d", sched_getcpu());
        fflush(stdout);
    }
    printf("\n");
    return 0;
}
