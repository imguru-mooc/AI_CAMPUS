/* 06_mlock.c — mlockall 로 page fault 를 미리 없애기
 * 빌드: make 06_mlock
 * 실행: ./06_mlock            (잠그지 않음)
 *       sudo ./06_mlock lock  (mlockall(MCL_CURRENT | MCL_FUTURE))
 *       ulimit -l             (일반 사용자가 잠글 수 있는 한도, kB)
 * 실시간 루프 안에서 처음 만지는 메모리가 page fault 를 일으키는지 본다.
 */
#include "memutil.h"
#include <sys/mman.h>

#define SIZE (32L * 1024 * 1024)

int main(int argc, char *argv[])
{
    int lock = (argc > 1 && strcmp(argv[1], "lock") == 0);
    if (lock && mlockall(MCL_CURRENT | MCL_FUTURE) == -1) {
        perror("mlockall (sudo 필요)");
        return 1;
    }
    long f0 = minflt();
    char *buf = malloc(SIZE);                  /* MCL_FUTURE: 할당 즉시 실제 페이지 확보 */
    if (buf == NULL)
        return 1;
    long f1 = minflt();

    double t0 = now_ms();
    for (long i = 0; i < SIZE; i += 4096)      /* "실시간 루프" 에서 처음 만짐 */
        buf[i] = 1;
    double t1 = now_ms();
    long f2 = minflt();

    printf("%-12s malloc 중 minflt %+6ld | 루프 중 minflt %+6ld | 루프 시간 %6.2f ms | VmLck %ld kB\n",
           lock ? "mlockall" : "잠금 없음", f1 - f0, f2 - f1, t1 - t0, status_kb("VmLck"));
    free(buf);
    return 0;
}
