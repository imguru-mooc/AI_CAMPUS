/* 04_speed.c — 같은 핀을 libgpiod 와 mmap 레지스터로 N 번 토글해 속도 비교
 * 배선: GPIO17 LED (깜빡임이 너무 빨라 눈에는 반쯤 켜진 것처럼 보인다)
 * 실행: ./04_speed            ./04_speed 2000000
 */
#include "gpio_common.h"
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#define LED 17

int main(int argc, char **argv)
{
    long n = (argc > 1) ? atol(argv[1]) : 1000000;
    unsigned int off = LED;

    /* ① libgpiod: 토글마다 ioctl 시스템 콜 1번 */
    struct gpiod_line_request *req = gpio_request_output("speed", &off, 1);
    if (!req) return EXIT_FAILURE;
    long long t0 = now_ns();
    for (long i = 0; i < n; i++)
        gpiod_line_request_set_value(req, off, (i & 1) ? GPIOD_LINE_VALUE_ACTIVE
                                                       : GPIOD_LINE_VALUE_INACTIVE);
    long long t1 = now_ns();
    gpiod_line_request_release(req);        /* mmap 실험 전에 반납 */

    /* ② mmap: 토글마다 메모리 쓰기 1번 (커널을 거치지 않음) */
    int fd = open("/dev/gpiomem", O_RDWR | O_SYNC);
    if (fd < 0) { perror("open /dev/gpiomem"); return EXIT_FAILURE; }
    volatile uint32_t *g = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (g == MAP_FAILED) { perror("mmap"); return EXIT_FAILURE; }
    g[1] = (g[1] & ~(7u << 21)) | (1u << 21);           /* GPFSEL1: GPIO17 = 출력 */
    long long t2 = now_ns();
    for (long i = 0; i < n; i++) {
        if (i & 1) g[0x1C / 4] = 1u << LED;             /* GPSET0 */
        else       g[0x28 / 4] = 1u << LED;             /* GPCLR0 */
    }
    long long t3 = now_ns();
    g[0x28 / 4] = 1u << LED;
    g[1] &= ~(7u << 21);                                /* 다시 입력으로 */
    munmap((void *)g, 4096);

    double a = (t1 - t0) / (double)n, b = (t3 - t2) / (double)n;
    printf("토글 %ld 번\n", n);
    printf("  libgpiod : %8.1f ns/회  (%6.2f MHz)\n", a, 1000.0 / a);
    printf("  mmap     : %8.1f ns/회  (%6.2f MHz)\n", b, 1000.0 / b);
    printf("  mmap 이 약 %.0f 배 빠름\n", a / b);
    return EXIT_SUCCESS;
}
