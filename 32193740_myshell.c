/**
* myshell program
* author : 임강민
* student ID : 32193740
* date: 2022.12.04
* E-mail : km00429@naver.com
**/

#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<stdbool.h>
#include<fcntl.h>
#include<sys/wait.h>
#include<wait.h>

void cmd_help() { // myshell에서 제공하는 기능 설명
	printf("********** myshell **********\n");
	printf("These shell commands are defined for admin\n");
	printf("Type 'help' to see this list\n");
	printf("Next, the list of commands provided\n");
	printf("cd : change directory\n");
	printf("help : show this help of shell\n");
	printf("exit : exit this shell\n");
	printf("> : output redirection\n");
	printf(">> : output append redirection\n");
	printf("> : input redirection\n");
	printf("& : background processing\n");
	printf("*******************************\n");
}

int tokenize(char* buf, char* delims, char* tokens[], int maxTokens) {  // 공백을 토큰으로 분리하는 함수
	char* token;
	int token_count = 0;

	token = strtok(buf, delims);

	while (token != NULL && token_count < maxTokens) {
		tokens[token_count] = token;
		token = strtok(NULL, delims);
		token_count++;
	}

	tokens[token_count] = NULL;

	return token_count;
}

bool run(char* line) {        // myshell 실행 함수
	char* tokens[1024];
	char* delims = " \n";
	int token_count, fd, i, pid_s, j;
	pid_t child;
	bool background = false;
	int state = 0;
	char* tmp[1024];

	token_count = tokenize(line, delims, tokens, sizeof(tokens) / sizeof(char*));

	for (i = 0; i < token_count; i++) {

		if (strcmp(tokens[i], ">") == 0) {
			fd = open(tokens[i + 1], O_WRONLY | O_TRUNC | O_CREAT, 0664);

			if (fd < 0) {
				printf("Error\n");
				exit(-1);
			}

			close(STDOUT_FILENO);
			dup2(fd, STDOUT_FILENO);  // STDOUT_FILENO가 fd를 가르키게 함
			close(fd);

			break;
		}

		else if (strcmp(tokens[i], ">>") == 0) {
			fd = open(tokens[i + 1], O_APPEND | O_TRUNC | O_CREAT, 0664);

			if (fd < 0) {
				printf("Error\n");
				exit(-1);
			}

			close(STDOUT_FILENO);
			dup2(fd, STDOUT_FILENO);  // STDOUT_FILENO가 fd를 가르키게 함
			close(fd);

			break;
		}

		else if (strcmp(tokens[i], "&") == 0) {
			background = true;

			break;
		}

		else if (strcmp(tokens[i], "cd") == 0) {
			if (token_count == 1)
				chdir(getenv("HOME"));
			else if (token_count == 2)
				chdir(tokens[1]);
			else
				printf("ERROR\n");
			break;
		}
	}


	for (j = 0; j < i; j++) {
		tmp[j] = tokens[j];
	}

	if ((child = fork()) == 0) {
		execvp(tokens[0], tmp);
	}
	else if (child < 0) {
		perror("fork");
		exit(1);
	}
	else {
		if (background == false)
			wait(&pid_s); //wait() 실행
		else
			waitpid(child, &pid_s, WNOHANG); // Background processing 실행 
	}

	return true;
}

int main() {
	char line[1024];

	while (1) {
		printf("myshell:%s# ", getcwd(line, 1024)); // myshell 실행 시 입력 창
		fgets(line, sizeof(line) - 1, stdin);

		if (strcmp(line, "exit\n") == 0)   // exit 입력시 종료
			break;
		else if (strcmp(line, "help\n") == 0) // help 입력시 shell 사용법 메뉴얼 출력
			cmd_help();

		run(line);
	}

	return 0;
}


