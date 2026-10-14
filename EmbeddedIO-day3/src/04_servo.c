/* 04_servo.c — 서보 모터 각도 제어 (하드웨어 PWM0, 50 Hz)
 * 배선: 서보 갈색(GND) → GND, 빨강(+) → 5V (물리 2), 주황(신호) → GPIO18 (물리 12)
 * 실행: ./04_servo            0° → 180° → 0° 왕복
 *       ./04_servo 90         90° 로 이동 후 유지
 *       ./04_servo 90 500 2400   끝점 보정: 0° = 500 µs, 180° = 2400 µs
 */
#include "gpio_common.h"
#include "pwm_sysfs.h"

#define PERIOD 20000000L                   /* 20 ms = 50 Hz */
static long g_min_us = 500, g_max_us = 2500;

/* 각도(0~180) → 펄스 폭(ns) : 직선 보간 */
static long angle_to_ns(int deg)
{
    if (deg < 0) deg = 0;
    if (deg > 180) deg = 180;
    long us = g_min_us + (g_max_us - g_min_us) * deg / 180;
    return us * 1000;
}

int main(int argc, char **argv)
{
    gpio_install_sigint();
    if (argc > 3) { g_min_us = atol(argv[2]); g_max_us = atol(argv[3]); }

    pwm_t p;
    if (pwm_open(&p, 0, 0) < 0) return EXIT_FAILURE;
    pwm_set(&p, PERIOD, angle_to_ns(90));
    pwm_enable(&p, 1);

    if (argc > 1) {
        int deg = atoi(argv[1]);
        pwm_set_duty(&p, angle_to_ns(deg));
        printf("%d° (펄스 %ld µs) — Ctrl+C 로 종료\n", deg, angle_to_ns(deg) / 1000);
        while (!g_stop) sleep_ms(100);
    } else {
        printf("왕복 (%ld ~ %ld µs) — Ctrl+C 로 종료\n", g_min_us, g_max_us);
        int deg = 0, dir = 1;
        while (!g_stop) {
            pwm_set_duty(&p, angle_to_ns(deg));
            if (deg % 30 == 0) printf("  %3d°  %4ld µs\n", deg, angle_to_ns(deg) / 1000);
            deg += dir * 2;
            if (deg >= 180 || deg <= 0) dir = -dir;
            sleep_ms(20);                  /* 한 주기마다 2° — 너무 빠르면 서보가 따라오지 못한다 */
        }
    }
    pwm_close(&p);
    printf("\n종료\n");
    return EXIT_SUCCESS;
}
