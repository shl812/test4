/**
* redir.c : redirection function
* author : Jiyun Song
* @email : 32202341@dankook.ac.kr
* @version : 1.0
* date : 2022.12.5
**/


#include "setting.h"

// redirection을 실행해주는 함수
void doRedirection(int flag, int isbg, char *argv[], char* input, char * output){
    int input_fd, output_fd; // 파일을 열고 변경 내용을 저장할 변수
    int status, pid;
    
    if ((pid=fork()) == -1) 
        perror("fork failed");
    else if (pid != 0) {
		// background 실행일 경우
        if(isbg==0) 
            pid = wait(&status);
        else {
            printf("[1] %d\n", getpid());
            waitpid(pid, &status, WNOHANG);
        }
    } else {
        if (flag == 2) {
            if((input_fd = open(input, O_RDONLY))==-1){
                perror(argv[0]);
                exit(2);
            }
            dup2(input_fd, 0);
            close(input_fd);
            execvp(argv[0], argv);
        } else if (flag == 3) {
            output_fd = open(output, O_CREAT|O_TRUNC|O_WRONLY, 0600);
            dup2(output_fd, 1);
            close(output_fd);
            execvp(argv[0], argv);
        } else {
            if (input != NULL && output != NULL) {
                input_fd = open(input, O_RDONLY);
                dup2(input_fd, 0);
                close(input_fd);
                
                output_fd = open(output, O_CREAT|O_TRUNC|O_WRONLY, 0600);
                dup2(output_fd, 1);
                close(output_fd);
                execvp(argv[0], argv);
            }
        }
    }
}

