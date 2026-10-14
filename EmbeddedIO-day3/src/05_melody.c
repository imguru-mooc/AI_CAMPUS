/* 05_melody.c — 패시브 부저로 음계 · 멜로디 (하드웨어 PWM1, 주파수 = 1 / period)
 * 배선: 패시브 부저(+) → GPIO19 (물리 35), 부저(−) → GND
 * 실행: ./05_melody          도레미파솔라시도 → "작은 별" (전래 동요)
 *       ./05_melody 440      440 Hz(라) 를 1초
 */
#include "gpio_common.h"
#include "pwm_sysfs.h"

static pwm_t g_p;

/* 주파수(Hz)로 소리 내기 — 듀티 50% 가 가장 크게 들린다. 0 이면 쉼표 */
static void tone(int hz, int ms)
{
    if (hz > 0) {
        long period = 1000000000L / hz;
        pwm_set(&g_p, period, period / 2);
        pwm_enable(&g_p, 1);
    }
    sleep_ms(ms);
    pwm_enable(&g_p, 0);
    sleep_ms(30);                          /* 같은 음이 이어질 때 끊어 들리게 */
}

enum { C4 = 262, D4 = 294, E4 = 330, F4 = 349, G4 = 392, A4 = 440, B4 = 494, C5 = 523 };

int main(int argc, char **argv)
{
    gpio_install_sigint();
    if (pwm_open(&g_p, 0, 1) < 0) return EXIT_FAILURE;  /* pwm1 = GPIO19 */
    pwm_set(&g_p, 1000000000L / 440, 0);

    if (argc > 1) {
        int hz = atoi(argv[1]);
        printf("%d Hz (period %ld ns)\n", hz, 1000000000L / (hz > 0 ? hz : 1));
        tone(hz, 1000);
    } else {
        const int scale[] = { C4, D4, E4, F4, G4, A4, B4, C5 };
        printf("음계\n");
        for (int i = 0; i < 8 && !g_stop; i++) tone(scale[i], 250);

        /* 작은 별 (전래 멜로디): 도도솔솔라라솔 파파미미레레도 */
        const int song[][2] = {
            { C4, 400 }, { C4, 400 }, { G4, 400 }, { G4, 400 }, { A4, 400 }, { A4, 400 }, { G4, 800 },
            { F4, 400 }, { F4, 400 }, { E4, 400 }, { E4, 400 }, { D4, 400 }, { D4, 400 }, { C4, 800 },
        };
        printf("작은 별\n");
        for (size_t i = 0; i < sizeof song / sizeof song[0] && !g_stop; i++)
            tone(song[i][0], song[i][1]);
    }
    pwm_close(&g_p);
    return EXIT_SUCCESS;
}
