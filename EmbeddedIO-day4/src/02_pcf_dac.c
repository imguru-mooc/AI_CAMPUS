/* 02_pcf_dac.c — DAC 로 모듈의 Aout LED 밝기 조절
 *
 *   ./02_pcf_dac           0 → 255 → 0 을 반복 (숨쉬기)
 *   ./02_pcf_dac 128       값 하나로 고정 (AOUT ≈ 128/255 × 3.3 V)
 *   ./02_pcf_dac -k        가변저항(AIN0) 값을 그대로 DAC 로 — 손잡이로 밝기
 */
#include <stdlib.h>
#include "gpio_common.h"
#include "pcf8591.h"

int main(int argc, char **argv)
{
    int fixed = -1, knob = 0;
    if (argc > 1) { if (!strcmp(argv[1], "-k")) knob = 1; else fixed = atoi(argv[1]) & 0xFF; }
    gpio_install_sigint();
    int fd = pcf_open("/dev/i2c-1", PCF_ADDR);
    if (fd < 0) return EXIT_FAILURE;

    if (fixed >= 0) {
        pcf_write_dac(fd, fixed);
        printf("DAC = %d → AOUT 약 %d mV — Ctrl+C 로 끄고 종료\n", fixed, pcf_mv(fixed));
        while (!g_stop) sleep_ms(200);
    } else if (knob) {
        printf("가변저항 → DAC — Ctrl+C 로 종료\n");
        while (!g_stop) {
            int v = pcf_read(fd, 0);          /* 읽기 · 쓰기 모두 같은 0x48 장치 */
            if (v >= 0) pcf_write_dac(fd, v);
            printf("\rAIN0 %3d → AOUT %4d mV ", v, v < 0 ? 0 : pcf_mv(v));
            fflush(stdout);
            sleep_ms(30);
        }
    } else {
        printf("숨쉬기 — Ctrl+C 로 종료\n");
        for (int v = 0, d = 4; !g_stop; v += d) {
            if (v >= 255) { v = 255; d = -4; }
            if (v <= 0)   { v = 0;   d = 4; }
            pcf_write_dac(fd, v);
            sleep_ms(15);
        }
    }
    pcf_write_dac(fd, 0);
    close(fd);
    printf("\n종료\n");
    return EXIT_SUCCESS;
}
