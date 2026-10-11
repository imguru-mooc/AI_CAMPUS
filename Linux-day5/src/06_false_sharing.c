/* 06_false_sharing.c — 같은 캐시 라인(64B)을 두 CPU 가 고치면 느려진다
 * 빌드: make 06_false_sharing
 * 실행: ./06_false_sharing          (두 쓰레드를 CPU 0 · 1 에 고정)
 * 서로 다른 변수인데도 같은 64바이트 안에 있으면 캐시 라인이 CPU 사이를 오간다.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>

#define ITERS 200000000L

struct together { volatile long a; volatile long b; };                          /* 같은 캐시 라인 */
struct apart    { volatile long a; char pad[64 - sizeof(long)]; volatile long b; }; /* 다른 캐시 라인 */

static struct together T;
static struct apart P __attribute__((aligned(64)));

struct arg { volatile long *p; int cpu; };

static void *spin_add(void *x)
{
    struct arg *a = x;
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(a->cpu, &set);
    pthread_setaffinity_np(pthread_self(), sizeof(set), &set);   /* 쓰레드별 CPU 고정 */
    for (long i = 0; i < ITERS; i++)
        (*a->p)++;
    return NULL;
}

static double run(volatile long *x, volatile long *y)
{
    int ncpu = sysconf(_SC_NPROCESSORS_ONLN) > 1 ? 1 : 0;
    struct arg a1 = { x, 0 }, a2 = { y, ncpu };
    struct timespec s, e;
    pthread_t t1, t2;
    clock_gettime(CLOCK_MONOTONIC, &s);
    pthread_create(&t1, NULL, spin_add, &a1);
    pthread_create(&t2, NULL, spin_add, &a2);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    clock_gettime(CLOCK_MONOTONIC, &e);
    return (e.tv_sec - s.tv_sec) * 1000.0 + (e.tv_nsec - s.tv_nsec) / 1e6;
}

int main(void)
{
    printf("&T.a=%p &T.b=%p (거리 %ld B)\n", (void *)&T.a, (void *)&T.b, (long)((char *)&T.b - (char *)&T.a));
    printf("&P.a=%p &P.b=%p (거리 %ld B)\n", (void *)&P.a, (void *)&P.b, (long)((char *)&P.b - (char *)&P.a));
    double t1 = run(&T.a, &T.b);
    double t2 = run(&P.a, &P.b);
    printf("같은 캐시 라인 : %8.1f ms\n", t1);
    printf("64B 떨어뜨림   : %8.1f ms  (%.1f 배 빠름)\n", t2, t1 / t2);
    if (sysconf(_SC_NPROCESSORS_ONLN) < 2)
        printf("(CPU 가 1개라 두 쓰레드가 번갈아 실행 — 차이가 거의 없다)\n");
    return 0;
}
