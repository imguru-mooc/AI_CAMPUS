/* 03_gpiomem.c — /dev/gpiomem 을 mmap 해 GPIO 레지스터를 직접 쓰기 (비교용 시연)
 * 배선: GPIO17 LED, GPIO5 버튼(→GND)
 * 동작: 버튼을 누르는 동안 LED 켜기 — libgpiod 없이 레지스터만으로
 *
 * ⚠ 수업 비교용 코드다. 커널이 모르는 사이에 핀 기능을 바꾸므로
 *   같은 핀을 libgpiod 로 쓰는 프로그램과 동시에 돌리면 안 된다.
 */
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

/* BCM2711 GPIO 레지스터 (32비트 워드 인덱스 = 바이트 오프셋 / 4) */
#define GPFSEL0   0                  /* 0x00 : GPIO 0~9   기능 선택, 핀당 3비트 */
#define GPSET0    (0x1C / 4)         /* 0x1C : 1 을 쓴 비트의 핀 = High */
#define GPCLR0    (0x28 / 4)         /* 0x28 : 1 을 쓴 비트의 핀 = Low  */
#define GPLEV0    (0x34 / 4)         /* 0x34 : 핀 현재 레벨 읽기 */
#define PUPPDN0   (0xE4 / 4)         /* 0xE4 : GPIO 0~15 pull 설정, 핀당 2비트 (BCM2711) */

#define LED 17
#define BTN 5

static volatile sig_atomic_t g_stop = 0;
static void on_sigint(int s) { (void)s; g_stop = 1; }

static void set_function(volatile uint32_t *gpio, int pin, int fsel)
{
    int reg = GPFSEL0 + pin / 10;          /* GPIO17 → GPFSEL1 */
    int shift = (pin % 10) * 3;            /* GPIO17 → 비트 21~23 */
    uint32_t v = gpio[reg];
    v &= ~(7u << shift);                   /* 3비트 지우기 */
    v |= ((uint32_t)fsel << shift);        /* 000 = 입력, 001 = 출력 */
    gpio[reg] = v;
}

int main(void)
{
    signal(SIGINT, on_sigint);

    int fd = open("/dev/gpiomem", O_RDWR | O_SYNC);
    if (fd < 0) { perror("open /dev/gpiomem"); return EXIT_FAILURE; }

    volatile uint32_t *gpio = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);                              /* 매핑은 fd 를 닫아도 남는다 */
    if (gpio == MAP_FAILED) { perror("mmap"); return EXIT_FAILURE; }

    set_function(gpio, LED, 1);             /* GPIO17 = 출력 */
    set_function(gpio, BTN, 0);             /* GPIO5  = 입력 */
    uint32_t p = gpio[PUPPDN0];             /* GPIO5 pull-up: 비트 10~11 = 01 */
    p &= ~(3u << (BTN * 2));
    p |=  (1u << (BTN * 2));
    gpio[PUPPDN0] = p;

    printf("버튼을 누르는 동안 LED 켜짐 (Ctrl+C 로 종료)\n");
    while (!g_stop) {
        int pressed = !(gpio[GPLEV0] & (1u << BTN));     /* pull-up → 누르면 0 */
        if (pressed) gpio[GPSET0] = 1u << LED;            /* 읽기-수정-쓰기 불필요 */
        else         gpio[GPCLR0] = 1u << LED;
        struct timespec ts = { 0, 1000000L };             /* 1 ms 폴링 */
        nanosleep(&ts, NULL);
    }

    gpio[GPCLR0] = 1u << LED;
    set_function(gpio, LED, 0);             /* 원래대로 입력으로 돌려 두기 */
    munmap((void *)gpio, 4096);
    printf("\n종료\n");
    return EXIT_SUCCESS;
}
