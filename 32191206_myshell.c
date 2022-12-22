/**
 * myshell.c		: make a shell
 * @author			: Taedeok Kim
 * @email			: 32191206@dankook.ac.kr
 * @student's ID  	: 32191206
 * @version			: 1.0
 * @date			: 2022. 12. 09
**/

//background processing과 redirection 기능은 구현하지 못했습니다.

#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <fcntl.h>

int bg = 0, redir = 0;   //background와 redirection을 위한 변수

//입력받은 명령어를 띄어쓰기 단위로 끊어서 저장하는 함수
int tokenize(char* buf,char* delims,char* tokens[],int maxTokens){    
	char *token;
	int token_count = 0;				//토큰의 개수
	token = strtok(buf,delims);			//token에 delims를 기준으로 토큰을 나눠서 저장
    
	while(token != NULL && token_count <maxTokens){
		tokens[token_count++] = token;	//tokens가 [cat,text.txt]라면
		token = strtok(NULL,delims);	//token은 [cat], [text.txt]
	}
	
	tokens[token_count] = NULL;
	return token_count;					//토큰의 개수를 반환
}

bool run(char* line){       
	int token_count; 
	char* tokens[1024]; 
	char* delims = " \n\t";		
	int status;
    int fd_redir, fd_bg, i,j;
    char* tokencp[1024];
    pid_t child = 0;
    bg = 0; redir = 0;
	
    token_count = tokenize(line,delims,tokens,sizeof(tokens)/sizeof(char*));
    
	for(i = 0; i< token_count; i++){
        if(!strcmp(tokens[i],">")){			//토큰 중 >가 있는지 검사
			redir++;
		}
		if(!strcmp(tokens[i],"&")){			//토큰 중 &가 있는지 검사
			bg++;
		}
}
if(strcmp(tokens[0],"cd") == 0){       //CD명령어 (내부명령어) 구현
    if(chdir(tokens[1])!=0){
        perror("cd");
	}
    return 0;
}
if(!bg) {
    if((child = fork())==0) {			//&가 사용되지 않았을 때
        execvp(tokens[0],tokens);		//명령어대로 프로그램을 실행함
        exit(0);
	}
    wait(&status);
} 
    memset(tokens, '\0', 1024);
    return 1;
}
 
int main(){       
	char line[1024];			//명령어를 입력받는 문자열

    while(1){       
		printf("[%s] $", get_current_dir_name());	//쉘처럼 현재 디렉토리의 경로를 보여줌
		fgets(line, sizeof(line)-1, stdin);			//사용자에게 명령어를 입력받음
		if(run(line) == false) break;				//잘못된 명령어를 입력하면 종료됨
	}
    return 0;
}


