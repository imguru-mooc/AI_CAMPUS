/* 04_lcd_demo.c — lcd.c 모듈 시험: 글자 · 고정 폭 · 사용자 문자
 *
 *   make 04_lcd_demo   (lcd.c 와 함께 링크)
 *   ./04_lcd_demo      윗줄 "Hello, Pi 4!" · 아랫줄 카운터와 ° · 하트
 */
#include <stdlib.h>
#include "gpio_common.h"
#include "lcd.h"

static const uint8_t DEG[8]   = { 0x06, 0x09, 0x09, 0x06, 0x00, 0x00, 0x00, 0x00 };   /* ° */
static const uint8_t HEART[8] = { 0x00, 0x0A, 0x1F, 0x1F, 0x0E, 0x04, 0x00, 0x00 };

int main(void)
{
    gpio_install_sigint();
    lcd_t *l = lcd_open();
    if (!l) return EXIT_FAILURE;
    lcd_custom(l, 1, DEG);                              /* 0 번은 쓰지 않는다: 문자 0 = 문자열의 끝 */
    lcd_custom(l, 2, HEART);

    lcd_printf(l, 0, "Hello, Pi 4!");
    lcd_goto(l, 15, 0); lcd_putc(l, 2);                 /* 하트 = 사용자 문자 2 */
    printf("LCD 출력 중 — 글자가 안 보이면 가변저항(V0) 을 천천히 돌린다. Ctrl+C 로 종료\n");

    for (int n = 1200; !g_stop; n = n > 0 ? n - 7 : 1200) {
        lcd_printf(l, 1, "%4d  25.0\x01" "C", n);       /* \x01 = 사용자 문자 1 (°) — %4d 로 폭 고정 */
        sleep_ms(200);
    }
    lcd_close(l);
    printf("\n종료\n");
    return EXIT_SUCCESS;
}
