/* 05_button_poll.c — 폴링으로 버튼 읽기 + 소프트웨어 디바운스
 * 배선: GPIO5 버튼(→GND), 내부 pull-up 사용 (외부 저항 없음)
 * 실행: ./05_button_poll          디바운스 켜고 (기본)
 *       ./05_button_poll -n       디바운스 없이 → 바운싱이 그대로 보인다
 *       ./05_button_poll -n 6     GPIO6 (기울기 센서) 를 읽기
 */
#include "gpio_common.h"
#include <stdlib.h>

#define POLL_MS     1      /* 1 ms 마다 읽기 */
#define STABLE_CNT  20     /* 20번(=20 ms) 연속 같은 값이어야 인정 */

int main(int argc, char **argv)
{
    int debounce = 1;
    unsigned int line = 5;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0) debounce = 0;
        else line = (unsigned int)atoi(argv[i]);
    }
    gpio_install_sigint();

    struct gpiod_line_request *req = gpio_request_input("button_poll", &line, 1,
                                                        GPIOD_LINE_EDGE_NONE, 0);
    if (!req) return EXIT_FAILURE;

    int stable = gpiod_line_request_get_value(req, line);   /* 확정된 값 */
    int last = stable, same = 0, presses = 0;
    printf("GPIO%u 폴링 중 (%s) — Ctrl+C 로 종료\n", line,
           debounce ? "디바운스 20 ms" : "디바운스 없음");

    while (!g_stop) {
        int v = gpiod_line_request_get_value(req, line);    /* 1 = 안 누름, 0 = 누름 */
        if (v < 0) { perror("get_value"); break; }

        if (!debounce) {
            if (v != stable) {                               /* 바뀌는 즉시 인정 */
                if (v == 0) printf("  눌림 #%d\n", ++presses);
                stable = v;
            }
        } else {
            same = (v == last) ? same + 1 : 0;               /* 같은 값이 몇 번 이어졌나 */
            last = v;
            if (same >= STABLE_CNT && v != stable) {         /* 20 ms 동안 안정 → 확정 */
                stable = v;
                if (v == 0) printf("  눌림 #%d\n", ++presses);
            }
        }
        sleep_ms(POLL_MS);
    }

    gpiod_line_request_release(req);
    printf("\n총 %d 번 눌림\n", presses);
    return EXIT_SUCCESS;
}
