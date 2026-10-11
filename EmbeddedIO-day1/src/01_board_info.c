/* 01_board_info.c — 보드 정보를 "파일"로 읽기 (/proc, /sys, uname)
 * 빌드: make 01_board_info
 * 실행: ./01_board_info
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <unistd.h>

/* 파일의 첫 줄을 buf 에 읽고 끝의 '\n' 을 지운다. 실패하면 -1 */
static int read_line(const char *path, char *buf, size_t n)
{
    FILE *fp = fopen(path, "r");
    if (fp == NULL)
        return -1;
    if (fgets(buf, (int)n, fp) == NULL) {
        fclose(fp);
        return -1;
    }
    fclose(fp);
    buf[strcspn(buf, "\n")] = '\0';
    return 0;
}

int main(void)
{
    char buf[128];
    struct utsname u;

    /* 1) Device Tree 가 알려 주는 보드 모델 */
    if (read_line("/proc/device-tree/model", buf, sizeof buf) == 0)
        printf("모델      : %s\n", buf);
    else
        printf("모델      : (읽기 실패 — Raspberry Pi 가 아님?)\n");

    /* 2) 커널 정보 — 시스템 콜 uname() */
    if (uname(&u) == 0)
        printf("커널      : %s %s (%s)\n", u.sysname, u.release, u.machine);

    /* 3) 온라인 CPU 코어 수 */
    printf("CPU 코어  : %ld\n", sysconf(_SC_NPROCESSORS_ONLN));

    /* 4) SoC 온도 — thermal 드라이버가 밀리도(m°C) 정수로 보여 준다 */
    if (read_line("/sys/class/thermal/thermal_zone0/temp", buf, sizeof buf) == 0)
        printf("SoC 온도  : %.1f °C\n", atoi(buf) / 1000.0);

    /* 5) GPIO 칩 장치 파일을 내 권한으로 열 수 있는가 */
    if (access("/dev/gpiochip0", F_OK) != 0)
        printf("GPIO 칩   : /dev/gpiochip0 없음\n");
    else if (access("/dev/gpiochip0", R_OK | W_OK) == 0)
        printf("GPIO 칩   : /dev/gpiochip0 읽기·쓰기 가능\n");
    else
        printf("GPIO 칩   : /dev/gpiochip0 권한 없음 (gpio 그룹 확인)\n");

    return 0;
}
