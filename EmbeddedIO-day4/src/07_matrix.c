/* 07_matrix.c — 8×8 도트 매트릭스: 열 = 595(SPI), 행 = GPIO 8개, 이중 버퍼
 *
 *   행 R1~R8 → GPIO 5 · 6 · 12 · 13 · 16 · 20 · 21 · 26
 *   열 C1~C8 → 1kΩ → 595 Q0~Q7   (1088AS 핀 번호는 강의 자료 표)
 *
 *   ./07_matrix            하트 → 웃는 얼굴 → "HELLO PI" 스크롤
 *   ./07_matrix -r         행 극성 반대 (그림이 반전되어 보이면)
 *   ./07_matrix -1         버퍼 하나로 (찢어짐 비교)
 *   ./07_matrix -f         행 스캔 쓰레드를 SCHED_FIFO 로
 */
#include <stdlib.h>
#include <stdatomic.h>
#include <pthread.h>
#include <sched.h>
#include <sys/mman.h>
#include "gpio_common.h"
#include "spi595.h"

static const unsigned int ROWS[8] = { 5, 6, 12, 13, 16, 20, 21, 26 };
static struct gpiod_line_request *g_rows;
static int g_spi, g_row_high = 1, g_single = 0;

static uint8_t fb[2][8];                    /* 화면 버퍼 두 장: bit7 = 왼쪽 열, 1 = 켜짐 */
static atomic_int g_front = 0;              /* 스캔 쓰레드가 보여 주는 쪽 */
static atomic_int g_swap = 0;               /* 1 = 다음 프레임 시작에 바꿔 달라 */

static void *scan_thread(void *arg)
{
    (void)arg;
    struct timespec next;
    clock_gettime(CLOCK_MONOTONIC, &next);
    enum gpiod_line_value v[8];
    for (int r = 0; !g_stop; r = (r + 1) % 8) {
        if (r == 0 && atomic_load(&g_swap)) {           /* 프레임 경계에서만 교체 */
            atomic_store(&g_front, 1 - atomic_load(&g_front));
            atomic_store(&g_swap, 0);
        }
        uint8_t row = fb[g_single ? 0 : atomic_load(&g_front)][r];
        uint8_t col = g_row_high ? (uint8_t)~row : row; /* 행이 HIGH 면 열은 LOW 일 때 켜짐 */

        for (int i = 0; i < 8; i++) v[i] = GPIOD_LINE_VALUE_INACTIVE;
        gpiod_line_request_set_values(g_rows, v);       /* ① 모든 행 끄기 */
        spi_send(g_spi, rev8(col));                     /* ② 열 데이터 (bit7 → Q0 = C1) */
        v[r] = GPIOD_LINE_VALUE_ACTIVE;
        gpiod_line_request_set_values(g_rows, v);       /* ③ 이 행만 켜기 */

        next.tv_nsec += 1500000L;                       /* 1.5 ms × 8행 = 12 ms ≈ 83 Hz */
        if (next.tv_nsec >= 1000000000L) { next.tv_nsec -= 1000000000L; next.tv_sec++; }
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);
    }
    for (int i = 0; i < 8; i++) v[i] = GPIOD_LINE_VALUE_INACTIVE;
    gpiod_line_request_set_values(g_rows, v);
    return NULL;
}

/* 뒤 버퍼에 그리고 → 교체 요청 → 스캔 쓰레드가 바꿀 때까지 기다림 */
static void show(const uint8_t img[8])
{
    if (g_single) { memcpy(fb[0], img, 8); return; }    /* ⚠ 비교용: 보이는 버퍼에 바로 */
    int back = 1 - atomic_load(&g_front);
    memcpy(fb[back], img, 8);
    atomic_store(&g_swap, 1);
    while (atomic_load(&g_swap) && !g_stop) sleep_ms(1);
}

static const uint8_t HEART[8] = { 0x00, 0x66, 0xFF, 0xFF, 0x7E, 0x3C, 0x18, 0x00 };
static const uint8_t SMILE[8] = { 0x3C, 0x42, 0xA5, 0x81, 0xA5, 0x99, 0x42, 0x3C };

/* 5×8 글꼴 (열 5개, bit7 쪽부터) — 필요한 글자만 */
static const uint8_t *glyph(char c)
{
    static const uint8_t H[8] = { 0x88, 0x88, 0x88, 0xF8, 0x88, 0x88, 0x88, 0x00 };
    static const uint8_t E[8] = { 0xF8, 0x80, 0x80, 0xF0, 0x80, 0x80, 0xF8, 0x00 };
    static const uint8_t L[8] = { 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xF8, 0x00 };
    static const uint8_t O[8] = { 0x70, 0x88, 0x88, 0x88, 0x88, 0x88, 0x70, 0x00 };
    static const uint8_t P[8] = { 0xF0, 0x88, 0x88, 0xF0, 0x80, 0x80, 0x80, 0x00 };
    static const uint8_t I[8] = { 0x70, 0x20, 0x20, 0x20, 0x20, 0x20, 0x70, 0x00 };
    static const uint8_t SP[8] = { 0 };
    switch (c) { case 'H': return H; case 'E': return E; case 'L': return L; case 'O': return O;
                 case 'P': return P; case 'I': return I; default: return SP; }
}

/* 글자열을 긴 띠로 만들어 한 열씩 왼쪽으로 민다 */
static void scroll(const char *s)
{
    int len = (int)strlen(s), width = len * 6 + 8;      /* 글자 5열 + 간격 1열, 끝에 빈 8열 */
    for (int x = 0; x < width && !g_stop; x++) {
        uint8_t img[8] = { 0 };
        for (int r = 0; r < 8; r++)
            for (int c = 0; c < 8; c++) {
                int col = x + c - 8;                    /* 오른쪽 밖에서 들어온다 */
                if (col < 0 || col >= len * 6 || col % 6 == 5) continue;
                if (glyph(s[col / 6])[r] & (0x80 >> (col % 6))) img[r] |= (uint8_t)(0x80 >> c);
            }
        show(img);
        sleep_ms(90);
    }
}

int main(int argc, char **argv)
{
    int fifo = 0;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-r")) g_row_high = 0;
        else if (!strcmp(argv[i], "-1")) g_single = 1;
        else if (!strcmp(argv[i], "-f")) fifo = 1;
    }
    gpio_install_sigint();
    g_spi = spi_open(SPI_DEV, 1000000);
    struct gpiod_line_settings *s = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(s, GPIOD_LINE_DIRECTION_OUTPUT);
    gpiod_line_settings_set_active_low(s, !g_row_high);         /* -r: 행 LOW = 켜짐 */
    gpiod_line_settings_set_output_value(s, GPIOD_LINE_VALUE_INACTIVE);
    g_rows = gpio_request("matrix", ROWS, 8, s);
    gpiod_line_settings_free(s);
    if (g_spi < 0 || !g_rows) return EXIT_FAILURE;

    pthread_attr_t at;
    pthread_attr_init(&at);
    if (fifo) {
        if (mlockall(MCL_CURRENT | MCL_FUTURE) < 0) perror("mlockall (계속 진행)");
        struct sched_param sp = { .sched_priority = 50 };
        pthread_attr_setinheritsched(&at, PTHREAD_EXPLICIT_SCHED);
        pthread_attr_setschedpolicy(&at, SCHED_FIFO);
        pthread_attr_setschedparam(&at, &sp);
    }
    pthread_t th;
    int e = pthread_create(&th, &at, scan_thread, NULL);
    if (e) { fprintf(stderr, "pthread_create: %s\n", strerror(e)); return EXIT_FAILURE; }

    printf("도트 매트릭스 (%s) — Ctrl+C 로 종료\n", g_single ? "버퍼 1장" : "이중 버퍼");
    while (!g_stop) {
        show(HEART); sleep_ms(1000);
        show(SMILE); sleep_ms(1000);
        scroll("HELLO PI");
    }
    pthread_join(th, NULL);
    spi_send(g_spi, g_row_high ? 0xFF : 0x00);      /* 열을 끈 상태로 */
    gpiod_line_request_release(g_rows);
    close(g_spi);
    printf("\n종료\n");
    return EXIT_SUCCESS;
}
