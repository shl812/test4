/**
 * mysh.c: my shell program
 *이름: 배준하
 *학번: 32212004
 *제출일: 2022.12.04
**/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX 32
#define PATH_MAX 4096

int tokenize(char* buf, char* delims, char* tokens[], int maxTokens) {
	int tokenCount = 0;
	char* token = NULL;
	token = strtok(buf, delims);
	while ((token != NULL) && (tokenCount < maxTokens-1)) {
		tokens[tokenCount] = token;
		token = strtok(NULL, delims);
		tokenCount++;
	}
	tokens[tokenCount] = '\0';
	return tokenCount;
}

char* enterRemover(char* line){
	char* p = line;
	while(*p) p++;
	p--;
	if(*p == '\n') *p = '\0';
	return line;
}

int run(char* line) {
	char* tokens[32] = {0};
	int tokenCount;
       	pid_t pid;
	int status;
	int background = 0;

	tokenCount = tokenize(line, " ", tokens, sizeof(tokens)/sizeof(char*)); //tokenize command
	
	if(tokens[0] == NULL) return 0;

	if(strncmp("exit", tokens[0], 4) == 0) { return 1; } //if exit entered, close process

	if(strncmp("cd", tokens[0], 2) == 0){ //if cd entered, change diretory
		if(tokenCount == 2) chdir(tokens[1]);
		else printf("Usage: cd [path]\n");
		return 0;
	}
	
	if (strncmp("&", tokens[tokenCount-1], 1) == 0) { //background processing
		background = 1;
		tokens[tokenCount-1] = (char*)NULL; //deleting $ character
		tokenCount--;
	}

	if ((pid = fork()) == 0) { //case of child
		if(execvp(tokens[0], tokens) == -1) { printf("Command not work!\n"); exit(1);}
	}
	else if (pid < 0) { //case of fork failure
		printf("fork failed\n");
		exit(1);
	}
	else { //case of parent
		if(background == 0) waitpid(pid, &status, 0); //case of non-background processing
		else {
			printf("%d\n", pid);
		}	
	}
	return 0;
}

int main() {
	char path[PATH_MAX];
	char line[1024];

	while (1) {
		getcwd(path, PATH_MAX); //get working directory and print
		printf("%s$ ", path);

		fgets(line, sizeof(line) - 1, stdin); //get input
		enterRemover(line);
		if (run(line) != 0) break;
	}

	return 0;
}
