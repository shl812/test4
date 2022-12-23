/**
* exe.c : program execution
* author : Jiyun Song
* @email : 32202341@dankook.ac.kr
* @version : 1.0
* date : 2022.12.5
**/

#include "setting.h"

// 명령어를 실행해주는 함수
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
		// 명령어가 존재하지 않으면 오류 처리하도록 만들어야할 듯
    }
}

