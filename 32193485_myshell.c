/**
 * name        : Lee Jong Hoon
 * student's ID: 32193485
 * date        : 2022. 12. 08
**/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <stdlib.h>
#define MAX 300

void *prompt(char cBuf[]) {
        time_t rawtime;
        struct tm *timeinfo;
        char hBuf[MAX], uBuf[MAX], dBuf[MAX];
        char *now;
        void *ret;

        time(&rawtime);
        timeinfo = localtime(&rawtime);
        now = asctime(timeinfo);

        now[strlen(now)-1] = 0;

        gethostname(hBuf, MAX);
        getlogin_r(uBuf, MAX);
        getcwd(dBuf, MAX);

        printf("[%s]%s@%s(%s)$ ", now, hBuf, uBuf, dBuf);

        ret = fgets(cBuf, MAX, stdin);

        if(cBuf[strlen(cBuf)-1] == '\n')
                cBuf[strlen(cBuf)-1] = 0;

        return ret;
}

       
                cBuf[strlen(cBuf)-1] = 0;

        return ret;
}

int main()
{
        char cBuf[MAX];
        char *arg;
        pid_t pid;
        int status;

        while(prompt(cBuf)) {

                if((pid = fork()) < 0) {
                        perror("fork 에러입니다r");
                }
                else if(pid == 0) {
                        strtok(cBuf, " ");
                        arg = strtok(NULL, " ");

                        if(arg == NULL)
                                execlp(cBuf, cBuf, (char*) 0);
                        else {
                                if(strcmp(cBuf, "cd") == 0) {
                                        chdir(arg);
                                        _exit(0);
                                }
                                else
                                        execlp(cBuf, cBuf, arg, (char*) 0);
                        }
                        perror("오류로 인한 실행불가");
                }

                waitpid(pid, &status, 0);
        }

     exit(0);

}
