/* 32211507 문민제 2022.12.09 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/wait.h>
#define MAX_SYS 128

bool cmd_help(int argc, char* argv[]);
bool cmd_exit(int argc, char* argv[]);
bool cmd_cd(int argc, char*argv[]);
int check(char *line[]);
int tokenize(char *buf, char *delims, char *tokens[], int max);
bool run(char *line);

struct shell {
	char *key; 
	char *key_ex; bool (*intr_func)(int argc, char *argv[]);
};

struct shell cmd[] = {
	{"help", "Show help ", cmd_help},
    {"cd", "Change directory", cmd_cd},
	{"exit", "Exit shell", cmd_exit},
	{">", "Redirection"},
	{"&", "Background processing"}
};

int main() { 
	char line[1024], usr[MAX_SYS], cwd[MAX_SYS];
	getlogin_r(usr, MAX_SYS); 
	getcwd(cwd, MAX_SYS); 
	while(1) {
		printf("%s@%s $ ", usr, cwd);
		fgets(line, sizeof(line) - 1, stdin);
		if(run(line) == false) break;
	}
	return 0;
}

bool cmd_help(int argc, char* argv[]) {  // help 함수
	int i;
	printf("---------------- myshell ----------------\n");
	printf("CMD\t FUNCTION\n-----------------------------------------\n");
	for (i = 0; i < 3; i++) printf("%s\t %s\n", cmd[i].key, cmd[i].key_ex);
	printf("-----------------------------------------\n");
	printf("KEY\t FUNCTION\n-----------------------------------------\n");
	for (i = 3; i < 5; i++) printf("%s\t %s\n", cmd[i].key, cmd[i].key_ex);
	return true;
}

bool cmd_cd(int argc, char*argv[]) {  // change directory 함수
	if(argc == 1); 
	else if(argc == 2) {
		if(chdir(argv[1])) printf("Type correct directory\n");
	}
	else printf("USAGE: %s [dir_name]\n", argv[0]);
	return true;
}

bool cmd_exit(int argc, char* argv[]) { return 0; }  // eixt 함수

void cmd_redirection(int argc, char *argv[]) {  // redirection 함수
	int fd;
	if (argc != 4) { printf("USAGE: %s input > output\n", argv[0]); return; }
	if((fd = open(argv[3], O_WRONLY | O_CREAT, 0664)) < 0) {
		printf("Can't open %s file with errno %d\n", argv[3], errno);
		return;
	}
	dup2(fd, STDOUT_FILENO);
	argv[2] = NULL;
	close(fd);
}

int check(char *line[]) {  // cmd 확인
	int i;
	for(i=0; line[i] != NULL; i++){
		if(!strcmp(line[i], ">")) return 1;  
		if(!strcmp(line[i], "&")) {line[i] = NULL; return 2;}
	}
	return 0;
}

int tokenize(char *buf, char *delims, char *tokens[], int max) {  // cmd 토큰화 함수
	int token_count = 0;
	char *token = strtok(buf, delims);
	while(token != NULL && token_count < max) {
		tokens[token_count] = token; token_count++;
		token = strtok(NULL, delims);
	}
	tokens[token_count] = NULL; 
	return token_count;
}

bool run(char *line) {  // 수행 함수
    char delims[] = " \n";
    char *tokens[MAX_SYS];
    pid_t pid;
    int i, stat, chk;
	int token_count = tokenize(line, delims, tokens, sizeof(tokens) / sizeof(char*));
	if(token_count == 0) return true; 
	chk = check(tokens); 
	for(i = 0; i < 4; i++) { 
        if(strcmp(cmd[i].key, tokens[0]) == 0)
        	return cmd[i].intr_func(token_count, tokens);
    }
	if((pid=fork()) < 0) {
		perror("Fork error caused\n"); 
		exit(-1);
	}
	else if(pid == 0) {
		if(chk == 1){  // redirection 실행
		cmd_redirection(token_count, tokens);
		execvp(tokens[0], tokens);
		}
		else {  // basic logic 실행
			execvp(tokens[0], tokens);
			printf("Execute failed\n"); 
			exit(-1);
		}
	}
	else {
		if(chk == 2) {  // background 수행
			waitpid(pid, &stat, WNOHANG); 
			sleep(1);
		}
		else waitpid(pid, &stat, 0);
	}
	return true;
}