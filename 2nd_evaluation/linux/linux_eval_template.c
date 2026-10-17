/* linux_eval_template.c — 2차 평가 · Linux & Shell 실습 (정규 평가)
 *
 * 환경 : VirtualBox · Ubuntu Server 22.04 (운영체제 수업 VM) · PuTTY 접속
 * 빌드 : gcc -Wall -g -pthread -o linux_eval linux_eval_template.c
 * 실행 : ./linux_eval
 *
 * 규칙
 *  - 빈칸 ______ 8곳과 TODO 3곳만 채운다.
 *  - 각 단계 끝의 printf("[N단계] 결과 ...") / printf("[N단계] 기대값 ...") 는 고치지 않는다.
 *  - 두 줄이 같으면 그 단계는 완료. 빌드가 안 되는 단계가 있으면 main() 에서 그 단계만
 *    주석 처리하고 나머지 단계를 진행해도 된다.
 *
 *  단계  주제                         Day   형태
 *   1    시스템 콜 실패와 errno         1    빈칸 2
 *   2    open · write · read         4    빈칸 1
 *   3    inode 와 하드 링크            4    빈칸 1
 *   4    fork · waitpid · 종료 코드    1    빈칸 2 + TODO 1 (한 줄)
 *   5    /proc/self/status 읽기        1    TODO 2 (함수)
 *   6    쓰레드와 mutex                5    빈칸 2 + TODO 3
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

/* ───────────── 1단계: 시스템 콜은 실패한다 ─────────────
 * 없는 파일을 열고, 반환값과 errno 로 실패 원인을 확인한다. */
static void step1(void)
{
    int fd = open("/no/such/file.txt", O_RDONLY);
    int failed    = (fd == ______);          /* ① 시스템 콜이 실패했을 때 돌려주는 값 */
    int is_enoent = (errno == ______);       /* ② "파일이 없다" 를 뜻하는 errno 이름 */

    printf("[1단계] 결과 : failed=%d, ENOENT=%d, msg=%s\n", failed, is_enoent, strerror(errno));
    printf("[1단계] 기대값: failed=1, ENOENT=1, msg=No such file or directory\n\n");
}

/* ───────────── 2단계: 파일 디스크립터로 쓰고 읽기 ─────────────
 * eval.txt 에 8바이트를 쓰고, 다시 열어 읽는다. */
static void step2(void)
{
    const char *msg = "edge-ai\n";                                    /* 8 바이트 */

    int fd = open("eval.txt", O_WRONLY | O_CREAT | ______, 0644);     /* ① 기존 내용을 지우고 새로 쓰기 */
    ssize_t w = write(fd, msg, strlen(msg));
    close(fd);

    char buf[32] = {0};
    fd = open("eval.txt", O_RDONLY);                                  /* 읽기 전용으로 다시 열기 */
    ssize_t r = read(fd, buf, sizeof(buf) - 1);
    close(fd);

    printf("[2단계] 결과 : write=%zd, read=%zd, 내용=%s", w, r, buf);
    printf("[2단계] 기대값: write=8, read=8, 내용=edge-ai\n\n");
}

/* ───────────── 3단계: inode 와 하드 링크 ─────────────
 * eval.txt 에 하드 링크 eval_link.txt 를 만들고 두 이름의 inode 를 비교한다. */
static void step3(void)
{
    unlink("eval_link.txt");                 /* 이전 실행에서 남은 링크 지우기 */
    link("eval.txt", "eval_link.txt");       /* 하드 링크 = 같은 inode 를 가리키는 이름 하나 더 */

    struct stat a, b;
    stat("eval.txt", &a);
    stat("eval_link.txt", &b);

    int same_inode = (a.______ == b.st_ino);                /* ① inode 번호 멤버 */
    unsigned long nlink = (unsigned long)a.st_nlink;        /* 링크 수(이름 개수) */

    printf("[3단계] 결과 : 같은 inode=%d, 링크 수=%lu, 크기=%lld\n",
           same_inode, nlink, (long long)a.st_size);
    printf("[3단계] 기대값: 같은 inode=1, 링크 수=2, 크기=8\n\n");
}

/* ───────────── 4단계: fork · waitpid · 종료 코드 ─────────────
 * 자식은 exit(7) 로 끝나고, 부모는 자식을 회수해 종료 코드를 꺼낸다. */
static void step4(void)
{
    fflush(stdout);                          /* fork 전에 stdio 버퍼를 비운다 (출력 중복 방지) */
    pid_t pid = fork();
    if (pid == ______) {                     /* ① fork 가 "자식" 에게 돌려주는 값 */
        exit(7);
    }

    int status = 0;
    /* TODO 1: waitpid 로 자식(pid)이 끝날 때까지 기다리고, 종료 상태를 status 에 받는다 (한 줄) */


    int code = -1;
    if (WIFEXITED(status))
        code = ______(status);               /* ② 종료 상태에서 종료 코드(0~255)를 꺼내는 매크로 */

    printf("[4단계] 결과 : 정상 종료=%d, 종료 코드=%d\n", WIFEXITED(status) ? 1 : 0, code);
    printf("[4단계] 기대값: 정상 종료=1, 종료 코드=7\n\n");
}

/* ───────────── 5단계: /proc/self/status 읽기 ─────────────
 * /proc/self/status 에서 key 로 시작하는 줄의 숫자를 돌려준다. 못 찾으면 -1.
 *   예) "Threads:\t1"      → proc_status_value("Threads:") == 1
 *       "VmRSS:\t   1480 kB" → proc_status_value("VmRSS:")   == 1480          */
static long proc_status_value(const char *key)
{
    long value = -1;
    /* TODO 2:
     *   ① fopen("/proc/self/status", "r") 로 연다 (실패하면 -1 반환)
     *   ② fgets 로 한 줄씩 읽으며 strncmp(line, key, strlen(key)) == 0 인 줄을 찾는다
     *   ③ sscanf(line + strlen(key), "%ld", &value) 로 숫자를 꺼낸다
     *   ④ fclose 후 value 를 반환한다                                         */

    return value;
}

static void step5(void)
{
    long th  = proc_status_value("Threads:");
    long rss = proc_status_value("VmRSS:");

    printf("[5단계] 결과 : Threads=%ld, VmRSS>0=%d\n", th, rss > 0);
    printf("[5단계] 기대값: Threads=1, VmRSS>0=1\n\n");
}

/* ───────────── 6단계: 쓰레드 4개와 mutex ─────────────
 * 쓰레드 4개가 공유 카운터를 100000 번씩 더한다. mutex 가 없으면 결과가 매번 달라진다. */
#define N_THREAD 4
#define N_LOOP   100000

static long g_counter = 0;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

static void *worker(void *arg)
{
    (void)arg;
    for (int i = 0; i < N_LOOP; i++) {
        /* TODO 3: 아래 g_counter++ 를 g_lock 으로 보호한다 (앞에 lock, 뒤에 unlock) */
        g_counter++;
    }
    return NULL;
}

static void step6(void)
{
    pthread_t t[N_THREAD];
    for (int i = 0; i < N_THREAD; i++)
        ______(&t[i], NULL, worker, NULL);   /* ① 쓰레드 만들기 */
    for (int i = 0; i < N_THREAD; i++)
        ______(t[i], NULL);                  /* ② 쓰레드가 끝날 때까지 기다려 회수하기 */

    printf("[6단계] 결과 : counter=%ld\n", g_counter);
    printf("[6단계] 기대값: counter=400000 (여러 번 실행해도 항상 같음)\n\n");
}

int main(void)
{
    step1();
    step2();
    step3();
    step4();
    step5();
    step6();
    return 0;
}
