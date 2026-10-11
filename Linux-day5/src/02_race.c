/* 02_race.c — 공유 카운터를 여러 쓰레드가 증가시키면?
 * 빌드: make 02_race
 * 실행: ./02_race 2 1000000 none      (보호 없음 — 결과가 매번 다르다)
 *       ./02_race 2 1000000 mutex     (pthread_mutex)
 *       ./02_race 2 1000000 atomic    (원자적 덧셈)
 *       ./02_race 2 1000000 spin      (spinlock)
 *       objdump -d 02_race | grep -A6 "<add_none>"   (counter++ 의 기계어)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

static volatile long counter = 0;     /* volatile: 매번 메모리에서 읽고 쓰게 해 race 를 드러낸다 */
static long iters;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_spinlock_t spin;

__attribute__((noinline)) static void add_none(void) { counter++; }   /* 읽기(mov) → 더하기(add) → 쓰기(mov) */

static void *w_none(void *a)   { (void)a; for (long i = 0; i < iters; i++) add_none(); return NULL; }
static void *w_mutex(void *a)  { (void)a; for (long i = 0; i < iters; i++) { pthread_mutex_lock(&mtx); counter++; pthread_mutex_unlock(&mtx); } return NULL; }
static void *w_atomic(void *a) { (void)a; for (long i = 0; i < iters; i++) __atomic_fetch_add((long *)&counter, 1, __ATOMIC_RELAXED); return NULL; }
static void *w_spin(void *a)   { (void)a; for (long i = 0; i < iters; i++) { pthread_spin_lock(&spin); counter++; pthread_spin_unlock(&spin); } return NULL; }

static double now_ms(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1000.0 + t.tv_nsec / 1e6;
}

int main(int argc, char *argv[])
{
    int n = (argc > 1) ? atoi(argv[1]) : 2;
    iters = (argc > 2) ? atol(argv[2]) : 1000000;
    const char *mode = (argc > 3) ? argv[3] : "none";
    void *(*fn)(void *) = w_none;
    if (strcmp(mode, "mutex") == 0) fn = w_mutex;
    else if (strcmp(mode, "atomic") == 0) fn = w_atomic;
    else if (strcmp(mode, "spin") == 0) fn = w_spin;
    pthread_spin_init(&spin, PTHREAD_PROCESS_PRIVATE);

    pthread_t th[16];
    if (n > 16) n = 16;
    double t0 = now_ms();
    for (int i = 0; i < n; i++)
        pthread_create(&th[i], NULL, fn, NULL);
    for (int i = 0; i < n; i++)
        pthread_join(th[i], NULL);
    double ms = now_ms() - t0;
    long expect = n * iters;
    printf("%-6s 쓰레드 %d × %ld : 기대 %ld, 실제 %ld (잃어버림 %ld)  %.1f ms\n",
           mode, n, iters, expect, counter, expect - counter, ms);
    return 0;
}
