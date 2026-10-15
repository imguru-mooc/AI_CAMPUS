/* 06_seg4.c — 4자리 7-Segment 멀티플렉싱 (표시 쓰레드)
 *
 *   ./06_seg4              0000 → 9999 카운터
 *   ./06_seg4 -l           PCF8591 AIN0(조도) 를 표시
 *   ./06_seg4 -f           표시 쓰레드를 SCHED_FIFO 로 (setcap cap_sys_nice,cap_ipc_lock+ep)
 *   ./06_seg4 -t 1234      글자 고정 (예: -t 23.4 · -t H-12)
 */
#include <stdlib.h>
#include "seg4.h"
#include "pcf8591.h"

int main(int argc, char **argv)
{
    int fifo = 0, light = 0;
    const char *text = NULL;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-f")) fifo = 1;
        else if (!strcmp(argv[i], "-l")) light = 1;
        else if (!strcmp(argv[i], "-t") && i + 1 < argc) text = argv[++i];
    }
    gpio_install_sigint();
    int spi = spi_open(SPI_DEV, 1000000);
    int pcf = light ? pcf_open("/dev/i2c-1", PCF_ADDR) : -1;
    if (spi < 0 || (light && pcf < 0)) return EXIT_FAILURE;

    seg4_t d;
    if (seg4_start(&d, spi, fifo) < 0) return EXIT_FAILURE;
    printf("4자리 표시%s — Ctrl+C 로 종료\n", fifo ? " (SCHED_FIFO)" : "");

    char buf[8];
    for (int n = 0; !g_stop; n = (n + 1) % 10000) {
        if (text) seg4_text(&d, text);
        else if (light) {
            int v = pcf_read(pcf, 0);
            snprintf(buf, sizeof buf, "%d", v < 0 ? 0 : v * 100 / 255);
            seg4_text(&d, v < 0 ? "Err" : buf);
        } else {
            snprintf(buf, sizeof buf, "%04d", n);
            seg4_text(&d, buf);
        }
        sleep_ms(100);
        if (n % 10 == 9) { printf("\r최대 지연 %ld µs   ", d.max_late_us); fflush(stdout); d.max_late_us = 0; }
    }
    seg4_stop(&d);
    if (pcf >= 0) close(pcf);
    close(spi);
    printf("\n종료\n");
    return EXIT_SUCCESS;
}
