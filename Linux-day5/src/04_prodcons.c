/* 04_prodcons.c — 생산자 · 소비자와 조건 변수 (링 버퍼 8칸)
 * 빌드: make 04_prodcons
 * 실행: ./04_prodcons            (소비자가 느림 → 생산자가 "가득 참" 에서 기다림)
 *       ./04_prodcons fastcons   (생산자가 느림 → 소비자가 "비어 있음" 에서 기다림)
 */
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define CAP 8
#define N   40

static int buf[CAP];
static int head = 0, tail = 0, count = 0;
static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t not_full  = PTHREAD_COND_INITIALIZER;
static pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
static int prod_wait = 0, cons_wait = 0;
static int slow_cons = 1;

static void *producer(void *arg)
{
    (void)arg;
    for (int i = 1; i <= N; i++) {
        if (!slow_cons) usleep(3000);
        pthread_mutex_lock(&m);
        while (count == CAP) {           /* if 가 아니라 while — 깨어나면 다시 확인 */
            prod_wait++;
            pthread_cond_wait(&not_full, &m);   /* m 을 풀고 잠듦 → 깨면 다시 잠금 */
        }
        buf[tail] = i;
        tail = (tail + 1) % CAP;
        count++;
        pthread_cond_signal(&not_empty); /* 기다리는 소비자 하나 깨움 */
        pthread_mutex_unlock(&m);
    }
    return NULL;
}

static void *consumer(void *arg)
{
    (void)arg;
    long sum = 0;
    for (int i = 1; i <= N; i++) {
        pthread_mutex_lock(&m);
        while (count == 0) {
            cons_wait++;
            pthread_cond_wait(&not_empty, &m);
        }
        int v = buf[head];
        head = (head + 1) % CAP;
        count--;
        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&m);
        sum += v;
        if (slow_cons) usleep(3000);     /* 처리하는 척 */
    }
    printf("소비자: 합계 %ld (기대 %d)\n", sum, N * (N + 1) / 2);
    return NULL;
}

int main(int argc, char *argv[])
{
    slow_cons = !(argc > 1 && strcmp(argv[1], "fastcons") == 0);
    pthread_t p, c;
    pthread_create(&p, NULL, producer, NULL);
    pthread_create(&c, NULL, consumer, NULL);
    pthread_join(p, NULL);
    pthread_join(c, NULL);
    printf("생산자가 '가득 참' 에서 기다린 횟수 %d, 소비자가 '비어 있음' 에서 기다린 횟수 %d\n", prod_wait, cons_wait);
    return 0;
}
