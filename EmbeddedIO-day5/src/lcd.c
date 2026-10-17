/* lcd.c — lcd.h 구현 (HD44780 4비트)
 *
 * HD44780 의 타이밍은 "최소 시간" 만 있다 (E 폭 ≥ 450 ns, 명령 뒤 ≥ 37 µs, clear 뒤 ≥ 1.52 ms).
 * 늦어지는 것은 괜찮다 → 스케줄링 지연이 있는 사용자 공간에서도 동작한다. (DHT11 과 다른 점)
 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "gpio_common.h"
#include "lcd.h"

enum { RS, E, D4, D5, D6, D7, NPIN };
static const unsigned int PINS[NPIN] = { 12, 16, 20, 21, 26, 22 };

struct lcd {
    struct gpiod_line_request *req;
    enum gpiod_line_value v[NPIN];
};

static void us_sleep(long us)
{
    struct timespec ts = { us / 1000000, (us % 1000000) * 1000 };
    while (nanosleep(&ts, &ts) < 0 && errno == EINTR) ;      /* Ctrl+C 로 짧아지지 않게 */
}

static void apply(lcd_t *l) { gpiod_line_request_set_values(l->req, l->v); }

/* 4비트 하나: D4~D7 에 올리고 E 를 High → Low (떨어지는 순간 LCD 가 읽음) */
static void nibble(lcd_t *l, uint8_t n)
{
    for (int i = 0; i < 4; i++) l->v[D4 + i] = (n >> i) & 1 ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE;
    l->v[E] = GPIOD_LINE_VALUE_ACTIVE;   apply(l);   us_sleep(1);
    l->v[E] = GPIOD_LINE_VALUE_INACTIVE; apply(l);   us_sleep(1);
}

static void send(lcd_t *l, uint8_t b, int rs)
{
    l->v[RS] = rs ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE;
    nibble(l, b >> 4);                  /* 위 4비트 먼저 */
    nibble(l, b & 0x0F);                /* 아래 4비트 */
    us_sleep(50);                       /* 명령 실행 ≥ 37 µs */
}

void lcd_cmd(lcd_t *l, uint8_t c) { send(l, c, 0); }
void lcd_putc(lcd_t *l, char c)   { send(l, (uint8_t)c, 1); }
void lcd_clear(lcd_t *l)          { lcd_cmd(l, 0x01); us_sleep(2000); }
void lcd_goto(lcd_t *l, int col, int row) { lcd_cmd(l, (uint8_t)(0x80 | ((row ? 0x40 : 0x00) + col))); }
void lcd_puts(lcd_t *l, const char *s) { while (*s) lcd_putc(l, *s++); }

void lcd_printf(lcd_t *l, int row, const char *fmt, ...)
{
    char buf[17];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    int n = (int)strlen(buf);
    for (; n < 16; n++) buf[n] = ' ';   /* 남은 칸은 공백 — 이전 글자가 남지 않게 */
    buf[16] = '\0';
    lcd_goto(l, 0, row);
    lcd_puts(l, buf);
}

void lcd_custom(lcd_t *l, int slot, const uint8_t bitmap[8])
{
    lcd_cmd(l, (uint8_t)(0x40 | ((slot & 7) << 3)));        /* CGRAM 주소 */
    for (int i = 0; i < 8; i++) send(l, bitmap[i] & 0x1F, 1);
    lcd_goto(l, 0, 0);                                       /* 다시 DDRAM 으로 */
}

lcd_t *lcd_open(void)
{
    lcd_t *l = calloc(1, sizeof *l);
    if (!l) return NULL;
    l->req = gpio_request_output("lcd", PINS, NPIN);
    if (!l->req) { free(l); return NULL; }

    us_sleep(50000);                    /* 전원 후 40 ms 이상 */
    l->v[RS] = GPIOD_LINE_VALUE_INACTIVE;
    nibble(l, 0x3); us_sleep(4500);     /* 8비트 모드라고 세 번 말해 상태를 맞춘다 */
    nibble(l, 0x3); us_sleep(150);
    nibble(l, 0x3); us_sleep(150);
    nibble(l, 0x2); us_sleep(150);      /* 이제 4비트 모드 */
    lcd_cmd(l, 0x28);                   /* 4비트 · 2줄 · 5×8 */
    lcd_cmd(l, 0x08);                   /* 화면 끔 */
    lcd_clear(l);
    lcd_cmd(l, 0x06);                   /* 쓰면 커서 오른쪽으로 */
    lcd_cmd(l, 0x0C);                   /* 화면 켬 · 커서 숨김 */
    return l;
}

void lcd_close(lcd_t *l)
{
    if (!l) return;
    lcd_clear(l);
    for (int i = 0; i < NPIN; i++) l->v[i] = GPIOD_LINE_VALUE_INACTIVE;
    apply(l);
    gpiod_line_request_release(l->req);
    free(l);
}

void lcd_release(lcd_t *l)
{
    if (!l) return;
    gpiod_line_request_release(l->req);     /* 출력은 마지막 값 그대로 — LCD 는 글자를 유지한다 */
    free(l);
}
