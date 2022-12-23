//이름 : 박주희
//학번 : 32191826
//날짜 : 2022.12.09

#include "myshell.h"

//명령어 실행
void execute(int isbg, char *argv[]){
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
