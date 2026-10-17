/* lcd.h — 1602A LCD (HD44780) 4비트 모드 드라이버 — libgpiod
 *
 *   배선: RS → GPIO12 · E → GPIO16 · D4 → GPIO20 · D5 → GPIO21 · D6 → GPIO26 · D7 → GPIO22
 *         VSS · RW · K → GND,  VDD → 5V,  A → 220Ω → 5V,  V0 → 가변저항 가운데 (양 끝 3V3 · GND)
 *
 *   lcd_t *l = lcd_open();
 *   lcd_printf(l, 0, "T %4.1f", t);     (행, 형식) — 16칸을 공백으로 채워 덮어쓴다
 *   lcd_custom(l, 1, bitmap8);          사용자 문자 1~7 → 문자열 속 "\x01"
 *   lcd_close(l);
 */
#ifndef LCD_H
#define LCD_H
#include <stdint.h>

typedef struct lcd lcd_t;

lcd_t *lcd_open(void);
void lcd_close(lcd_t *l);                       /* 화면 지우고 반납 */
void lcd_release(lcd_t *l);                     /* 화면은 그대로 두고 반납 */
void lcd_cmd(lcd_t *l, uint8_t c);              /* RS = 0 */
void lcd_putc(lcd_t *l, char c);                /* RS = 1 */
void lcd_clear(lcd_t *l);
void lcd_goto(lcd_t *l, int col, int row);
void lcd_puts(lcd_t *l, const char *s);
void lcd_printf(lcd_t *l, int row, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
void lcd_custom(lcd_t *l, int slot, const uint8_t bitmap[8]);

#endif
