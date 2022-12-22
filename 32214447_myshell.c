/*
date		: 2022.12.09
student ID	: 32214447 차수민
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <stdlib.h>
#define MAXLINE 64
#define _GNU_SOURCE

typedef bool cmd_func(int argc, char *argv[]);

struct cmd {
	char *name;
	char *desc;
	cmd_func *func;
};

cmd_func cmd_cd, cmd_exit, cmd_help;

struct cmd builtin_commnads [] = {
	{ "cd",		"change directory",	cmd_cd },
	{ "exit",	"exit this shell",	cmd_exit },
	{ "quit",	"quit tihs shell",	cmd_exit },
	{ "help",	"show this help",	cmd_help },
	{ "?",		"show this help",	cmd_help }
};


bool cmd_cd(int argc, char *argv[])
{
	if (argc == 1) {
		chdir(getenv("HOME"));
	} else if (argc == 2) {
		if (chdir(argv[1]))
			printf("no directory\n");
	} else printf("USAGE: cd [dir]\n");

	return true;
}

bool cmd_exit(int argc, char *argv[])
{
	return false;
}
bool cmd_help(int argc, char* argv[]) {
	
	for (int i = 0; i < builtin_commnads_size; i++)
		printf("%-10s: %s\n", builtin_commnads[i].name,
				      builtin_commnads[i].desc);

	return true;
}

int tokenize(char* buf, char* delims, char* tokens[], int maxTokens) {
	int count=0;
	char *token;
	
	token = strtok(buf, delims);
	while(token != NULL && token_count < maxTokens) {
		tokens[count] = token;
		count++;
		token = strtok(NULL, delims);
		
	}
	tokens[count] = NULL;
	return count;
}


bool run(char* line) {
	const char delims[]= " \r\n\t";
	char *tokens[128];
	int token_count;
	int status;
	pid_t child;
	
	token_count = tokennize(line, delims, tokens, sizeof(tokens)/sizeof(char*));
	if((child = fork())==0) {
		
		execvp(token[0], tokens);
		printf("no such file\n");
	} else if ((child = fork()) < 0) {
		printf("failed\n")
	} else wait(&status);
	
	return true;
	
	wait();
	
}

int main(void) {
	char line[1024];
	while(true) {
		printf("%s $", get_current_dir_name());
		fgets(line, sizeof(line)-1, stdin);
		if(run(line)==false) break;
	}
	return 0;
}