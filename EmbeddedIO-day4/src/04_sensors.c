/* 04_sensors.c — 아날로그 센서 네 개 → 물리량, 이동 평균, 경보
 *
 *   IN0 포토레지스터 분압 (3V3 – 포토레지스터 – IN0 – 10kΩ – GND)   밝을수록 값이 크다
 *   IN1 LM35 Vout   (LM35 +Vs → 5V · GND → GND, 출력은 10 mV/°C 라 1.5 V 를 넘지 않음)
 *   IN2 불꽃 센서 분압 (짧은 다리 − → 3V3, 긴 다리 + → IN2 · 10kΩ → GND)
 *   IN3 수위 센서 S  (+ 는 GPIO25 — 잴 때만 켬, − → GND)
 */
#include <stdlib.h>
#include "gpio_common.h"
#include "pcf8591.h"

#define N 8                                 /* 이동 평균 길이 */
#define FLAME_ON  120                       /* 이 위면 경보 ON  */
#define FLAME_OFF  90                       /* 이 아래로 내려가야 OFF (히스테리시스) */

typedef struct { int buf[N], head, count, sum; } avg_t;

static int avg_push(avg_t *a, int v)
{
    if (a->count == N) a->sum -= a->buf[a->head];   /* 가장 오래된 값 빼기 */
    else a->count++;
    a->buf[a->head] = v;
    a->sum += v;
    a->head = (a->head + 1) % N;
    return a->sum / a->count;
}

int main(void)
{
    gpio_install_sigint();
    int fd = pcf_open("/dev/i2c-1", PCF_ADDR);
    unsigned int pwr = 25;
    struct gpiod_line_request *wp = gpio_request_output("water", &pwr, 1);
    if (fd < 0 || !wp) return EXIT_FAILURE;

    avg_t al = {0}, at = {0}, af = {0};
    int flame_alarm = 0;
    printf("조도%%   온도°C   불꽃   수위   — Ctrl+C 로 종료\n");
    while (!g_stop) {
        gpiod_line_request_set_value(wp, pwr, GPIOD_LINE_VALUE_ACTIVE);    /* 수위 센서 전원 ON */
        sleep_ms(10);
        int v[4];
        int ok = pcf_read_all(fd, PCF_ADDR, v);
        gpiod_line_request_set_value(wp, pwr, GPIOD_LINE_VALUE_INACTIVE);  /* 바로 OFF */
        if (ok < 0) { printf("I2C 오류\n"); sleep_ms(500); continue; }

        int light = avg_push(&al, v[0]) * 100 / 255;            /* 0 ~ 100 % */
        double temp = pcf_mv(avg_push(&at, v[1])) / 10.0;       /* 10 mV = 1 °C */
        int flame = avg_push(&af, v[2]);
        if (!flame_alarm && flame > FLAME_ON) flame_alarm = 1;
        else if (flame_alarm && flame < FLAME_OFF) flame_alarm = 0;
        int water = v[3] * 100 / 255;

        printf("\r%4d%%   %5.1f   %3d%s  %3d%%  ", light, temp, flame,
               flame_alarm ? " 🔥" : "   ", water);
        fflush(stdout);
        sleep_ms(250);
    }
    gpiod_line_request_set_value(wp, pwr, GPIOD_LINE_VALUE_INACTIVE);
    gpiod_line_request_release(wp);
    close(fd);
    printf("\n종료\n");
    return EXIT_SUCCESS;
}
