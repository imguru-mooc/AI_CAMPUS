/* 03_demand.c — malloc 은 주소만 예약, 실제 페이지는 처음 만질 때 할당 (demand paging)
 * 빌드: make 03_demand
 * 실행: ./03_demand
 */
#include "memutil.h"
#include <unistd.h>

#define MB (1024 * 1024)

static void report(const char *step, long f0)
{
    printf("%-34s VmSize %7ld kB  VmRSS %7ld kB  minflt %+7ld\n",
           step, status_kb("VmSize"), status_kb("VmRSS"), minflt() - f0);
}

int main(void)
{
    long page = sysconf(_SC_PAGESIZE);
    long f0 = minflt();
    report("시작", f0);

    char *p = malloc(100 * MB);
    if (p == NULL)
        return 1;
    report("malloc(100MB) 직후", f0);

    for (long i = 0; i < 50 * MB; i += page)   /* 앞 50MB 를 4KB 마다 1바이트씩 */
        p[i] = 1;
    report("앞 50MB, 페이지마다 1바이트 쓰기", f0);

    memset(p, 2, 100 * MB);                     /* 전체 쓰기 */
    report("memset 100MB", f0);

    printf("페이지 크기 = %ld 바이트, 50MB / 4KB = %ld 페이지\n", page, 50L * MB / page);
    free(p);
    return 0;
}
