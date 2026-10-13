/* 07_evloop.c — epoll 하나로 여러 입력을 동시에 기다리는 이벤트 루프
 *   fd ① 버튼 A  (GPIO5, falling)       → 누른 횟수 세기
 *   fd ② PIR    (GPIO6, both)          → 움직임 시작/끝 로그
 *   fd ③ timerfd (0.5초 주기)           → LED(GPIO17) 깜빡임
 *   fd ④ stdin  ("s" Enter = 통계, "q" Enter = 종료)
 * 배선: GPIO5 버튼(→GND), GPIO6 PIR OUT (PIR VCC 는 3V3), GPIO17 LED
 */
#include "gpio_common.h"
#include <stdint.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>

enum { TAG_BTN = 1, TAG_PIR, TAG_TIMER, TAG_STDIN };

static int add_fd(int ep, int fd, int tag)
{
    struct epoll_event e = { .events = EPOLLIN, .data.u32 = (uint32_t)tag };
    return epoll_ctl(ep, EPOLL_CTL_ADD, fd, &e);
}

int main(void)
{
    unsigned int btn = 5, pir = 6, led = 17;
    gpio_install_sigint();

    struct gpiod_line_request *rb = gpio_request_input("evloop", &btn, 1,
                                                       GPIOD_LINE_EDGE_FALLING, 10000);
    /* PIR 은 스스로 0/1 을 내보내므로 bias 를 끄고 요청한다 */
    struct gpiod_line_settings *s = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(s, GPIOD_LINE_DIRECTION_INPUT);
    gpiod_line_settings_set_bias(s, GPIOD_LINE_BIAS_DISABLED);
    gpiod_line_settings_set_edge_detection(s, GPIOD_LINE_EDGE_BOTH);
    struct gpiod_line_request *rp = gpio_request("evloop", &pir, 1, s);
    gpiod_line_settings_free(s);
    struct gpiod_line_request *rl = gpio_request_output("evloop", &led, 1);
    if (!rb || !rp || !rl) return EXIT_FAILURE;

    int tfd = timerfd_create(CLOCK_MONOTONIC, 0);
    struct itimerspec its = { .it_interval = { 0, 500000000L }, .it_value = { 0, 500000000L } };
    timerfd_settime(tfd, 0, &its, NULL);

    int ep = epoll_create1(0);
    add_fd(ep, gpiod_line_request_get_fd(rb), TAG_BTN);
    add_fd(ep, gpiod_line_request_get_fd(rp), TAG_PIR);
    add_fd(ep, tfd, TAG_TIMER);
    add_fd(ep, STDIN_FILENO, TAG_STDIN);

    struct gpiod_edge_event_buffer *buf = gpiod_edge_event_buffer_new(8);
    long presses = 0, motions = 0, ticks = 0;
    int led_on = 0;
    printf("이벤트 루프 시작 — s=통계, q=종료 (Enter)\n");

    while (!g_stop) {
        struct epoll_event evs[4];
        int n = epoll_wait(ep, evs, 4, -1);            /* 준비된 fd 목록만 돌려준다 */
        if (n < 0) { if (errno == EINTR) continue; perror("epoll_wait"); break; }

        for (int i = 0; i < n; i++) {
            switch (evs[i].data.u32) {
            case TAG_BTN: {
                int k = gpiod_line_request_read_edge_events(rb, buf, 8);
                presses += k;
                printf("[버튼] %ld 번째\n", presses);
                break;
            }
            case TAG_PIR: {
                int k = gpiod_line_request_read_edge_events(rp, buf, 8);
                for (int j = 0; j < k; j++) {
                    struct gpiod_edge_event *ev = gpiod_edge_event_buffer_get_event(buf, j);
                    int rise = gpiod_edge_event_get_event_type(ev) == GPIOD_EDGE_EVENT_RISING_EDGE;
                    if (rise) motions++;
                    printf("[PIR ] %s\n", rise ? "움직임 감지" : "조용해짐");
                }
                break;
            }
            case TAG_TIMER: {
                uint64_t exp;
                if (read(tfd, &exp, sizeof exp) == sizeof exp) ticks += (long)exp;
                led_on = !led_on;
                gpiod_line_request_set_value(rl, led, led_on ? GPIOD_LINE_VALUE_ACTIVE
                                                             : GPIOD_LINE_VALUE_INACTIVE);
                break;
            }
            case TAG_STDIN: {
                char line[32];
                if (!fgets(line, sizeof line, stdin) || line[0] == 'q') g_stop = 1;
                else if (line[0] == 's')
                    printf("[통계] 버튼 %ld · 움직임 %ld · 타이머 %ld\n", presses, motions, ticks);
                break;
            }
            }
        }
    }

    gpiod_line_request_set_value(rl, led, GPIOD_LINE_VALUE_INACTIVE);
    gpiod_edge_event_buffer_free(buf);
    close(ep); close(tfd);
    gpiod_line_request_release(rb);
    gpiod_line_request_release(rp);
    gpiod_line_request_release(rl);
    printf("종료\n");
    return EXIT_SUCCESS;
}
