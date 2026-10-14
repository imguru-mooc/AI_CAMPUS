/* 06_dcmotor.c — L293D 로 DC 모터(팬) 방향 · 속도 제어
 * 배선 (L293D 핀 번호):
 *   1 EN1  → GPIO18 (PWM0, 속도)     2 IN1 → GPIO20     7 IN2 → GPIO21
 *   3 OUT1 · 6 OUT2 → 모터 두 선      4 · 5 GND → GND    16 VCC1 · 8 VCC2 → 5V
 *   버튼 A (GPIO5) = 속도 단계 0 → 40 → 70 → 100 %,  버튼 B (GPIO6) = 방향 반전
 * 실행: ./06_dcmotor
 */
#include "gpio_common.h"
#include "pwm_sysfs.h"
#include <poll.h>

#define PERIOD 50000L                      /* 50 µs = 20 kHz — 귀에 거슬리는 소리 줄이기 */
static const unsigned int DIRS[2] = { 20, 21 };     /* IN1, IN2 */
static const unsigned int BTNS[2] = { 5, 6 };
static const int STEPS[] = { 0, 40, 70, 100 };

static struct gpiod_line_request *g_dir;

/* IN1 · IN2 진리표: 10 정회전, 01 역회전, 00 자유 정지, 11 브레이크 */
static void set_dir(int in1, int in2)
{
    enum gpiod_line_value v[2] = { in1 ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE,
                                   in2 ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE };
    gpiod_line_request_set_values(g_dir, v);
}

int main(void)
{
    gpio_install_sigint();
    g_dir = gpio_request_output("dcmotor", DIRS, 2);
    struct gpiod_line_request *btn = gpio_request_input("dcmotor", BTNS, 2, GPIOD_LINE_EDGE_FALLING, 10000);
    pwm_t en;
    if (!g_dir || !btn || pwm_open(&en, 0, 0) < 0) return EXIT_FAILURE;
    pwm_set(&en, PERIOD, 0);
    pwm_enable(&en, 1);

    int step = 0, fwd = 1;
    set_dir(1, 0);
    struct gpiod_edge_event_buffer *buf = gpiod_edge_event_buffer_new(8);
    struct pollfd pfd = { .fd = gpiod_line_request_get_fd(btn), .events = POLLIN };
    printf("A = 속도, B = 방향 — Ctrl+C 로 종료\n");

    while (!g_stop) {
        if (poll(&pfd, 1, -1) <= 0) continue;
        int n = gpiod_line_request_read_edge_events(btn, buf, 8);
        for (int i = 0; i < n; i++) {
            unsigned int who = gpiod_edge_event_get_line_offset(gpiod_edge_event_buffer_get_event(buf, i));
            if (who == 5) {
                step = (step + 1) % 4;
                pwm_set_duty(&en, PERIOD / 100 * STEPS[step]);
            } else {
                pwm_set_duty(&en, 0);       /* 속도 0 → 브레이크 → 방향 바꾸기 → 원래 속도 */
                set_dir(1, 1);
                sleep_ms(300);
                fwd = !fwd;
                if (fwd) set_dir(1, 0); else set_dir(0, 1);
                pwm_set_duty(&en, PERIOD / 100 * STEPS[step]);
            }
            printf("  %s %3d %%\n", fwd ? "정회전" : "역회전", STEPS[step]);
        }
    }

    pwm_set_duty(&en, 0);
    set_dir(0, 0);
    pwm_close(&en);
    gpiod_edge_event_buffer_free(buf);
    gpiod_line_request_release(btn);
    gpiod_line_request_release(g_dir);
    printf("\n정지\n");
    return EXIT_SUCCESS;
}
