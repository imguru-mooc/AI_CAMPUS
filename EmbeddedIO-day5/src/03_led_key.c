/* 03_led_key.c — overlay 가 만든 커널 장치 쓰기: LED 클래스 + input 이벤트
 *
 *   준비: eio.dtbo 를 overlays 에 복사, config.txt 에 dtoverlay=eio, 재부팅
 *   ./03_led_key        버튼 A 를 누를 때마다 LED 트리거를 바꾼다
 *                       none(꺼짐) → default-on → timer → heartbeat → none …
 *
 *   GPIO 를 직접 요청하지 않는다 — LED 와 버튼은 이제 커널 드라이버의 것
 */
#include <dirent.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include "gpio_common.h"

#define LED_DIR "/sys/class/leds/eio:green"

static int write_str(const char *file, const char *s)
{
    char path[128];
    snprintf(path, sizeof path, "%s/%s", LED_DIR, file);
    int fd = open(path, O_WRONLY);
    if (fd < 0) { perror(path); return -1; }
    int r = write(fd, s, strlen(s)) < 0 ? -1 : 0;
    close(fd);
    return r;
}

/* /dev/input/event* 중 이름이 "eio-keys" 인 장치 찾기 */
static int open_keys(void)
{
    DIR *d = opendir("/dev/input");
    struct dirent *e;
    while (d && (e = readdir(d))) {
        if (strncmp(e->d_name, "event", 5)) continue;
        char path[300], name[64] = "";
        snprintf(path, sizeof path, "/dev/input/%s", e->d_name);
        int fd = open(path, O_RDONLY | O_CLOEXEC);
        if (fd < 0) continue;
        ioctl(fd, EVIOCGNAME(sizeof name), name);        /* 장치 이름 묻기 */
        if (!strcmp(name, "eio-keys")) { printf("버튼 장치: %s (%s)\n", path, name); closedir(d); return fd; }
        close(fd);
    }
    if (d) closedir(d);
    return -1;
}

int main(void)
{
    const char *TRIG[] = { "none", "default-on", "timer", "heartbeat" };
    gpio_install_sigint();
    int kfd = open_keys();
    if (kfd < 0) { fprintf(stderr, "eio-keys 없음 — overlay 적용 · 재부팅 · input 그룹 확인\n"); return EXIT_FAILURE; }
    if (write_str("trigger", "none") < 0) { fprintf(stderr, "LED 쓰기 권한 확인 (sudo 또는 udev 규칙)\n"); return EXIT_FAILURE; }

    printf("버튼 A 를 누르세요 — Ctrl+C 로 종료\n");
    int k = 0;
    struct pollfd p = { .fd = kfd, .events = POLLIN };
    while (!g_stop) {
        if (poll(&p, 1, -1) <= 0) continue;
        struct input_event ev;
        if (read(kfd, &ev, sizeof ev) != sizeof ev) continue;
        if (ev.type != EV_KEY) continue;                  /* EV_SYN 등은 건너뜀 */
        printf("%ld.%06ld  code %d  value %d (%s)\n", (long)ev.input_event_sec, (long)ev.input_event_usec,
               ev.code, ev.value, ev.value == 1 ? "누름" : ev.value == 0 ? "뗌" : "반복");
        if (ev.code == KEY_ENTER && ev.value == 1) {
            k = (k + 1) % 4;
            write_str("trigger", TRIG[k]);
            printf("  → trigger = %s\n", TRIG[k]);
        }
    }
    write_str("trigger", "heartbeat");                     /* overlay 기본값으로 되돌림 */
    close(kfd);
    printf("\n종료\n");
    return EXIT_SUCCESS;
}
