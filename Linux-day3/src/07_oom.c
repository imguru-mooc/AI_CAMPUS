/* 07_oom.c — 메모리를 계속 쓰다가 한도를 넘으면 OOM killer 에게 죽는다
 * 빌드: make 07_oom
 * 실행 (반드시 cgroup 한도 안에서!):
 *   sudo systemd-run --scope -p MemoryMax=100M -p MemorySwapMax=0 ./07_oom
 *   sudo dmesg | tail -5        (oom-kill 기록 확인)
 */
#include "memutil.h"
#include <unistd.h>

#define CHUNK (10L * 1024 * 1024)

int main(void)
{
    FILE *fp = fopen("/proc/sys/vm/overcommit_memory", "r");
    int oc = -1;
    if (fp) {
        if (fscanf(fp, "%d", &oc) != 1)
            oc = -1;
        fclose(fp);
    }
    printf("overcommit_memory = %d (0=추정 허용, 1=항상 허용, 2=엄격)\n", oc);

    for (int i = 1; i <= 100; i++) {
        char *p = malloc(CHUNK);
        if (p == NULL) {
            printf("malloc 실패 (%d MB 째)\n", i * 10);
            return 1;
        }
        memset(p, 1, CHUNK);                    /* 실제로 써야 RSS 가 는다 */
        printf("%4d MB 사용, VmRSS %6ld kB\n", i * 10, status_kb("VmRSS"));
        fflush(stdout);
        usleep(100000);
    }
    return 0;
}
