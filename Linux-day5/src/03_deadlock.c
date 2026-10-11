/* 03_deadlock.c — 두 mutex 를 반대 순서로 잡으면 교착(deadlock)
 * 빌드: make 03_deadlock
 * 실행: ./03_deadlock          (A→B / B→A — 멈춘다, 3초 뒤 감시자가 알림)
 *       ./03_deadlock fixed    (둘 다 A→B — 정상 종료)
 *       멈춘 상태에서 다른 창: sudo gdb -p $(pgrep 03_deadlock) -batch -ex "thread apply all bt"
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>

static pthread_mutex_t A = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t B = PTHREAD_MUTEX_INITIALIZER;
static int fixed = 0;

static void *t1(void *arg)
{
    (void)arg;
    pthread_mutex_lock(&A);
    printf("t1: A 잡음\n");
    usleep(100000);                      /* 상대가 B 를 잡을 시간 */
    printf("t1: B 기다림...\n");
    pthread_mutex_lock(&B);
    printf("t1: A, B 모두 잡음 — 작업\n");
    pthread_mutex_unlock(&B);
    pthread_mutex_unlock(&A);
    return NULL;
}

static void *t2(void *arg)
{
    (void)arg;
    pthread_mutex_t *first = fixed ? &A : &B, *second = fixed ? &B : &A;
    pthread_mutex_lock(first);
    printf("t2: %s 잡음\n", fixed ? "A" : "B");
    usleep(100000);
    printf("t2: %s 기다림...\n", fixed ? "B" : "A");
    pthread_mutex_lock(second);
    printf("t2: A, B 모두 잡음 — 작업\n");
    pthread_mutex_unlock(second);
    pthread_mutex_unlock(first);
    return NULL;
}

static void on_alarm(int sig)
{
    (void)sig;
    const char msg[] = "감시자: 3초 동안 진행 없음 — 교착(deadlock). 서로 상대가 가진 mutex 를 기다린다.\n";
    if (write(2, msg, sizeof(msg) - 1) < 0) { }
    _exit(1);
}

int main(int argc, char *argv[])
{
    fixed = (argc > 1 && strcmp(argv[1], "fixed") == 0);
    signal(SIGALRM, on_alarm);
    alarm(3);                            /* 3초 안에 안 끝나면 교착으로 본다 */
    pthread_t a, b;
    pthread_create(&a, NULL, t1, NULL);
    pthread_create(&b, NULL, t2, NULL);
    pthread_join(a, NULL);
    pthread_join(b, NULL);
    printf("정상 종료 (%s)\n", fixed ? "잠금 순서를 A→B 로 통일" : "운 좋게 엇갈리지 않음");
    return 0;
}
