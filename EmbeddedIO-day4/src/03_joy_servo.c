/* 03_joy_servo.c — 조이스틱 X 로 서보 각도, 네 채널을 I2C_RDWR 로 한 번에
 *
 *   배선: 조이스틱 VRx → IN1 · VRy → IN2 · SW → GPIO6 · +5V 라고 쓰여 있어도 3V3 · GND
 *         가변저항은 IN0 그대로, 서보 신호 → GPIO18 (PWM0) · + → 5V · − → GND
 *   ./03_joy_servo
 *   ./03_joy_servo -c      시작할 때 가운데 값 보정 (손을 떼고 실행)
 */
#include <stdlib.h>
#include "gpio_common.h"
#include "pcf8591.h"
#include "pwm_sysfs.h"

#define DEAD 8                       /* 가운데 ±8 은 움직이지 않음 (데드존) */

static long angle_ns(int deg)
{
    if (deg < 0) deg = 0;
    if (deg > 180) deg = 180;
    return 500000L + (long)deg * 2000000L / 180;    /* 0.5 ~ 2.5 ms */
}

int main(int argc, char **argv)
{
    int calib = argc > 1 && !strcmp(argv[1], "-c");
    gpio_install_sigint();
    int fd = pcf_open("/dev/i2c-1", PCF_ADDR);
    unsigned int sw_pin = 6;
    struct gpiod_line_request *sw = gpio_request_input("joy", &sw_pin, 1, GPIOD_LINE_EDGE_NONE, 0);
    pwm_t servo;
    if (fd < 0 || !sw || pwm_open(&servo, 0, 0) < 0) return EXIT_FAILURE;
    pwm_set(&servo, 20000000L, angle_ns(90));
    pwm_enable(&servo, 1);

    int v[4], cx = 128;
    if (calib && pcf_read_all(fd, PCF_ADDR, v) == 0) { cx = v[1]; printf("가운데 X = %d\n", cx); }

    int angle = 90;
    printf("조이스틱 X → 서보, 버튼 → 90° — Ctrl+C 로 종료\n");
    while (!g_stop) {
        if (pcf_read_all(fd, PCF_ADDR, v) < 0) { printf("I2C 오류\n"); sleep_ms(500); continue; }
        int dx = v[1] - cx;
        if (abs(dx) > DEAD) angle += dx / 32;           /* 많이 밀수록 빠르게 */
        if (angle < 0) angle = 0;
        if (angle > 180) angle = 180;
        if (gpiod_line_request_get_value(sw, sw_pin) == GPIOD_LINE_VALUE_INACTIVE) angle = 90;  /* 눌림 = LOW */
        pwm_set_duty(&servo, angle_ns(angle));
        printf("\rAIN0 %3d  X %3d  Y %3d  AIN3 %3d  → %3d°  ", v[0], v[1], v[2], v[3], angle);
        fflush(stdout);
        sleep_ms(40);
    }
    pwm_close(&servo);
    gpiod_line_request_release(sw);
    close(fd);
    printf("\n종료\n");
    return EXIT_SUCCESS;
}
