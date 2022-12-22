/*
myshell.c: shell program
Author: KIM SEOJUN
Student ID: 32190644
Date: 2022.12.02
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

void help() { // 도움말 출력
	printf("-----------MyShell-----------\n");
	printf("& : background processing\n");
	printf("> : redirection\n");
	printf("help : 도움말 출력\n");
	printf("exit : Shell 종료\n");
	printf("-----------------------------\n");
}

int tokenize(char* buf, char* delims, char* tokens[], int MAX) { // tokenize 함수, parse 역할 수행
	char* token;
	int token_count = 0;

	token = strtok(buf, delims);

	while(token != NULL && token_count < MAX) { // token의 마지막 인덱스까지 읽기
		tokens[token_count] = token;
		token_count++;	
		token = strtok(NULL, delims);
	}
	
	tokens[token_count] = NULL; // parsing
	return token_count;
}

bool run(char* line) { // 실행 함수
	int i;
	int fd; // file directory
	
	int myRedirection = 0; // redirection 변수
	int myBackgroundProcessing = 0; // background processing 변수
	int redirectionCheck = 0; // redirection 점검 변수
	int backgroundProcessingCheck = 0; // background processing 점검 변수
	
	char* tokens[128]; // 토큰
	int token_count; // 토큰 수
	char delims[] = " \r\t\n";
	
	pid_t child; // 자식 프로세스 process ID
	
	token_count = tokenize(line, delims, tokens, sizeof(tokens)/sizeof(char*)); // tokenize 함수를 이용하여 token_count를 얻음

	if(token_count == 0) // 입력하지 않았을 때
		return true;
	
	if(strcmp(tokens[0], "help") == 0){ // help를 입력했을 때
		help(); // 도움말 출력
		return true;
	}
	
	if(strcmp(tokens[0], "exit") == 0) // exit을 입력했을 때
		return false; // shell 종료
	
	// handling redirection, pipe and background processing
	//Background Processing과 Redirection을 해야 하는지 검사 
	
	// background processing, redirection
	for(i = 0; i < token_count; i++) {
		if(strcmp(tokens[i],"&") == 0) { // & 이 tokens 안에 있을 때 background processing
			myBackgroundProcessing = i;
			backgroundProcessingCheck = 1;
			break;
		}
		
		if(strcmp(tokens[i],">") == 0) { // > 이 tokens 안에 있을 때 redirection
			myRedirection = i;
			redirectionCheck = 1;
			break;
		}

	}	

	child = fork(); // fork
	if(child < 0) { // process ID가 0보다 작을 때
		printf("fork Error!!\n"); // fork error
		return false;
	}
	
	else if(child == 0) { // process ID가 0일 때, 즉 자식 프로세스일 때
		if(backgroundProcessingCheck) { // background process가 있는지 점검
			tokens[myBackgroundProcessing] = '\0';
		}
	
		if(redirectionCheck == 1) {// redirection이 있는지 점검
			fd = open(tokens[myRedirection + 1], O_WRONLY|O_TRUNC|O_CREAT, 0664);
			close(STDOUT_FILENO);
			dup2(fd, STDOUT_FILENO);
			tokens[myRedirection] = '\0';
		}
		execvp(tokens[0], tokens); // 실행
		printf("execvp Error!!\n"); // execvp error
		
		return false;
	}
	else if(backgroundProcessingCheck == false) { // background processing이 없을 때
		wait(NULL); // wait
	}
	
	return true;
}	

int main() { // 메인 함수
	char line[128]; // 입력 줄
	
	while(1) {
		printf("%s $ ", get_current_dir_name()); // 현재 디렉토리의 이름 출력
		fgets(line, sizeof(line) - 1, stdin); // line을 입력 받음
		if(run(line) == false) // return 값이 false일 때 까지 실행
			break;
	}
	
	return 0;
}
