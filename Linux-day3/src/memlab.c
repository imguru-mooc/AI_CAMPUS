/* memlab.c — Day 3 종합: 실험을 골라 page fault · RSS · 시간을 CSV 한 줄로
 * 빌드: make memlab
 * 실행: ./memlab demand | cow | mmap | lock      (lock 은 sudo)
 *       ./memlab -h
 * 출력: exp,size_mb,minflt,majflt,rss_kb,ms
 */
#include "memutil.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define MB (1024L * 1024)
static long PAGE;

static void touch(char *p, long size)
{
    for (long i = 0; i < size; i += PAGE)
        p[i] = 1;
}

static void row(const char *exp, long mb, long f0, long j0, double t0)
{
    printf("%s,%ld,%ld,%ld,%ld,%.2f\n", exp, mb, minflt() - f0, majflt() - j0,
           status_kb("VmRSS"), now_ms() - t0);
}

int main(int argc, char *argv[])
{
    const char *e = (argc > 1) ? argv[1] : "-h";
    long mb = (argc > 2) ? atol(argv[2]) : 64;
    long size = mb * MB;
    PAGE = sysconf(_SC_PAGESIZE);

    if (strcmp(e, "demand") == 0) {             /* malloc 후 처음 만질 때 fault */
        long f0 = minflt(), j0 = majflt(); double t0 = now_ms();
        char *p = malloc(size);
        touch(p, size);
        row(e, mb, f0, j0, t0);
        free(p);
    } else if (strcmp(e, "cow") == 0) {         /* fork 후 자식이 전부 쓸 때 fault */
        char *p = malloc(size);
        touch(p, size);
        if (fork() == 0) {
            long f0 = minflt(), j0 = majflt(); double t0 = now_ms();
            touch(p, size);
            row(e, mb, f0, j0, t0);
            exit(0);
        }
        wait(NULL);
        free(p);
    } else if (strcmp(e, "mmap") == 0) {        /* 파일 매핑을 처음 읽을 때 fault */
        char path[] = "/tmp/memlab_XXXXXX";
        int fd = mkstemp(path);
        if (fd == -1 || ftruncate(fd, size) == -1) { perror("file"); return 1; }
        char *p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        long f0 = minflt(), j0 = majflt(); double t0 = now_ms();
        touch(p, size);
        row(e, mb, f0, j0, t0);
        munmap(p, size);
        close(fd);
        unlink(path);
    } else if (strcmp(e, "lock") == 0) {        /* mlockall 후에는 루프 중 fault 없음 */
        if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1) { perror("mlockall (sudo)"); return 1; }
        char *p = malloc(size);
        long f0 = minflt(), j0 = majflt(); double t0 = now_ms();
        touch(p, size);
        row(e, mb, f0, j0, t0);
        free(p);
    } else {
        fprintf(stderr, "사용법: %s demand|cow|mmap|lock [MB(기본 64)]\n", argv[0]);
        fprintf(stderr, "출력: exp,size_mb,minflt,majflt,rss_kb,ms\n");
        return 1;
    }
    return 0;
}
