/*
 mysh.c : my shell program
 author : SC park
 email : rik369@dankook.ac.kr
 date : 2022.12.01
*/

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdbool.h>
#define MAX 1024

int tokenize(char* buf, char* delims, char* tokens[], int max) { // 공백으로 나눔
	int token_count = 0;
	char* token = strtok(buf, delims);

	while (token != NULL && token_count < max) { 	// 공백으로 자른 문자열 저장
		tokens[token_count] = token;
		token_count++;
		token = strtok(NULL, delims);
	}
	tokens[token_count] = NULL;
	return token_count;
}

void Help_shell() { // mysh 도움말
	printf("************ SC shell ****************\n");
	printf("cd\t: 디렉토리 변경\n");
	printf("exit\t: SC shell 나가기\n");
	printf("help\t: SC shell 도움말\n");
	printf("**************************************\n");
}

void cmd_cd(char* token) { // cd를 입력했을 때 호출 될 함수
	if (chdir(token) == -1) perror("cd"); 
}

void Redirection(char* token[]) { // 입출력 재지정
	pid_t pid; // 프로세스 id를 받을 pid 변수
	int fd_red;
	fd_red = open(token[3], O_RDWR | O_CREAT | O_TRUNC, 0641);
	dup2(fd_red, STDOUT_FILENO);
	token[2] = NULL; // 2번 인덱스 값인 '>' 처리
	close(fd_red);
	execvp(token[0], token);
	exit(0);
}

void Bg_exec(char* token[]) { // Background process 실행
	pid_t pid;
	int fd_bg;
	if ((pid = fork()) < 0) { 	// 오류 처리
		perror("fork error\n");
		exit(-1);
	}

	else if (pid == 0) { 	// fork()했을 때 자식 프로세스인 경우
		fd_bg = open("/dev/nell", O_RDONLY); 
		dup2(fd_bg, STDOUT_FILENO); // 출력 재지정
		execvp(token[0], token); // 명령어 실행
		exit(0);
	}
}

bool Run(char* instruction) { // 명령어 매개변수 Run 함수
	int token_count, stat;
	int i ;
	int number = 0;
	char* tokens[MAX];
	char delims[] = " \n";
	pid_t pid;

	token_count = tokenize(instruction, delims, tokens, sizeof(tokens) / sizeof(char*)); 

	if (strncmp(tokens[0], "cd", 2) == 0) { 	// token에 저장된 명령어가 cd면 cmd_cd 실행
		cmd_cd(tokens[1]); // InnerCmd를 호출 1로 넘긴걸 받아옴
		
	}

	if (strncmp(tokens[0], "help", 2) == 0) { 	// tokens에 저장된 명령어가 help면 Help_shell 실행
		Help_shell();
	}

	pid = fork(); // 자식 프로세스 생성
	if (pid < 0) {
		perror("fork error\n");
		exit(-1);
	}
	
	else if (pid == 0) { 	// 자식 프로세스인 경우 명령어 실행
		for (i = 0; i < token_count; i++) { 
			if (!strcmp(tokens[i], ">")) { // redirection 실행
				number++;
				Redirection(tokens);
			}
			if (!strcmp(tokens[i], "&")) { // background 실행
				number++;
				Bg_exec(tokens);
				exit(0);
			}
		}
		if (number == 0) { 		// 조건문에서 해당하지 않는 다른 명렁어 실행
			execvp(tokens[0], tokens); 
			exit(0);
		}
	}
	else {
		sleep(1);
	}
	memset(tokens[0], '\0', MAX);
	return 1;
}

int main() {
	char instruction[MAX]; // 명령어 배열
	while (1) { 	// exit가 나올때까지 반복해서 실행
		printf("shell 사용 # "); 
		fgets(instruction, sizeof(instruction) - 1, stdin); // 문자열 입력받아 instruction에 저장
		if (strncmp("exit", instruction, 4) == 0) { 		// exit를 입력하면 while문 빠져나감
			printf("exit mysh.c\n");
			break;
		}
		Run(instruction); 
	}	
	return 0;
}
