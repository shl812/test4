/**
 * * myshell.c
 * * @author  : 컴퓨터공학과  32190750 김수연(3분반)
 * * @email   : 1108suyeon@naver.com
 * * @version : 1.0
 * * @date    : 2022.12.09
 * **/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdbool.h>
#define MAX 1024

/*--------------문자열 parsing--------------*/
int Tokenize(char *buf,char * tokens[]){
	char * token;
	int tokenCount = 0;
	token = strtok(buf, " ");
	
	while(token != NULL) {
    	tokens[tokenCount++] = token;
     	token = strtok(NULL, " ");
 	}
 	
	tokens[tokenCount] = NULL;
 	
	return tokenCount;
	}

/*--------------cd명령어(내부 명령어) 구현--------------*/
void innercmd(char *token) { // 
 	if(chdir(token) == -1) 
		perror("cd");
	}

/*--------------리다이렉션일 때--------------*/
void rd(char *token[]) { 
 	pid_t pid;
 	int fd_rd;
 	printf("redirection!!\n");
 	if((pid = fork()) == 0) {
     	fd_rd = open(token[3], O_RDWR | O_CREAT | O_TRUNC, 0641);
     	dup2(fd_rd, STDOUT_FILENO);
     	close(fd_rd);
     	execvp(token[0], token);
     	exit(0); 
	}
 	wait(); 
}

/*--------------백그라운드일 때--------------*/
void bg(char *token[]) { 
 pid_t pid;
 int fd_bg;
 printf("!!! : %s\n",token[0]);
 if((pid = fork()) == 0) {
     fd_bg = open("/dev/null", O_RDONLY);
     dup2(fd_bg, STDOUT_FILENO);
     execvp(token[0],token);
     exit(0);
 }
}

/*--------------실행--------------*/
bool execute(char *cmd) {
 	int tokenCount;
 	int i = 0;
 	char * tokens[MAX];
 	pid_t pid;
 	int additionalFlag = 0;
 
 	tokenCount = Tokenize(cmd,tokens);
 	if(strcmp(tokens[0], "cd") == 0) {
     	innercmd(tokens[1]);
     	return;
 	}
 	for(i = 0; i < tokenCount; i++) {
     	if(!strcmp(tokens[i],">")) {
        	additionalFlag++;
     		rd(tokens);
     	}
     	if(!strcmp(tokens[i],"&")) {
        	additionalFlag++;
        	bg(tokens);
     	} 
	}
 	if(!additionalFlag) {
   		if((pid = fork()) == 0) {
   			execvp(tokens[0],tokens);
		exit(0);
   		}
   	wait();
 	}
 	memset(tokens, '\0', MAX);
 	return 1;
}

/*--------------메인--------------*/
int main() {
 	char cmd[MAX];
 	while(1) {
     	printf("%s$ ",get_current_dir_name());
     	fgets(cmd, sizeof(cmd) - 1, stdin);
     	
		if(strncmp("exit",cmd,4) == 0) {
     	printf("Bye!");
     	break;
     	}
    execute(cmd);
 	}
}
