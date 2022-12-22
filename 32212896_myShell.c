/*
 * myShell.c	: shell program
 * @author	: YoungEun Yoon
 * @email	: 32212896@dankook.ac.kr
 * date		: 2022.12.09
**/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#define MAX_BUF 100
#define MAX_ARG 50

int tokenize(char * buf, char * delims, char * tokens[], int maxTokens);
int cd(int token_count, char * tokens[]);
int redir(char * tokens[], int option);

int main()
{
	int i;
	int token_count;
	
	pid_t fork_return; // fork
	
	char current_path[MAX_BUF]; // 현재 절대경로
	char command_path[MAX_BUF]; // 초기 절대경로
	char input_str[MAX_BUF] = ""; // 명령문 입력
	
	char * tokens[MAX_ARG]; // 명령어 배열
	char * background_processing = NULL; // background processing
	char * redirection = NULL; // redirection
	
	// 초기 myShell이 위치한 절대경로 => 명령어 실행 파일들이 위치한 디렉토리 절대 경로
	realpath("./", command_path);
	
	while(1) {
		// --- input ---
		realpath("./", current_path); // 현재 위치에 대한 절대경로
		printf("user@hostname:%s$ ", current_path); // prompt 출력
		fgets(input_str, MAX_BUF, stdin); // input 입력
		
		// background processing & redirection check
		background_processing = strchr(input_str, '&'); // background processing check
		redirection = strchr(input_str, '>'); // redirection check
		
		// --- parsing ---
		// token_count = tokenize(input_str, " \n", tokens, sizeof(tokens) / sizeof(char*));
		token_count = tokenize(input_str, " \n", tokens, MAX_ARG);
		if(token_count == 0) {
			continue;
		}
		
		
		// --- internal commands ---
		// // exit
		if (strcmp(tokens[0], "exit") == 0) {
			exit(0);
		}
		
		// cd
		if (strcmp(tokens[0], "cd") == 0) {
			if (cd(token_count, tokens) < 0) {
				// cd error
			}
			continue;
		}
		
		
		// --- external commands: fork() and execve() ---
		if ((fork_return = fork()) < 0) { // fork error
			perror("fork error\n");
			exit(1);
		}
		else if (fork_return == 0) { // child
			// 명령어 경로 처리
			char command[MAX_BUF];
			strcpy(command, command_path); // 명령어 파일의 절대경로
			strcat(command, "/");
			strcat(command, tokens[0]);
			
			// redirection
			if (redirection != NULL) {
				if (*(redirection + 1) != '>') { // > redirection
					if (redir(tokens, 0) < 0) {
						return -1;
					}
				}
				else if (*(redirection + 1) == '>') { // >> redirection
					if (redir(tokens, 1) < 0) {
						return -1;
					}
				}
			}
			
			// execve
			if (execv(command, tokens) < 0) {
				printf("%s: command not found\n", tokens[0]);
				return -1;
			}
		} else { // parent
			// background processing
			if (background_processing == NULL) { // not background processing
				pid_t temp;
				int stat;
				if ((temp = waitpid(fork_return, &stat, 0)) < 0) { // wait
					printf("wait error\n");
					exit(0);
				}
			}
		}
	}
	
	return 0;
}

/*
 * int tokenize(char * buf, char * delims, char * tokens[], int maxTokens)
 * buf: object of tokenize
 * delims: 
 * tokens: result array of tokenize
 * maxTokens: max num of tokens
 * return value: int token_count
**/
int tokenize(char * buf, char * delims, char * tokens[], int maxTokens) {
	int i = 0;
	int token_count = 0;
	
	tokens[i] = strtok(buf, delims);
	while(tokens[i] != NULL && token_count < maxTokens) {
		i++; token_count++;
		tokens[i] = strtok(NULL, delims);
	}
	
	return token_count;
}

/*
 * int cd(int token_count, char * tokens[])
 * token_count: 
 * tokens: 
 * return value - 0: 
 * 		- 1:  
**/
int cd(int token_count, char * tokens[]) {
	if(token_count > 2) {
		printf("cd: 인수가 너무 많음\n");
		return -1;
	}
	else if(token_count == 1) { // home으로 이동
		if (chdir("/home") < 0) {
			printf("cd error\n");
			exit(-1);
		}
	}
	else { // token_count == 2
		if(chdir(tokens[1]) < 0) { // 해당 경로로 이동
			printf("cd: %s: 해당 디렉터리로 이동할 수 없습니다\n", tokens[1]);
			return -1;
		}
	}
}

/*
 * int redir(char ** tokens, int option): redirection function
 * tokens: 
 * option: 0(> redirection), 1(>> redirection)
 * return value - 0: success to redirection
 * 		- 1: fail to redirection (error)
**/
int redir(char ** tokens, int option) {
	int fd, i;
	char * op = NULL; // option
	int rIndex = 0; int argc = 0;
	char filepath[MAX_BUF] = "";
	
	// option
	if (option == 0) {
		op = ">";
	}
	else if (option == 1) {
		op = ">>";
	}
	
	// find > or >> token index
	for(i = 0; tokens[i] != NULL; i++) {
		if (strcmp(tokens[i], op) == 0) {
			rIndex = i;
			tokens[i] = NULL;
		}
		argc++;
	}
	
	// arguments error
	if ((argc - rIndex) != 2) {
		printf("USAGE : input \">\" output_file_name\n");
		return -1;
	}
	
	// output file path
	strcat(filepath, tokens[rIndex+1]);
	
	// output file open
	if (option == 0) {
		if ((fd = open(filepath, O_WRONLY | O_CREAT)) < 0) {
			printf("file open error\n");
			exit(-1);
		}
	}
	else if (option == 1) {
		if ((fd = open(filepath, O_WRONLY | O_CREAT | O_APPEND)) < 0) {
			printf("file open error\n");
			exit(-1);
		}
	}
	else {
		printf("redir() option error\n");
		return -1;
	}
	
	// use dup2()
	dup2(fd, 1); // 1: STDOUT
	
	close(fd);
	return 0;
}
