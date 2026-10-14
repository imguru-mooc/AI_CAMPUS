/* 03_hwpwm.c — 하드웨어 PWM 으로 LED 밝기 조절 (sysfs, pwm_sysfs.h)
 * 배선: GPIO18 (물리 12, PWM0) → 220Ω → LED → GND
 * 실행: ./03_hwpwm          숨쉬기 (1 kHz)
 *       ./03_hwpwm 30       듀티 30% 고정
 * 같은 부하(yes ×4)에서 01_swpwm 과 깜빡임을 비교해 본다.
 */
#include "gpio_common.h"
#include "pwm_sysfs.h"

#define PERIOD 1000000L                    /* 1 ms = 1 kHz */

int main(int argc, char **argv)
{
    gpio_install_sigint();
    pwm_t p;
    if (pwm_open(&p, 0, 0) < 0) return EXIT_FAILURE;    /* pwmchip0 / pwm0 = GPIO18 */
    if (pwm_set(&p, PERIOD, 0) < 0) return EXIT_FAILURE;
    pwm_enable(&p, 1);

    if (argc > 1) {
        int d = atoi(argv[1]);
        pwm_set_duty(&p, PERIOD / 100 * d);
        printf("HW PWM 1 kHz, 듀티 %d%% — Ctrl+C 로 종료\n", d);
        while (!g_stop) sleep_ms(100);
    } else {
        printf("HW PWM 1 kHz 숨쉬기 — Ctrl+C 로 종료\n");
        int d = 0, dir = 1;
        while (!g_stop) {
            pwm_set_duty(&p, PERIOD / 100 * d);     /* 파형은 하드웨어가 만든다 — 우리는 값만 바꾼다 */
            d += dir;
            if (d >= 100 || d <= 0) dir = -dir;
            sleep_ms(15);
        }
    }
    pwm_close(&p);
    printf("\n종료 (pwm0 unexport)\n");
    return EXIT_SUCCESS;
}
