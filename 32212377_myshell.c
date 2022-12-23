/*
 * shell.c		: shell program
 * @author		: HyeounBin Shin
 * email		: tlsgusqls120@naver.com
 * @version		: 1.0
 * data			: 2022. 11. 25
 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <wait.h>

int redirection(char *tokens[100], int idx, int token_count){
	int fd,fd1, oldfd;
	char *argv[100];
	pid_t fork_return;
	int exit_status;
	for(int i = 0; i < idx; i++){
		argv[i] = tokens[i];
	}
	argv[idx] = NULL;
	
	remove(tokens[token_count-1]);
	fd1 = open(tokens[token_count-1], O_RDWR | O_CREAT, 0641);
	
	oldfd = dup(STDOUT_FILENO);
	dup2(fd1, STDOUT_FILENO);
	if((fork_return = fork()) == -1){
		printf("ERROR!!!\n");
		exit(1);
	}
	else if(fork_return == 0){
		fd = execvp(argv[0], argv);
		exit(127);
	}
	else{
		wait(&exit_status);
		dup2(oldfd, STDOUT_FILENO);
		close(fd1);
		close(oldfd);
	}
	return 0;
}

int tokenize(char* buf, char* delims, char *tokens[100], int maxTokens){
	char *token;
	int token_count = 0;
	token = strtok(buf, delims);
	
	while(token != NULL && token_count < maxTokens){
		tokens[token_count++] = token;
		token = strtok(NULL, delims);
	}
	tokens[token_count] = NULL;
	return token_count;
}

int run(char* line){
	char* delims = " ";
	char *tokens[100];
	pid_t fork_return;
	int exit_status;
	int token_count = tokenize(line, delims, tokens, sizeof(tokens) / sizeof(char*));
	int idx = -1;
	for(int i = 0; i < token_count; i++){
		if(strcmp(tokens[i], ">") == 0){
			idx = i;
			break;
		}
	}
	if(idx == -1){
		if((fork_return = fork()) == -1){
			printf("ERROR!!!\n");
			exit(1);
		}
		else if(fork_return == 0){
			execvp(tokens[0], tokens);
			exit(127);
		}
		else{
			wait(&exit_status);
		}
	}
	else{
		redirection(tokens, idx, token_count);
	}
	
	
	return 1;
}

int main(){
	
	char line[1024];
	char path[200];
	
	while(1){
		getcwd(path, 200);
		printf("%s $", path);
		fgets(line, sizeof(line), stdin);
		if(line[strlen(line) - 1] == '\n')
			line[strlen(line) - 1] = NULL;
		if(run(line) == 0) break;
	}
	
	return 0;
}