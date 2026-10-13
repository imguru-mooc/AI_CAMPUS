/* react.c — 미니 프로젝트: 반응속도 게임 (MCU Day 1 과 같은 규칙)
 *
 *   1) "준비" 후 2~5초 무작위로 기다린다
 *   2) LED(GPIO17) 가 켜지는 순간부터 버튼을 누를 때까지의 시간을 잰다
 *      — 측정은 커널 엣지 타임스탬프 − LED 를 켠 시각 (둘 다 CLOCK_MONOTONIC)
 *   3) LED 가 켜지기 전에 누르면 부정 출발 → 부저 길게
 *   4) 라운드가 끝나면 평균 · 최고 기록을 출력하고 react_log.csv 에 추가
 *
 * 배선: GPIO17 LED · GPIO5 버튼 A · GPIO6 버튼 B(2인용) · GPIO13 액티브 부저
 * 실행: ./react            (1인, 5라운드)
 *       ./react 10         (1인, 10라운드)
 *       ./react -2         (2인 대전: 먼저 누른 사람 승, 부정 출발은 패)
 */
#include "gpio_common.h"
#include <poll.h>
#include <stdlib.h>

#define LED 17
#define BTN_A 5
#define BTN_B 6
#define BUZ 13

static struct gpiod_line_request *g_out, *g_in;
static struct gpiod_edge_event_buffer *g_buf;

static void set_line(unsigned int off, int on)
{
    gpiod_line_request_set_value(g_out, off, on ? GPIOD_LINE_VALUE_ACTIVE
                                                : GPIOD_LINE_VALUE_INACTIVE);
}

static void beep(int ms) { set_line(BUZ, 1); sleep_ms(ms); set_line(BUZ, 0); }

/* 쌓여 있던 이벤트 버리기 (이전 라운드에서 늦게 뗀 것 등) */
static void drain(void)
{
    struct pollfd p = { .fd = gpiod_line_request_get_fd(g_in), .events = POLLIN };
    while (poll(&p, 1, 0) > 0)
        gpiod_line_request_read_edge_events(g_in, g_buf, 16);
}

/* timeout_ms 동안 누름(falling) 을 기다린다.
 * 반환: 1 = 눌림 (*ts, *who 채움), 0 = 시간 초과, -1 = Ctrl+C */
static int wait_press(int timeout_ms, long long *ts, unsigned int *who)
{
    struct pollfd p = { .fd = gpiod_line_request_get_fd(g_in), .events = POLLIN };
    long long deadline = now_ns() + (long long)timeout_ms * 1000000LL;
    for (;;) {
        if (g_stop) return -1;
        int left = timeout_ms < 0 ? -1 : (int)((deadline - now_ns()) / 1000000LL);
        if (timeout_ms >= 0 && left <= 0) return 0;
        int r = poll(&p, 1, left);
        if (r < 0) { if (errno == EINTR) continue; return -1; }
        if (r == 0) return 0;
        int n = gpiod_line_request_read_edge_events(g_in, g_buf, 16);
        for (int i = 0; i < n; i++) {
            struct gpiod_edge_event *ev = gpiod_edge_event_buffer_get_event(g_buf, i);
            if (gpiod_edge_event_get_event_type(ev) != GPIOD_EDGE_EVENT_FALLING_EDGE)
                continue;
            *ts = (long long)gpiod_edge_event_get_timestamp_ns(ev);
            *who = gpiod_edge_event_get_line_offset(ev);
            return 1;
        }
    }
}

int main(int argc, char **argv)
{
    int duo = 0, rounds = 5;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-2") == 0) duo = 1;
        else rounds = atoi(argv[i]) > 0 ? atoi(argv[i]) : rounds;
    }
    gpio_install_sigint();
    srand((unsigned)now_ns());

    unsigned int outs[2] = { LED, BUZ };
    unsigned int ins[2] = { BTN_A, BTN_B };
    g_out = gpio_request_output("react", outs, 2);
    g_in  = gpio_request_input("react", ins, duo ? 2 : 1, GPIOD_LINE_EDGE_FALLING, 5000);
    g_buf = gpiod_edge_event_buffer_new(16);
    if (!g_out || !g_in || !g_buf) return EXIT_FAILURE;

    long best = 0, sum = 0; int ok = 0, fouls = 0, winA = 0, winB = 0;
    printf("반응속도 게임 — %s, %d 라운드 (Ctrl+C 로 중단)\n", duo ? "2인 대전" : "1인", rounds);

    for (int r = 1; r <= rounds && !g_stop; r++) {
        long long ts; unsigned int who;
        printf("\n[%d/%d] 준비...\n", r, rounds);
        drain();

        int delay = 2000 + rand() % 3000;                 /* 2~5초 */
        int pr = wait_press(delay, &ts, &who);
        if (pr < 0) break;
        if (pr == 1) {                                    /* LED 가 켜지기 전에 누름 */
            fouls++;
            printf("  부정 출발! (%s)\n", who == BTN_A ? "A" : "B");
            if (duo) { if (who == BTN_A) winB++; else winA++; }
            beep(800);
            continue;
        }

        set_line(LED, 1);
        long long t_on = now_ns();                        /* LED 를 켠 시각 */
        pr = wait_press(3000, &ts, &who);
        set_line(LED, 0);
        if (pr < 0) break;
        if (pr == 0) { printf("  시간 초과 (3초)\n"); continue; }

        long ms = (long)((ts - t_on) / 1000000LL);         /* 커널 타임스탬프 기준 */
        printf("  %s%ld ms\n", duo ? (who == BTN_A ? "A 승 · " : "B 승 · ") : "", ms);
        if (duo) { if (who == BTN_A) winA++; else winB++; }
        sum += ms; ok++;
        if (!best || ms < best) best = ms;
        beep(100);
    }

    set_line(LED, 0); set_line(BUZ, 0);
    printf("\n== 결과 ==\n");
    if (ok) printf("성공 %d 회 · 평균 %ld ms · 최고 %ld ms · 부정 출발 %d 회\n",
                   ok, sum / ok, best, fouls);
    if (duo) printf("A %d : %d B\n", winA, winB);

    FILE *fp = fopen("react_log.csv", "a");
    if (fp && ok) {
        time_t t = time(NULL);
        char when[32];
        strftime(when, sizeof when, "%Y-%m-%d %H:%M:%S", localtime(&t));
        fprintf(fp, "%s,%s,%d,%ld,%ld,%d\n", when, duo ? "duo" : "solo", ok, sum / ok, best, fouls);
    }
    if (fp) fclose(fp);

    gpiod_edge_event_buffer_free(g_buf);
    gpiod_line_request_release(g_in);
    gpiod_line_request_release(g_out);
    return EXIT_SUCCESS;
}
