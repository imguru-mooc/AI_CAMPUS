/* mycp.c — open/read/write/close 만으로 만든 cp
 * 빌드: make mycp
 * 실행: ./mycp /etc/os-release copy.txt && diff /etc/os-release copy.txt && echo 같음
 */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "사용법: %s 원본 사본\n", argv[0]);
        return 1;
    }
    int in = open(argv[1], O_RDONLY);
    if (in == -1) {
        perror(argv[1]);
        return 1;
    }
    int out = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out == -1) {
        perror(argv[2]);
        close(in);
        return 1;
    }
    char buf[4096];
    ssize_t n;
    while ((n = read(in, buf, sizeof(buf))) > 0) {      /* 0 = 파일 끝 */
        if (write(out, buf, n) != n) {
            perror("write");
            return 1;
        }
    }
    if (n == -1)
        perror("read");
    close(in);
    close(out);
    return 0;
}
