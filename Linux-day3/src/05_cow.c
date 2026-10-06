/* 05_cow.c — fork 후 Copy-on-Write: 자식이 쓴 페이지만 복사된다
 * 빌드: make 05_cow
 * 실행: ./05_cow 0 / 10 / 50 / 100      (자식이 버퍼의 몇 % 를 쓸지)
 */
#include "memutil.h"
#include <unistd.h>
#include <sys/wait.h>

#define SIZE (50L * 1024 * 1024)

int main(int argc, char *argv[])
{
    int pct = (argc > 1) ? atoi(argv[1]) : 50;
    long page = sysconf(_SC_PAGESIZE);
    char *buf = malloc(SIZE);
    if (buf == NULL)
        return 1;
    memset(buf, 1, SIZE);                       /* 부모가 미리 모두 채움 */

    pid_t pid = fork();
    if (pid == 0) {
        long f0 = minflt();
        long limit = SIZE * pct / 100;
        for (long i = 0; i < limit; i += page)
            buf[i] = 2;                         /* 쓰는 순간 그 페이지만 복사 */
        long faults = minflt() - f0;
        printf("자식이 %3d %% (%5ld 페이지) 씀 → minflt %+ld\n", pct, limit / page, faults);
        exit(0);
    }
    wait(NULL);
    printf("부모 buf[0] = %d (자식이 써도 부모 값은 그대로)\n", buf[0]);
    free(buf);
    return 0;
}
