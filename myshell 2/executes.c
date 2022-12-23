/*
	이름 : 박민규
	학번 : 32191597
	날짜 : 2022-12-03
*/

#include "myshell.h"

// fork와 wait 함수를 사용하여 명령어 실행 구현
void executesLine(int isbg, char *argv[]){
    int status, pid;

    if ((pid=fork()) == -1)
        perror("fork failed");
    else if (pid != 0) {
        if(isbg==0)
             pid = wait(&status);
        else {
             printf("[1] %d\n", getpid());
             waitpid(pid, &status, WNOHANG);    
        }
    } else {
        execvp(argv[0], argv);
    }
}
