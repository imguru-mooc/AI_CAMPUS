/* spi595.h — /dev/spidev0.0 으로 74HC595 에 바이트 보내기
 *
 *   배선:  MOSI(GPIO10) → DS(14)   SCLK(GPIO11) → SH_CP(11)   CE0(GPIO8) → ST_CP(12)
 *          OE(13) → GND   MR(10) → 3V3   VCC(16) → 3V3   GND(8) → GND
 *
 *   CE0 는 전송하는 동안 LOW, 끝나면 HIGH 로 돌아간다 → 그 상승 에지가 래치(ST_CP)다.
 *   그래서 SPI 전송 한 번 = shiftOut + 래치 한 번.
 *
 *   int fd = spi_open("/dev/spidev0.0", 1000000);
 *   spi_send(fd, 0x3F);                     1바이트 = Q7..Q0
 *   rev8(x)                                 비트 순서 뒤집기 (bit7 ↔ bit0)
 */
#ifndef SPI595_H
#define SPI595_H

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#define SPI_DEV "/dev/spidev0.0"

static uint32_t g_spi_hz = 1000000;

static inline int spi_open(const char *dev, uint32_t hz)
{
    int fd = open(dev, O_RDWR | O_CLOEXEC);
    if (fd < 0) { perror(dev); return -1; }
    uint8_t mode = SPI_MODE_0, bits = 8;            /* CPOL 0 · CPHA 0: 상승 에지에서 샘플 */
    if (ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0 ||
        ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0 ||
        ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &hz) < 0) {
        perror("spi ioctl"); close(fd); return -1;
    }
    g_spi_hz = hz;
    return fd;
}

/* n 바이트를 한 번의 전송으로 — 595 가 하나면 n = 1 */
static inline int spi_send_buf(int fd, const uint8_t *tx, size_t n)
{
    struct spi_ioc_transfer tr;
    memset(&tr, 0, sizeof tr);
    tr.tx_buf = (unsigned long)tx;
    tr.len = n;
    tr.speed_hz = g_spi_hz;
    tr.bits_per_word = 8;
    return ioctl(fd, SPI_IOC_MESSAGE(1), &tr) < 0 ? -1 : 0;
}

static inline int spi_send(int fd, uint8_t b) { return spi_send_buf(fd, &b, 1); }

/* MSB 먼저 보내면 bit7 → Q7, bit0 → Q0 가 된다. Q0 에 bit7 이 와야 할 때 뒤집는다 */
static inline uint8_t rev8(uint8_t x)
{
    x = (uint8_t)((x & 0xF0) >> 4 | (x & 0x0F) << 4);
    x = (uint8_t)((x & 0xCC) >> 2 | (x & 0x33) << 2);
    x = (uint8_t)((x & 0xAA) >> 1 | (x & 0x55) << 1);
    return x;
}

/* 7-Segment 글꼴 — bit0 = a … bit6 = g, bit7 = dp  (Q0 = a … Q7 = dp 배선) */
static const uint8_t SEG_HEX[16] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,
    0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71 };
#define SEG_DP     0x80
#define SEG_MINUS  0x40
#define SEG_BLANK  0x00

/* 숫자 · 일부 글자 → 세그먼트 패턴 */
static inline uint8_t seg_char(char c)
{
    if (c >= '0' && c <= '9') return SEG_HEX[c - '0'];
    if (c >= 'a' && c <= 'f') return SEG_HEX[c - 'a' + 10];
    if (c >= 'A' && c <= 'F') return SEG_HEX[c - 'A' + 10];
    switch (c) {
    case '-': return SEG_MINUS;
    case 'H': case 'h': return 0x76;
    case 'L': case 'l': return 0x38;
    case 'o':           return 0x5C;
    case 'r':           return 0x50;
    case 'n':           return 0x54;
    case 'P': case 'p': return 0x73;
    default:            return SEG_BLANK;
    }
}

#endif
