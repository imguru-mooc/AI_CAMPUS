/* 01_fd.c — 파일 디스크립터 번호와 dup2 리다이렉션
 * 빌드: make 01_fd
 * 실행: ./01_fd            (화면에는 앞부분만, 뒷부분은 out.txt 에)
 *       cat out.txt
 */
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

static void show_fds(const char *title)
{
    DIR *d = opendir("/proc/self/fd");
    struct dirent *e;
    char path[300], target[256];
    printf("--- %s ---\n", title);
    while ((e = readdir(d)) != NULL) {
        if (e->d_name[0] == '.')
            continue;
        snprintf(path, sizeof(path), "/proc/self/fd/%s", e->d_name);
        ssize_t n = readlink(path, target, sizeof(target) - 1);
        if (n < 0)
            continue;
        target[n] = '\0';
        printf("fd %-3s → %s\n", e->d_name, target);
    }
    closedir(d);
}

int main(void)
{
    int a = open("/etc/hostname", O_RDONLY);
    int b = open("out.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    printf("새로 연 fd: a=%d, b=%d  (0·1·2 다음 빈 번호)\n", a, b);
    show_fds("open 두 개 후");      /* opendir 이 쓰는 fd 도 하나 보인다 */

    close(a);
    int c = open("/etc/os-release", O_RDONLY);
    printf("a 를 닫고 다시 열면 c=%d (가장 작은 빈 번호 재사용)\n", c);

    int saved = dup(1);              /* 원래 화면(stdout)을 보관 */
    dup2(b, 1);                      /* 이제 fd 1 = out.txt */
    printf("이 줄은 화면이 아니라 out.txt 로 갑니다\n");
    fflush(stdout);
    dup2(saved, 1);                  /* fd 1 을 화면으로 되돌림 */
    printf("다시 화면으로 돌아왔습니다. cat out.txt 로 확인하세요\n");

    close(b);
    close(c);
    close(saved);
    return 0;
}
