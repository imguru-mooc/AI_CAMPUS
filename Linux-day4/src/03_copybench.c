/* 03_copybench.c — 버퍼 크기와 동기화가 파일 복사 속도에 주는 영향
 * 빌드: make 03_copybench
 * 준비: dd if=/dev/urandom of=src.bin bs=1M count=8
 * 실행: ./03_copybench src.bin 1           (1바이트씩)
 *       ./03_copybench src.bin 4096
 *       ./03_copybench src.bin 65536
 *       ./03_copybench src.bin 4096 sync    (write 마다 fsync)
 *       strace -c ./03_copybench src.bin 4096
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

static double now_ms(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1000.0 + t.tv_nsec / 1e6;
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(stderr, "사용법: %s 원본 버퍼크기 [sync]\n", argv[0]);
        return 1;
    }
    size_t bs = strtoul(argv[2], NULL, 10);
    int sync = (argc > 3 && strcmp(argv[3], "sync") == 0);
    char *buf = malloc(bs);
    int in = open(argv[1], O_RDONLY);
    int out = open("out.dat", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (!buf || in == -1 || out == -1) {
        perror("준비");
        return 1;
    }
    long calls = 0;
    long long total = 0;
    ssize_t n;
    double t0 = now_ms();
    while ((n = read(in, buf, bs)) > 0) {
        calls++;
        if (write(out, buf, n) != n) {
            perror("write");
            return 1;
        }
        calls++;
        if (sync) {
            fsync(out);                       /* 디스크에 닿을 때까지 기다림 */
            calls++;
        }
        total += n;
    }
    double ms = now_ms() - t0;
    printf("버퍼 %7zu B%s : %8.1f ms  시스템 콜 %9ld 번  %7.1f MB/s\n",
           bs, sync ? " +fsync" : "       ", ms, calls, total / 1048576.0 / (ms / 1000.0));
    close(in);
    close(out);
    free(buf);
    return 0;
}
