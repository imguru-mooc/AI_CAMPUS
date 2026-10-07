/* 05_inotify.c — 디렉터리의 파일 변화를 이벤트로 받기
 * 빌드: make 05_inotify
 * 실행: mkdir -p watch && ./05_inotify watch
 *       (두 번째 PuTTY 창) echo hi > watch/a.txt ; mv watch/a.txt watch/b.txt ; rm watch/b.txt
 *       FileZilla 로 watch 폴더에 파일을 올려 봐도 된다. Ctrl+C 로 종료
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/inotify.h>

int main(int argc, char *argv[])
{
    const char *dir = (argc > 1) ? argv[1] : ".";
    int fd = inotify_init1(0);
    if (fd == -1) {
        perror("inotify_init1");
        return 1;
    }
    int wd = inotify_add_watch(fd, dir, IN_CREATE | IN_MODIFY | IN_CLOSE_WRITE |
                                        IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO);
    if (wd == -1) {
        perror(dir);
        return 1;
    }
    printf("%s 감시 중 (Ctrl+C 로 종료)\n", dir);
    fflush(stdout);

    char buf[4096] __attribute__((aligned(__alignof__(struct inotify_event))));
    for (;;) {
        ssize_t len = read(fd, buf, sizeof(buf));     /* 이벤트가 올 때까지 잠듦 */
        if (len <= 0)
            break;
        for (char *p = buf; p < buf + len; ) {
            struct inotify_event *ev = (struct inotify_event *)p;
            const char *what =
                (ev->mask & IN_CREATE)      ? "CREATE     " :
                (ev->mask & IN_MODIFY)      ? "MODIFY     " :
                (ev->mask & IN_CLOSE_WRITE) ? "CLOSE_WRITE" :
                (ev->mask & IN_DELETE)      ? "DELETE     " :
                (ev->mask & IN_MOVED_FROM)  ? "MOVED_FROM " :
                (ev->mask & IN_MOVED_TO)    ? "MOVED_TO   " : "기타       ";
            printf("%s %s\n", what, ev->len ? ev->name : "");
            fflush(stdout);
            p += sizeof(struct inotify_event) + ev->len;
        }
    }
    close(fd);
    return 0;
}
