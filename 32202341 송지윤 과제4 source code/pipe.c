/**
* pipe.c : pipe function
* author : Jiyun Song
* @email : 32202341@dankook.ac.kr
* @version : 1.0
* date : 2022.12.5
**/

#include "setting.h"

// pipe 실행해주는 함수
void doPipe(char* arg1[], char* arg2[]){
	// pipe 생성 후 descriptor 저장을 위한 배열 fd
    int fd[2];
    if(pipe(fd) == -1) {
        perror("Pipe failed");
        exit(1);
    }

    if(fork() == 0){            
        close(STDOUT_FILENO);  
        dup2(fd[1], 1);        
        close(fd[0]);    
        close(fd[1]);

        execvp(arg1[0], arg1);
        perror("parent execvp failed");
        exit(1);
    }

    if(fork() == 0) {           
        close(STDIN_FILENO); 
        dup2(fd[0], 0);      
        close(fd[1]);  
        close(fd[0]);

        execvp(arg2[0], arg2);
        perror("child execvp failed");
        exit(1);
    }

    close(fd[0]);
    close(fd[1]);
    wait(0);
    wait(0);
}

