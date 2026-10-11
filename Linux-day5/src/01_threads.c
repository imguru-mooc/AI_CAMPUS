/* 01_threads.c — 쓰레드는 주소 공간을 공유한다 (fork 와 비교)
 * 빌드: make 01_threads
 * 실행: ./01_threads
 *       (다른 창) ps -L -p $(pgrep 01_threads)   또는  ls /proc/$(pgrep 01_threads)/task
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>

int g_value = 0;                         /* 모든 쓰레드가 같은 변수를 본다 */

static void *worker(void *arg)
{
    int id = *(int *)arg;
    g_value += 10;                       /* (이 예제는 순서대로 실행되도록 아래에서 join) */
    printf("쓰레드 %d: PID=%d TID=%d  &g_value=%p  g_value=%d\n",
           id, getpid(), gettid(), (void *)&g_value, g_value);
    sleep(2);                            /* ps -L 로 볼 시간 */
    int *ret = malloc(sizeof(int));
    *ret = id * 100;
    return ret;                          /* pthread_join 으로 받는다 */
}

int main(void)
{
    pthread_t th[3];
    int ids[3] = { 1, 2, 3 };
    printf("main   : PID=%d TID=%d\n", getpid(), gettid());
    for (int i = 0; i < 3; i++) {
        pthread_create(&th[i], NULL, worker, &ids[i]);
        usleep(100000);                  /* 출력이 섞이지 않게 잠깐 */
    }
    for (int i = 0; i < 3; i++) {
        void *r;
        pthread_join(th[i], &r);         /* 끝날 때까지 기다리고 반환값 회수 */
        printf("join   : 쓰레드 %d 반환값 %d\n", i + 1, *(int *)r);
        free(r);
    }
    printf("쓰레드가 모두 끝난 뒤 g_value = %d (공유)\n", g_value);

    fflush(stdout);                      /* Day 1: fork 전에 stdio 버퍼를 비운다 */
    pid_t pid = fork();                  /* 비교: 프로세스는 복사본 */
    if (pid == 0) {
        g_value += 1000;
        printf("fork 자식: g_value = %d\n", g_value);
        exit(0);
    }
    wait(NULL);
    printf("fork 부모: g_value = %d (자식이 바꿔도 그대로)\n", g_value);
    return 0;
}
