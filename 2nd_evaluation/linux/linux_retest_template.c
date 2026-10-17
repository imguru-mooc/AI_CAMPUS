/* linux_retest_template.c — 2차 평가 · Linux & Shell 실습 (재평가용 대체 과제)
 *
 * 환경 : VirtualBox · Ubuntu Server 22.04 (운영체제 수업 VM) · PuTTY 접속
 * 빌드 : gcc -Wall -g -pthread -o linux_retest linux_retest_template.c
 * 실행 : ./linux_retest
 *
 * 6단계 · 빈칸 ______ 8곳 + TODO 3곳 · 난이도 3구간 (단계마다 독립 — 앞 단계가 틀려도 다음 단계 채점 가능)
 *   [쉬움   1·2·4] 빈칸 채우기 (1단계 1곳, 2단계 2곳, 4단계 2곳)
 *   [보통   3    ] 빈칸 1곳 + TODO 1 (한 줄)
 *   [어려움 5·6  ] TODO 2 (함수) · TODO 3 (함수) + 6단계 빈칸 2곳
 * 각 단계 끝의 [N단계] 결과 / [N단계] 기대값 printf 는 고치지 않는다. 두 줄이 같으면 완료.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>

/* ───────────── [쉬움] 1단계: write 시스템 콜 직접 호출 ─────────────
 * printf 대신 write 로 표준 출력에 6바이트를 쓴다. */
static void step1(void)
{
    ssize_t n = write(______, "hello\n", 6);      /* ① 표준 출력(stdout)의 fd 번호 */

    printf("[1단계] 결과 : write 반환값=%zd\n", n);
    printf("[1단계] 기대값: hello 가 먼저 한 줄 출력되고, write 반환값=6\n\n");
}

/* ───────────── [쉬움] 2단계: lseek 으로 파일 크기 구하기 ───────────── */
static void step2(void)
{
    int fd = open("retest.txt", O_WRONLY | O_CREAT | ______, 0644);   /* ① 기존 내용을 지우고 새로 쓰기 */
    write(fd, "0123456789", 10);
    close(fd);

    fd = open("retest.txt", O_RDONLY);
    off_t size = lseek(fd, 0, ______);            /* ② 파일 끝을 기준으로 이동하는 whence 값 */
    close(fd);

    printf("[2단계] 결과 : size=%lld\n", (long long)size);
    printf("[2단계] 기대값: size=10\n\n");
}

/* ───────────── [보통] 3단계: fork + exec + waitpid ─────────────
 * 자식은 /bin/false 프로그램으로 바뀐다. false 는 항상 종료 코드 1 로 끝난다. */
static void step3(void)
{
    fflush(stdout);                               /* fork 전에 stdio 버퍼 비우기 */
    pid_t pid = fork();
    if (pid == 0) {
        /* TODO 1: execl 로 자식을 "/bin/false" 프로그램으로 바꾼다 (한 줄)
         *         인자 목록: "false", 마지막은 (char *)NULL                  */

        _exit(127);                               /* exec 가 실패했을 때만 여기에 온다 */
    }
    int status = 0;
    waitpid(pid, &status, 0);

    int code = WIFEXITED(status) ? ______(status) : -1;   /* ① 종료 상태에서 종료 코드를 꺼내는 매크로 */
    printf("[3단계] 결과 : 종료 코드=%d\n", code);
    printf("[3단계] 기대값: 종료 코드=1 (127 이면 exec 를 하지 않았거나 실패)\n\n");
}

/* ───────────── [쉬움] 4단계: CLOCK_MONOTONIC 으로 경과 시간 재기 ───────────── */
static void step4(void)
{
    struct timespec t0, t1;
    clock_gettime(______, &t0);                   /* ① 거꾸로 가지 않는(경과 시간용) 시계 */
    usleep(200 * 1000);                           /* 200 ms 잠들기 */
    clock_gettime(CLOCK_MONOTONIC, &t1);

    long ms = (t1.tv_sec - t0.tv_sec) * 1000
            + (t1.tv_nsec - t0.tv_nsec) / ______;  /* ② 나노초 → 밀리초로 나눌 값 */

    printf("[4단계] 결과 : 경과 %s\n", (ms >= 200 && ms < 260) ? "200~259 ms (OK)" : "범위 밖");
    printf("[4단계] 기대값: 경과 200~259 ms (OK)\n\n");
}

/* ───────────── [어려움] 5단계: read 로 줄 수 세기 ─────────────
 * path 파일을 open · read · close 로 끝까지 읽어 '\n' 의 개수를 돌려준다. 열기 실패 시 -1. */
static long count_lines(const char *path)
{
    long lines = 0;
    /* TODO 2:
     *   ① open(path, O_RDONLY) — 실패(-1)면 -1 반환
     *   ② char buf[64]; 로 read 를 반복 — read 가 0(파일 끝)을 돌려줄 때까지
     *   ③ 읽은 바이트 중 '\n' 의 개수를 lines 에 더한다
     *   ④ close 후 lines 반환                                               */

    return lines;
}

static void step5(void)
{
    FILE *fp = fopen("sensor.csv", "w");
    for (int i = 1; i <= 25; i++)
        fprintf(fp, "S%d,%d.%d\n", i % 3 + 1, 20 + i, i % 10);
    fclose(fp);

    printf("[5단계] 결과 : lines=%ld, 없는 파일=%ld\n",
           count_lines("sensor.csv"), count_lines("no_such.csv"));
    printf("[5단계] 기대값: lines=25, 없는 파일=-1\n\n");
}

/* ───────────── [어려움] 6단계: 쓰레드 4개로 부분합 ─────────────
 * 1 ~ 1000 을 네 구간으로 나눠 쓰레드 4개가 각자 더하고, main 이 합친다.
 * 쓰레드마다 자기 칸(sum)에만 쓰므로 mutex 가 필요 없다. */
typedef struct {
    int  from;        /* 시작 값 (포함) */
    int  to;          /* 끝 값 (포함)   */
    long sum;         /* 결과           */
} range_t;

static void *sum_worker(void *arg)
{
    /* TODO 3:
     *   ① arg 를 range_t * 로 바꾼다
     *   ② from ~ to 까지 더한 값을 그 구조체의 sum 에 저장한다
     *   ③ NULL 을 반환한다                                                */

    return NULL;
}

static void step6(void)
{
    range_t r[4] = { {1, 250, 0}, {251, 500, 0}, {501, 750, 0}, {751, 1000, 0} };
    pthread_t t[4];
    for (int i = 0; i < 4; i++) pthread_create(&t[i], NULL, ______, &r[i]);   /* ① 쓰레드가 실행할 함수 */
    for (int i = 0; i < 4; i++) ______(t[i], NULL);                           /* ② 쓰레드 회수 */

    long total = r[0].sum + r[1].sum + r[2].sum + r[3].sum;
    printf("[6단계] 결과 : 구간합=%ld/%ld/%ld/%ld, total=%ld\n",
           r[0].sum, r[1].sum, r[2].sum, r[3].sum, total);
    printf("[6단계] 기대값: 구간합=31375/93875/156375/218875, total=500500\n\n");
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
