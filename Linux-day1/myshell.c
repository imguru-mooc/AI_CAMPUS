/* myshell.c — Day 1 종합 실습: fork-exec-wait 루프로 만드는 미니 셸
 * 빌드: make myshell
 * 실행: ./myshell
 * 관찰: strace -f -e trace=clone,execve,wait4 ./myshell
 *
 * 지원: 일반 명령 실행, 내장 명령 exit / cd, 직전 종료 코드 표시
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define MAX_LINE 1024
#define MAX_ARGS 64

/* line을 공백 기준으로 잘라 argv에 담고 인자 개수를 반환 */
static int parse(char *line, char *argv[])
{
    int argc = 0;
    char *tok = strtok(line, " \t\n");
    while (tok != NULL && argc < MAX_ARGS - 1) {
        argv[argc++] = tok;
        tok = strtok(NULL, " \t\n");
    }
    argv[argc] = NULL;                 /* execvp는 NULL로 끝나는 배열을 요구 */
    return argc;
}

int main(void)
{
    char line[MAX_LINE];
    char *argv[MAX_ARGS];
    int last = 0;                      /* 직전 명령의 종료 코드 */

    signal(SIGINT, SIG_IGN);           /* 셸 자신은 Ctrl+C로 죽지 않음 */

    for (;;) {
        printf("myshell[%d]$ ", last);
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {   /* Ctrl+D */
            printf("\n");
            break;
        }

        int argc = parse(line, argv);
        if (argc == 0)
            continue;

        /* 내장 명령: 자식이 아니라 셸 자신이 처리해야 하는 것 */
        if (strcmp(argv[0], "exit") == 0)
            break;
        if (strcmp(argv[0], "cd") == 0) {
            const char *dir = (argc > 1) ? argv[1] : getenv("HOME");
            if (chdir(dir) == -1) {
                perror("cd");
                last = 1;
            } else {
                last = 0;
            }
            continue;
        }

        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            continue;
        }
        if (pid == 0) {
            signal(SIGINT, SIG_DFL);   /* 자식은 Ctrl+C에 정상 반응 */
            execvp(argv[0], argv);
            perror(argv[0]);
            _exit(127);
        }

        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
            last = WEXITSTATUS(status);
        else if (WIFSIGNALED(status))
            last = 128 + WTERMSIG(status);   /* bash와 같은 관례 */
    }
    return 0;
}
