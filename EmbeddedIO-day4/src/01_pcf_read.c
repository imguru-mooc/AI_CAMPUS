/* 01_pcf_read.c — PCF8591 채널 하나를 0.2초마다 읽기
 *
 *   ./01_pcf_read          AIN0 (가변저항)
 *   ./01_pcf_read 1        AIN1
 *   ./01_pcf_read 0 -n     첫 바이트를 버리지 않으면? (한 박자 늦은 값 확인)
 *
 * 배선: 모듈 Scl → SCL(물리 5) · Sda → SDA(물리 3) · Vcc → 3V3 · Gnd → GND
 *       가변저항 양 끝 → 3V3 · GND, 가운데 → 모듈 IN0
 */
#include <stdlib.h>
#include "gpio_common.h"
#include "pcf8591.h"

int main(int argc, char **argv)
{
    int ch = 0, no_discard = 0;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-n")) no_discard = 1;
        else ch = atoi(argv[i]) & 3;
    }
    gpio_install_sigint();
    int fd = pcf_open("/dev/i2c-1", PCF_ADDR);
    if (fd < 0) return EXIT_FAILURE;

    printf("AIN%d 읽기%s — Ctrl+C 로 종료\n", ch, no_discard ? " (첫 바이트 사용)" : "");
    while (!g_stop) {
        int v;
        if (no_discard) {                               /* ⚠ 비교용: 1바이트만 읽기 */
            uint8_t c = PCF_DAC_ON | ch, b;
            if (write(fd, &c, 1) != 1 || read(fd, &b, 1) != 1) v = -1; else v = b;
        } else {
            v = pcf_read(fd, ch);
        }
        if (v < 0) { printf("응답 없음 — 배선 · i2cdetect 확인\n"); sleep_ms(1000); continue; }
        int mv = pcf_mv(v);
        int bar = v * 40 / 255;
        printf("raw %3d  %4d mV  |%-40.*s|\n", v, mv, bar, "########################################");
        sleep_ms(200);
    }
    close(fd);
    printf("\n종료\n");
    return EXIT_SUCCESS;
}
