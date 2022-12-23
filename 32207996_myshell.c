/**
*mycat.c : my shell program
*@author: Haeyoung Hwang
*@email: 32207996sw@dankook.ac.kr
*@date : 2022.12.08
**/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdbool.h>
#define MAX 1024

int status;
int bg = 0, redir = 0;
pid_t child;

int tokenize(char* buf, char* delims, char* tokens[], int maxTokens) {	//토큰을 기준으로 나눔
    int tokenCount = 0;	//토큰 개수
    char* token;	//토큰 저장을 위한 포인터변수
    token = strtok(buf, delims);	//delims 기준으로 buf를 나눠서 token에 저장함
    while (token != NULL && tokenCount < maxTokens) {
        tokens[tokenCount] = token;	//tokens에 토큰 저장
        tokenCount++;	//토큰 개수 1씩 증가
        token = strtok(NULL, delims);
    }
    tokens[tokenCount] = NULL;
    return tokenCount;	//개수 반환
}

void redirection(char* token[]) { // 입력, 출력 재지정
    pid_t pid;
    int fd_redir;

    fd_redir = open(token[3], O_RDWR | O_CREAT | O_TRUNC, 0641);
    dup2(fd_redir, STDOUT_FILENO);
    token[2] = NULL; //>는 널값으로 
    close(fd_redir);
    execvp(token[0], token);
    exit(0);
}


bool run(char* line) {	//쉘 동작
    char* tokens[MAX];
    char delims[] = " \r\n\t";		//이를 기준으로 토큰 나눔
    int i, tokenCount, fd_redir;
    bool backgr;

    tokenCount = tokenize(line, delims, tokens, sizeof(tokens) / sizeof(char*));
    if (tokenCount == 0) //토큰이 없다면(입력 없음)
        return true;	//계속 작동

    //명령어 구현 
    //cd 
    if (strcmp(tokens[0], "cd") == 0) {
        if (chdir(tokens[1]) != 0)	//token[1]로 디렉토리 변경      
            perror("cd error");	//에러메세지       
        return;
    }
    //exit 
    if (strcmp(tokens[0], "exit") == 0)
        return false;
    //help
    if (strcmp(tokens[0], "help") == 0) {
        printf("==================================================\n");
        printf("My shell!!!\n");
        printf("Some examples of the built-in commands\n");
        printf("------------------------------------------\n");
        printf("cd    : 디렉토리 변경하기\n");
        printf("exit  : 쉘 나가기\n");
        printf("help  : 도움말\n");
        printf("==================================================\n");
    }


    for (i = 0; i < tokenCount; i++) {
        //background processing
        if (strcmp(tokens[i], "&") == 0) {	//&이 있는 경우
            bg++;				//1씩 증가
            backgr = true;			//부모 프로세스에서 사용하기 위함
            tokens[i] = '\0';	//&는 null로 설정
            break;
        }
        if (strcmp(tokens[i], ">") == 0) { // redirection 실행
            redir++;
            redirection(tokens);
        }
    }

    child = fork();	//fork
    if (child == 0) {	//자식 프로세스(execve() 실행)
        if (bg == 0) {	//redirection 없는 백그라운드 =0 즉 foreground일 때
            if (redir == 0) {
                execvp(tokens[0], tokens);
                exit(0);
            }
        }
        else if (bg > 0) {	//background
            backgr = true;
            execvp(tokens[0], tokens);
            exit(0);
        }
        else if (redir > 0) {	//redirection
            fd_redir = open(tokens[i], O_RDWR | O_CREAT | O_TRUNC, 0644);

            if (fd_redir < 0) {
                perror("open fail\n");
                exit(1);
            }
            dup2(fd_redir, 1);
            close(fd_redir);
        }
    }

    else if (child < 0) {	//fork 실패
        perror("fork error\n");
        exit(0);
    }

    else {	//부모 프로세스
        if (backgr == false) //background는 wait 안함
            wait(&status);

    }

    return true;
}

//메인 함수
int main() {
    char line[MAX];	//입력을 저장할 배열

    while (1) {	//무한루프
        printf("%s $ ", get_current_dir_name());
        fgets(line, sizeof(line) - 1, stdin);		//입력받기

        if (run(line) == false)
            break;
    }
    return 0;
}

