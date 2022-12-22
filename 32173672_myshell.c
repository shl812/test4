/**
 * mysh3.c      : myshell program
 * @author      : Kangho Lim
 * @email       : 32173672@dankook.ac.kr
 * @version     : 3.0
 * @date        : 2022. 12. 09 
**/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#define MAX 100

// 함수 선언부
char** tokenize(char*);
void execute(char**);
void redirection(char**, int);
void background(char**, int);

// 함수 정의부
int main() {
	while(1) {
		char commandLine[MAX] = {0};
		char PWD[MAX] = {0};
		char** command;
		int read_size;
		int errno;

        // get_current_dir_name() 대신 사용
		getcwd(PWD, MAX);
        
        // printf()보다 빠른 write() 사용
		write(STDOUT_FILENO, "@32173672:", 10);
		write(STDOUT_FILENO, PWD, MAX);
		write(STDOUT_FILENO, "# ", 2);
		
		read_size = read(STDIN_FILENO, commandLine, MAX);
		
		if (read_size == 1) {       // 개행(\n) 입력받을 시
			continue;               // 프로그램 계속 진행
		}
		
		command = tokenize(commandLine);    // 입력받은 명령어 나누기
		
		if (!strcmp(command[0], "exit")) {  // exit 입력 시
			exit(1);                        // shell 프로그램 종료
		} else {
			execute(command);               // 입력받은 명령어 실행
		}
	}
}

char** tokenize(char* commandLine) {        
	char** tokens; 
	char *token;
	int count;

	commandLine[strlen(commandLine)-1] = '\0';  // 마지막 문자(\n)를 바꾸기
	tokens = malloc((sizeof(char*))*MAX);       // 토큰을 저장할 크기의 배열 할당
	token = strtok(commandLine, " ");           // " "(띄어쓰기)를 구분자로
	
	while (token != NULL) {                             // token이 있을 경우
		tokens[count] = malloc(strlen(token)+1);        // 마지막 문자(\0)을 포함하기 위해 + 1
		strncpy(tokens[count], token, strlen(token)+1); // 복사
		token = strtok(NULL, " ");              
		count++;
	}

	tokens[count] = NULL;   // 배열 마지막칸 NULL 넣어주기

	return tokens;
}

void execute(char** commands) {
	int forkedProcess;
	int i=0;
	char* directory;

	if (!strcmp(commands[0], "cd")) {
		if (commands[1] == NULL) {
			printf("USAGE: change directory, \"cd [directory name]\" \n");
			chdir("/root");
		}
		chdir(commands[1]);
	} else if (!strcmp(commands[0], "pwd")) {
		printf("%s\n", getcwd(directory, MAX));
	} else {
		if ((forkedProcess = fork()) == 0) {                // fork() 성공 시
			while(commands[i]) {                            // 명령어 차례차례 읽어나가면서 문자열 비교
				if (!strcmp(commands[i], ">")) {            // ">" 리다이렉션
					redirection(commands, i);
				} else if (!strcmp(commands[i], "&")) {     // "&" 백그라운드 실행
					background(commands, i);
				}
				i++;
			}
			if (execvp(commands[0], commands) == -1) {      // PATH에 등록된 명령어 실행
				printf("%s: command not found\n", commands[0]);
				exit(1);
			}
		} else if (forkedProcess < 0) {             // fork() 실패 시
			printf("fork error\n");
			exit(1);
		} else {
			wait(0);
		}
	}
}

void redirection(char** commands, int i) {
	char* from = commands[i-1];                         // ">" 기준 이전 명령어
	char* to = commands[i+1];                           // ">" 기준 이후 명령어
	int fileDescriptor = open(to, O_WRONLY | O_CREAT, 0641);     // read(6) write(4) execute(1)
	
	if (fileDescriptor < 0) {
		printf("open error\n");
		exit(1);
	}

	dup2(fileDescriptor, STDOUT_FILENO);                         // STDOUT_FILENO가 fileDescriptor를 가리켜
	commands[i] = NULL;
	
	if (execvp(commands[0], commands) == -1) {
		printf("No such command: %s\n", commands[0]);
		exit(1);
	}
}

void background(char** commands, int i) {
	commands[i] = NULL;
	int forkedProcess;

	if ((forkedProcess = fork()) == 0) {
		if (execvp(commands[0], commands) == -1) {
			printf("No such command: %s\n", commands[0]);
			exit(1);
		}
	} else if (forkedProcess < 0) {
		printf("fork error\n");
		exit(1);
	} else {
		exit(1);
	}
}

