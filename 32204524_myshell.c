/*
* 시스템프로그래밍
* 천은유
* 소프트웨어학과
* 32204524
* 2022.12.08
*/

#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <fcntl.h>

int tokenize(char* buf,char* delims,char* tokens[],int maxTokens) {   // 명령어 분리 함수
    char *token;
    int token_count = 0;
    token = strtok(buf,delims);
    while(token != NULL && token_count <maxTokens) {
        tokens[token_count++] = token;
        token = strtok(NULL,delims);
    }
    tokens[token_count] = 0;
    return token_count;
}

bool run(char* line) {   // 명령어 실행 함수
    int token_count;   // 명령어 토큰 개수
    char* tokens[1024];   // 분리된 명령어
    char* delims = " \n\t";   // 토큰 분류 기준
    char path[1024] = "/bin/";   // 명령어 경로
	char vipath[1024] = "/usr/bin/"; // vi 명령어 경로
    char *env[] = {0};   // 환경변수
    char* tokens_redir[1024];   // redirection 명령어
    char* tokens_bg[1024];   // background 명령어
    int fd_redir,fd_bg;   // redirection, background file descriptor
    pid_t child = 0;   // 자식 프로세스 PID
    int b=0, b_loc=0;   // background 판별
    int L=0, L_loc=0;  // redirection 판별
	int V=0;

    token_count = tokenize(line,delims,tokens,sizeof(tokens)/sizeof(char*));   // 명령어 분리

    int i;
    for(i = 0; i< token_count; i++) {   // redirection과 background 확인
        if(!strcmp(tokens[i],">")) {
            L++;
            L_loc=i;
        }
        if(!strcmp(tokens[i],"&")) {
            b++;
            b_loc=i;
        }
    }

    if(strcmp(tokens[0],"cd") == 0) {   // cd 명령어 처리
        if(chdir(tokens[1])!=0)
            perror("cd");
        return 1;
    } else if(strcmp(tokens[0],"exit") == 0) {   // exit 명령어 처리
        return 0;
    } else if(strcmp(tokens[0],"vi") == 0) {   // vi 명령어 처리
		V++;
    }

    strcat(path, tokens[0]);   // 명령어 경로 상세화
	strcat(vipath, tokens[0]); // vi 명령어 경로 상세화
    
    if(b) {   // background 처리
        if((child = fork())==0) {
            for (i=0;i<b_loc;i++) {
                tokens_bg[i]=tokens[i];
                printf("%s", tokens[i]);
            }
			if (V){
				execve(vipath, tokens_bg, env);
			} else {
				execve(path, tokens_bg, env);
			}
            exit(0);
        }
    } else if(L) {   // redirection 처리
        if((child=fork())==0) {  
            fd_redir = open(tokens[L_loc+1],O_RDWR | O_CREAT | O_TRUNC,0641);
            for (i=0;i<L_loc;i++) {
                tokens_redir[i]=tokens[i];
            }
            dup2(fd_redir,STDOUT_FILENO);
            close(fd_redir);
			if (V){
				execve(vipath, tokens_redir, env);
			} else {
				execve(path, tokens_redir, env);
			}
            exit(0);
        } else {
            wait(NULL);
        }
    } else {   // 기본 실행
        if((child = fork())==0) {
			if (V){
				execve(vipath, tokens, env);
			} else {
				execve(path, tokens, env);
			}
            exit(0);
        } else {
            wait(NULL);
        }
    }
    memset(tokens, '\0', 1024);   // tokens 초기화
    return 1;
}

int main() {   // 메인 함수
    char line[1024];   // 명령어 저장 변수

    while(1) {
        printf("[%s] $", get_current_dir_name());   //현재 디렉토리
        fgets(line, sizeof(line)-1, stdin);   // 명령어 입력
        if(run(line) == false) break;   // 명령어 실행
    }
    return 0;
}