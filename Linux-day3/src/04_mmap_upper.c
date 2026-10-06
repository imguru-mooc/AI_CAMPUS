/* 04_mmap_upper.c — 파일을 메모리에 매핑해 대문자로 바꾸기
 * 빌드: make 04_mmap_upper
 * 실행: echo "hello mmap world" > test.txt
 *       ./04_mmap_upper test.txt           (MAP_SHARED → 파일이 바뀐다)
 *       ./04_mmap_upper test.txt private   (MAP_PRIVATE → 내 복사본만 바뀐다)
 *       cat test.txt
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "사용법: %s 파일 [private]\n", argv[0]);
        return 1;
    }
    int priv = (argc > 2 && strcmp(argv[2], "private") == 0);
    int fd = open(argv[1], O_RDWR);
    if (fd == -1) {
        perror("open");
        return 1;
    }
    struct stat st;
    fstat(fd, &st);
    if (st.st_size == 0) {
        fprintf(stderr, "빈 파일은 매핑할 수 없습니다\n");
        return 1;
    }
    char *p = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE,
                   priv ? MAP_PRIVATE : MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    close(fd);                                  /* 매핑은 fd 를 닫아도 유지된다 */

    for (off_t i = 0; i < st.st_size; i++)      /* 배열처럼 바로 고친다 */
        p[i] = toupper((unsigned char)p[i]);
    printf("메모리 안 내용: %.*s", (int)st.st_size, p);

    if (!priv)
        msync(p, st.st_size, MS_SYNC);          /* 디스크에 즉시 반영 */
    munmap(p, st.st_size);
    printf("모드: %s\n", priv ? "MAP_PRIVATE (파일은 그대로)" : "MAP_SHARED (파일에 반영)");
    return 0;
}
