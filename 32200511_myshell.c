/*
 * @author	: Min Kim
 * @student's ID: 32200511
 * @date	: 2022.12.09
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#define _GNU_SOURCE

bool cmd_help(int argc, char* argv[]){
	if (argc != 1){
		printf("USAGE : %s\n", argv[0]);
		return false;
	}
	printf("/**************Simple Shells**************/\n");
	printf("You can use it just as the conventional shell\n\n");
	printf("Some examples of the built-in commands\n");
	printf("cd\t: change directory\nexit\t: exit this shell\nquit\t: quit this shell\n");
	printf("help\t: show this help\n?\t: show this help\n");
	printf("****************************\n");
	return true;
}
int tokenize(char* buf, char* delims, char* tokens[], int maxTokens){
	int token_count = 0;
	char* token = strtok(buf, delims);
	while (token != NULL && token_count < maxTokens){
		tokens[token_count] = token;
		token = strtok(NULL, delims);
		token_count++;
	}
	return token_count;
}
bool run(char* line){
	int fd = open(get_current_dir_name(), O_RDONLY);
	pid_t child;
	char delims = " ";
	char* tokens[10];
	int token_count = tokenize(line, delims, tokens, sizeof(tokens)/sizeof(char*));

	if (strcmp(tokens[0], "exit") == 0 || strcmp(tokens[0], "quit") == 0){
		return false;
	}
	else if (strcmp(tokens[0], "help") == 0 || strcmp(tokens[0], "?") == 0){
		cmd_help(token_count, tokens);
		return true;
	}
	else if (strcmp(tokens[0], "cd") == 0){
		if (token_count != 2)
			printf("USAGE : %s dir\n", tokens[0]);
		chdir(tokens[1]);
		return true;
	}
	if ((child = fork()) == 0){
		dup2(fd, STDOUT_FILENO);
		execvp(tokens[0], tokens);
		exit(0);
	}
	return true;
}
int main(){
	char line[1024];
	while(1){
		printf("%s $ ", get_current_dir_name());
		fgets(line, sizeof(line)-1, stdin);
		if(run(line) == false) break;
	}
}
