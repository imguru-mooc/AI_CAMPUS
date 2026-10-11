/* 05_sem.c — 세마포어로 동시 사용자 수 제한 (가속기 2개를 쓰레드 5개가 나눠 씀)
 * 빌드: make 05_sem
 * 실행: ./05_sem
 */
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

static sem_t slots;                      /* 남은 자리 수 */
static int active = 0, max_active = 0;
static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

static void *job(void *arg)
{
    int id = *(int *)arg;
    sem_wait(&slots);                    /* 자리가 0 이면 여기서 기다림 */
    pthread_mutex_lock(&m);
    active++;
    if (active > max_active) max_active = active;
    printf("작업 %d 시작 (동시 실행 %d)\n", id, active);
    pthread_mutex_unlock(&m);

    usleep(200000);                      /* 가속기 사용 0.2초 */

    pthread_mutex_lock(&m);
    active--;
    printf("작업 %d 끝\n", id);
    pthread_mutex_unlock(&m);
    sem_post(&slots);                    /* 자리 반납 → 기다리던 쓰레드 하나 진행 */
    return NULL;
}

int main(void)
{
    sem_init(&slots, 0, 2);              /* 0 = 쓰레드끼리 공유, 초기값 2 */
    pthread_t th[5];
    int ids[5] = { 1, 2, 3, 4, 5 };
    for (int i = 0; i < 5; i++)
        pthread_create(&th[i], NULL, job, &ids[i]);
    for (int i = 0; i < 5; i++)
        pthread_join(th[i], NULL);
    printf("최대 동시 실행 = %d (세마포어 초기값 2)\n", max_active);
    sem_destroy(&slots);
    return 0;
}
