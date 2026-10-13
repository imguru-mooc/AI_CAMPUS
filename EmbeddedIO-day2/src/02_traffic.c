/* 02_traffic.c — 라인 3개를 한 요청으로 묶어 신호등 상태표 재생
 * 배선: GPIO17 빨강 · GPIO27 노랑 · GPIO22 초록 (각각 220Ω → LED → GND)
 * 실행: ./02_traffic          ./02_traffic -l (active-low 로 요청해 보기)
 */
#include "gpio_common.h"
#include <stdlib.h>

static const unsigned int LINES[3] = { 17, 27, 22 };      /* R, Y, G */

/* 상태표: 각 행 = { R, Y, G, 유지 시간(ms) } */
static const struct { int r, y, g; int ms; const char *name; } STEPS[] = {
    { 0, 0, 1, 3000, "초록" },
    { 0, 1, 0, 1000, "노랑" },
    { 1, 0, 0, 3000, "빨강" },
    { 1, 1, 0, 1000, "빨강+노랑" },
};
#define NSTEPS (sizeof STEPS / sizeof STEPS[0])

int main(int argc, char **argv)
{
    int active_low = (argc > 1 && strcmp(argv[1], "-l") == 0);
    gpio_install_sigint();

    struct gpiod_line_settings *s = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(s, GPIOD_LINE_DIRECTION_OUTPUT);
    gpiod_line_settings_set_output_value(s, GPIOD_LINE_VALUE_INACTIVE);
    gpiod_line_settings_set_active_low(s, active_low);   /* 논리값 1 = 전압 0V 로 뒤집기 */

    struct gpiod_line_request *req = gpio_request("traffic", LINES, 3, s);
    gpiod_line_settings_free(s);
    if (!req) return EXIT_FAILURE;

    printf("신호등 시작%s (Ctrl+C 로 종료)\n", active_low ? " — active-low" : "");
    for (size_t i = 0; !g_stop; i = (i + 1) % NSTEPS) {
        /* values[] 의 순서 = 요청할 때 넘긴 offsets 순서 (17, 27, 22) */
        enum gpiod_line_value v[3] = {
            STEPS[i].r ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE,
            STEPS[i].y ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE,
            STEPS[i].g ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE,
        };
        gpiod_line_request_set_values(req, v);           /* 세 줄을 한 번에 */
        printf("  %-10s R=%d Y=%d G=%d\n", STEPS[i].name, STEPS[i].r, STEPS[i].y, STEPS[i].g);
        sleep_ms(STEPS[i].ms);
    }

    enum gpiod_line_value off[3] = { 0, 0, 0 };
    gpiod_line_request_set_values(req, off);
    gpiod_line_request_release(req);
    printf("\n종료\n");
    return EXIT_SUCCESS;
}
