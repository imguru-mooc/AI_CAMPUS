/* 04_pagecache.c — 같은 파일을 두 번 읽으면 왜 빨라지는가 (page cache)
 * 빌드: make 04_pagecache
 * 준비: dd if=/dev/urandom of=big.bin bs=1M count=500 && sync   (sync 안 하면 dirty 라 drop 안 됨)
 * 실행: ./04_pagecache big.bin drop     (읽은 뒤 이 파일의 cache 를 버림)
 *       ./04_pagecache big.bin          (첫 읽기 — 디스크)
 *       ./04_pagecache big.bin          (두 번째 — 메모리)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

static long cached_kb(void)
{
    FILE *fp = fopen("/proc/meminfo", "r");
    char line[128];
    long v = -1;
    while (fp && fgets(line, sizeof(line), fp))
        if (strncmp(line, "Cached:", 7) == 0) {
            v = atol(line + 7);
            break;
        }
    if (fp)
        fclose(fp);
    return v;
}

static double now_ms(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1000.0 + t.tv_nsec / 1e6;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "사용법: %s 파일 [drop]\n", argv[0]);
        return 1;
    }
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror(argv[1]);
        return 1;
    }
    size_t bs = 4 * 1024 * 1024;
    char *buf = malloc(bs);
    long c0 = cached_kb();
    double t0 = now_ms();
    long long total = 0;
    ssize_t n;
    while ((n = read(fd, buf, bs)) > 0)
        total += n;
    double ms = now_ms() - t0;
    long c1 = cached_kb();
    printf("%lld MB 읽기: %8.1f ms (%7.1f MB/s)   Cached %ld → %ld kB (%+ld)\n",
           total >> 20, ms, (total >> 20) / (ms / 1000.0), c0, c1, c1 - c0);
    if (argc > 2 && strcmp(argv[2], "drop") == 0) {
        posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED);   /* 이 파일의 cache 만 버림 */
        printf("posix_fadvise(DONTNEED) → Cached %ld kB\n", cached_kb());
    }
    close(fd);
    free(buf);
    return 0;
}
