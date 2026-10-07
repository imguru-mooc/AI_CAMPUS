/* 02_myls.c — opendir/readdir + lstat 으로 만든 ls -li
 * 빌드: make 02_myls
 * 실행: ./02_myls .
 *       ln a.txt hard.txt ; ln -s a.txt soft.txt ; ./02_myls .
 */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

static void perm_str(mode_t m, char *s)
{
    s[0] = S_ISDIR(m) ? 'd' : S_ISLNK(m) ? 'l' : '-';
    const char *rwx = "rwxrwxrwx";
    for (int i = 0; i < 9; i++)
        s[i + 1] = (m & (1 << (8 - i))) ? rwx[i] : '-';
    s[10] = '\0';
}

int main(int argc, char *argv[])
{
    const char *dir = (argc > 1) ? argv[1] : ".";
    DIR *d = opendir(dir);
    if (d == NULL) {
        perror(dir);
        return 1;
    }
    struct dirent *e;
    while ((e = readdir(d)) != NULL) {
        if (e->d_name[0] == '.')
            continue;
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dir, e->d_name);
        struct stat st;
        if (lstat(path, &st) == -1) {          /* 링크 자체의 정보 */
            perror(path);
            continue;
        }
        char perm[11], when[32];
        perm_str(st.st_mode, perm);
        strftime(when, sizeof(when), "%m-%d %H:%M", localtime(&st.st_mtime));
        printf("%8lu %s %2lu %8lld %s %s", (unsigned long)st.st_ino, perm,
               (unsigned long)st.st_nlink, (long long)st.st_size, when, e->d_name);
        if (S_ISLNK(st.st_mode)) {
            char target[256];
            ssize_t n = readlink(path, target, sizeof(target) - 1);
            if (n >= 0) {
                target[n] = '\0';
                printf(" -> %s", target);
            }
        }
        printf("\n");
    }
    closedir(d);
    return 0;
}
