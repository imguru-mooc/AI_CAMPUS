/* 02_dht_iio.c — 커널 dht11 드라이버가 만든 IIO 파일 읽기
 *
 *   준비: config.txt 의 [all] 아래에  dtoverlay=dht11,gpiopin=4  → 재부팅
 *   ./02_dht_iio
 */
#include <stdlib.h>
#include "gpio_common.h"
#include "dht.h"

int main(void)
{
    gpio_install_sigint();
    dht_t d;
    if (dht_open(&d) < 0) {
        fprintf(stderr, "IIO 장치 없음 — config.txt 의 dtoverlay=dht11,gpiopin=4 와 재부팅 확인\n");
        return EXIT_FAILURE;
    }
    printf("장치: %s — Ctrl+C 로 종료\n", d.dir);
    int ok = 0, n = 0;
    while (!g_stop) {
        double t, h;
        long long t0 = now_ns();
        int r = dht_read(&d, &t, &h);
        long ms = (long)((now_ns() - t0) / 1000000);
        n++;
        if (r == 0) { ok++; printf("%5.1f °C  %4.0f %%   (%ld ms)   성공 %d / %d\n", t, h, ms, ok, n); }
        else printf("  읽기 실패 (재시도 3번)          성공 %d / %d\n", ok, n);
        sleep_ms(2000);
    }
    printf("\n종료\n");
    return EXIT_SUCCESS;
}
