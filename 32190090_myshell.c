/**
 * 32190090_myshell.c	: shell program
 * @author				: 강창진
 * @student ID 			: 32190090
 * @email				: 32190090@gmail.com
 * @date				: 2022.12.08
**/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>

//입력받은 명령어를 공백을 기준으로 문자열을 조각낸다.
int Tokenize(char *buf,char *delims, char *tokens[], int maxTokens){
	char *token;
	int tokenCount = 0;
	
	//공백을 기준으로 앞에 있는 문자를 token에 저장한다.
	token = strtok(buf, delims);
	while(token != NULL && tokenCount < maxTokens){
		tokens[tokenCount++] = token;
		token = strtok(NULL, delims);
	}
	tokens[tokenCount] = NULL;
	return tokenCount;
}

//background 실행 함수
void BackgroundExe(char *tokens[]){
	pid_t pid;
	int fd_Bg;
				
	if((pid = fork()) == 0){
		fd_Bg = open("/dev/null", O_RDONLY);
		dup2(fd_Bg, STDOUT_FILENO);
		execvp(tokens[0], tokens);
		exit(0);
	}
}

//'>'redirection 함수
void Redirection(char *tokens[], int tokenCount){
	pid_t pid;
	int fd_Rd;
	
	int i;
	//명령어가 '>'를 인식하는 경우가 생겨 '>'을 NULL로 만든다.
	for(i = 0; i < tokenCount; i++){
		if(!strcmp(tokens[i], ">"))
			tokens[i] = NULL;
	}
	
	if((pid = fork()) == 0){
		//O_CREAT를 통해 파일을 생성하고, O_TRUNC를 통해 기존 파일의 내용을 지우고 새로운 내용을 입력받는다.
		fd_Rd = open(tokens[tokenCount - 1], O_RDWR | O_CREAT | O_TRUNC, 0644);
		//dup2를 이용해 파일에 명령어의 결과를 입력받는다.
		dup2(fd_Rd, STDOUT_FILENO);
		close(fd_Rd);
		
		execvp(tokens[0],tokens);
		exit(0);
	}
	wait();
}

//'>>'redirection 함수
void d_Redirection(char *tokens[], int tokenCount){
	pid_t pid;
	int fd_Rd;
	
	int i;
	//명령어가 '>>'를 인식하는 경우가 생겨 '>>'을 NULL로 만든다.
	for(i = 0; i < tokenCount; i++){
		if(!strcmp(tokens[i], ">>"))
			tokens[i] = NULL;
	}
	
	if((pid = fork()) == 0){
		//'>'리다이렉션과 달리 O_APPEND를 통해 기존 파일의 내용을 지우지 않고 이어쓴다.
		fd_Rd = open(tokens[tokenCount - 1], O_RDWR | O_CREAT | O_APPEND, 0644);
		dup2(fd_Rd, STDOUT_FILENO);
		close(fd_Rd);
		
		execvp(tokens[0],tokens);
		exit(0);
	}
	wait();
}

bool Execute(char *cmd){
	int tokenCount;
	char *tokens[1024];
	int i = 0;
	pid_t pid;
	int Flag = 0;
	char *delims = " \n\t";
	
	tokenCount = Tokenize(cmd, delims, tokens, sizeof(tokens)/sizeof(char*));
	for(i = 0; i < tokenCount; i++){
		//tokens에 "&"이 있으면 Background함수를 실행하도록 한다.
		if(!strcmp(tokens[i], "&")){
			Flag++;
			BackgroundExe(tokens);
		}
		//tokens에 ">"이 있으면 Redirection함수를 실행하도록 한다.
		if(!strcmp(tokens[i], ">")){
			Flag++;
			Redirection(tokens, tokenCount);
		}
		//tokens에 ">>"이 있으면 d_Redirection합수를 실행하도록 한다.
		if(!strcmp(tokens[i], ">>")){
			Flag++;
			d_Redirection(tokens, tokenCount);
		}
	}
	
	//백그라운드나 리다이렉션이 없을 경우 일반실행을 하도록 한다. Flag의 기본값이 0이므로 참이되도록 !을 붙여준다.
	if(!Flag){
		if((pid = fork()) == 0){
			execvp(tokens[0], tokens);
			exit(0);
		}
		wait();
	}
	memset(tokens, '\0', 1024);
	return 1;
}

int main(){
	char cmd[1024];
	while(1){
		printf("%s$ ", get_current_dir_name());
		fgets(cmd, sizeof(cmd) - 1, stdin);
		
		//exit이 입력되면 myshell을 종료한다.
		if(strncmp("exit", cmd, 4) == 0){
		printf("종료\n");
		break;
	}
		if(Execute(cmd) == false)
			break;
	}
}
