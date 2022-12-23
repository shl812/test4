/**
 * myshell.c: custom shell that has redirection and background processing functions 
 * Inchang Kang
 * 32210086
 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include <fcntl.h>

#define TOKEN_MAX 128 //max token count
#define T 1
#define F 0

//FLAGS for redirect and background
//so we can tell when the first command ends
typedef struct FLAGS {
	int redirect; // 1 for >, 2 for >>, 3 for <
	int redInd; //index of redirection sign
	int background; //bool
} FLAGS;

//prints commands supported my myshell
void printHelp() {
	printf("cd [dir]\n");
	printf("exit\n");
	printf("help\n");
}

/*
buf: line
delims: delims for tokens " "
tokens: save result of tokenizer
flags: redirect, backround detection etc
*/
int tokenize(char* buf, char* delims, char* tokens[], FLAGS *flags) {
	int token_count = 0;
	char *token = strtok(buf, delims);
	while(token != NULL && token_count < TOKEN_MAX) { //maxTokens changed to global constant
		if (strcmp(token, ">") == 0 || strcmp(token, ">>") == 0 || strcmp(token, "<") == 0) {
			if (strcmp(token, ">") == 0) {
				flags->redirect = 1;
			}
			else if (strcmp(token, ">>") == 0){
				flags->redirect = 2;
			}
			else { // <
				flags->redirect = 3;
			}
			flags->redInd = token_count;
			token = NULL; //null terminate
		}
		
		if (token && (strcmp(token, "&") == 0)) { //token is NULL when redirect
			flags->background = T;
			token = NULL;
		}
		
		tokens[token_count++] = token;
		token = strtok(NULL, delims);
	}
	return token_count;
}

int run(char *line) {
	char *tokens[TOKEN_MAX] = {0};
	int token_count;
	
	//flags for redirecttion and background processing
	FLAGS flags = {0, 0, 0}; //all initialized to 0/False
	
	token_count = tokenize(line, " \n", tokens, &flags);
	
	//check valid input
	if (token_count == TOKEN_MAX) {
		printf("Max number of tokens exceeded\n");
		return T; //return without running anything
	}
	if (token_count == 0) { //empty line
		return T;
	}
	if (!tokens[0]) { //token_count > 1 but NULL (due to ">" input etc)
		printf("Unexpected token\n");
		return T;
	}
	
	//check for cd
	if (strcmp(tokens[0], "cd") == 0) {
		if (token_count == 1) {
			chdir("/workspace/sys32210086/myshell");
		}
		else if (token_count != 2) {
			printf("Wrong number of arguments for cd. Useage: cd [dir]\n");
		}
		else if (chdir(tokens[1]) != 0) {
			perror("Failed to change directory");
		}
		return T;
	}
	
	//check for exit
	if (strcmp(tokens[0], "exit") == 0) {
		return F;
	}
	
	//check for help
	if (strcmp(tokens[0], "help") == 0) {
		printHelp();
		return T;
	}
	
	//execute
	pid_t pid;
	
	if ((pid = fork()) < 0) {
		perror("Failed fork");
		exit(1);
	}
	else if (pid == 0) {
		if (flags.redirect) { //redirect in child
			int fd;
			char *redPath = tokens[flags.redInd+1];
			if (flags.redirect == 3) { // <
				close(STDIN_FILENO);
				if (fd = open(redPath, O_RDONLY) < 0) {
					perror("Can't open redirection path");
				}
				if(dup2(fd, STDIN_FILENO) < 0) {
					perror("Failed while redirecting fd");
				}
			}
			else { // > or >>
				close(STDOUT_FILENO);
				if (flags.redirect == 1) { // >
					fd = open(redPath, O_RDWR | O_CREAT | O_TRUNC);
				}
				else if(flags.redirect == 2) { // >>
					fd = open(redPath, O_RDWR | O_APPEND);
				}

				if (fd < 0) { //error check
					perror("Can't open redirection path");
				}
				//change fd with dupe
				if (dup2(fd, STDOUT_FILENO) < 0) {
					perror("Failed while redirecting fd");
				}
			}
			 
		}
		execvp(tokens[0], tokens);
		perror("Error while executing");
		exit(1); //exit for child
	}
	
	if (flags.background) {
		printf("Background process on %d\n", pid);
	}
	else {
		waitpid(pid, NULL, 0); //wait for child from this fork
	}
	
	return T;
}

int main(void) {
	char line[LINE_MAX];
	while(T) {
		printf("%s$", get_current_dir_name());
		fgets(line, LINE_MAX, stdin);
		if (run(line) == F) break;
	}
	printf("Exiting shell...\n");
	
	return 0;
}
