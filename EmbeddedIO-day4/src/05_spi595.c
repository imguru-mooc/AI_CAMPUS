/* 05_spi595.c — SPI 로 74HC595 → 1자리 7-Segment 에 0~F
 *
 *   ./05_spi595            0 → F 카운터 (0.5초)
 *   ./05_spi595 -s         세그먼트 a → dp 를 하나씩 (배선 확인용)
 *   ./05_spi595 0x3F       바이트 하나를 그대로
 *
 * 배선: spi595.h 머리말 · 595 Q0~Q7 → 1kΩ → a~dp · 7-Segment COM → GND (공통 음극)
 */
#include <stdlib.h>
#include "gpio_common.h"
#include "spi595.h"

int main(int argc, char **argv)
{
    gpio_install_sigint();
    int fd = spi_open(SPI_DEV, 1000000);           /* 1 MHz — 1바이트 전송 8 µs */
    if (fd < 0) return EXIT_FAILURE;

    if (argc > 1 && strcmp(argv[1], "-s")) {
        uint8_t b = (uint8_t)strtol(argv[1], NULL, 0);
        spi_send(fd, b);
        printf("보냄: 0x%02X — Ctrl+C 로 끄고 종료\n", b);
        while (!g_stop) sleep_ms(200);
    } else if (argc > 1) {
        const char *NAME = "abcdefgP";
        printf("세그먼트 하나씩 — Ctrl+C 로 종료\n");
        for (int i = 0; !g_stop; i = (i + 1) % 8) {
            spi_send(fd, (uint8_t)(1u << i));
            printf("Q%d = %c  (0x%02X)\n", i, NAME[i], 1u << i);
            sleep_ms(500);
        }
    } else {
        printf("0 ~ F 카운터 — Ctrl+C 로 종료\n");
        for (int n = 0; !g_stop; n = (n + 1) % 16) {
            uint8_t p = SEG_HEX[n] | ((n & 1) ? SEG_DP : 0);   /* 홀수일 때 점 */
            if (spi_send(fd, p) < 0) { perror("SPI_IOC_MESSAGE"); break; }
            printf("%X → 0x%02X\n", n, p);
            sleep_ms(500);
        }
    }
    spi_send(fd, 0x00);
    close(fd);
    printf("종료\n");
    return EXIT_SUCCESS;
}
