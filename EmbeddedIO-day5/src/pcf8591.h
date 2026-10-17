/* pcf8591.h — PCF8591 (I2C 8비트 ADC 4채널 + DAC 1채널) 도우미
 *
 *   int fd = pcf_open("/dev/i2c-1", 0x48);   열기 + 대상 주소 지정
 *   pcf_read(fd, ch)                          채널 하나 읽기 (0 ~ 255, 실패 -1)
 *   pcf_read_all(fd, 0x48, v)                 AIN0~3 한 번에 (I2C_RDWR 묶음 트랜잭션)
 *   pcf_write_dac(fd, value)                  AOUT = value / 255 × Vref
 *   pcf_mv(raw)                               raw → mV (Vref = 3.3 V 기준)
 *
 * 제어 바이트:  bit6 = DAC 출력 켜기 · bit5~4 = 입력 방식(00 단일) · bit2 = 자동 증가 · bit1~0 = 채널
 * 읽기 첫 바이트는 "이전 변환 값" — 그래서 하나 더 읽고 첫 바이트를 버린다.
 */
#ifndef PCF8591_H
#define PCF8591_H

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#define PCF_ADDR     0x48
#define PCF_DAC_ON   0x40
#define PCF_AUTOINC  0x04
#define PCF_VREF_MV  3300            /* 모듈 Vcc = 3V3 → 기준 전압 */

static inline int pcf_open(const char *bus, int addr)
{
    int fd = open(bus, O_RDWR | O_CLOEXEC);
    if (fd < 0) { perror(bus); return -1; }
    if (ioctl(fd, I2C_SLAVE, addr) < 0) {          /* 이후 read/write 는 이 주소로 */
        perror("ioctl I2C_SLAVE"); close(fd); return -1;
    }
    return fd;
}

/* 채널 하나: 제어 바이트 쓰기 → 2바이트 읽기 → 두 번째가 지금 값 */
static inline int pcf_read(int fd, int ch)
{
    uint8_t ctrl = PCF_DAC_ON | (ch & 0x03);
    uint8_t buf[2];
    if (write(fd, &ctrl, 1) != 1) return -1;
    if (read(fd, buf, 2) != 2) return -1;
    return buf[1];
}

/* AIN0~3 을 한 번에: [쓰기 1바이트] + [다시 시작] + [읽기 5바이트] 를 커널에 한 덩어리로 넘긴다 */
static inline int pcf_read_all(int fd, uint16_t addr, int out[4])
{
    uint8_t ctrl = PCF_DAC_ON | PCF_AUTOINC | 0;   /* 0x44: 채널 0 부터 자동 증가 */
    uint8_t buf[5];
    struct i2c_msg msgs[2] = {
        { .addr = addr, .flags = 0,        .len = 1, .buf = &ctrl },
        { .addr = addr, .flags = I2C_M_RD, .len = 5, .buf = buf   },
    };
    struct i2c_rdwr_ioctl_data data = { .msgs = msgs, .nmsgs = 2 };
    if (ioctl(fd, I2C_RDWR, &data) < 0) return -1;
    for (int i = 0; i < 4; i++) out[i] = buf[i + 1];   /* buf[0] = 이전 값 → 버림 */
    return 0;
}

static inline int pcf_write_dac(int fd, uint8_t value)
{
    uint8_t buf[2] = { PCF_DAC_ON, value };
    return write(fd, buf, 2) == 2 ? 0 : -1;
}

static inline int pcf_mv(int raw) { return raw * PCF_VREF_MV / 255; }

#endif
