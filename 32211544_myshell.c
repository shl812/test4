/**
 * 32211544_myshell.c : make a shell
 * DATE : 2022.12.08
 * NAME : ChangJun Mun
 * ID : 32211544
**/

#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<wait.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>
#include<signal.h>
#include<errno.h>
#define MAX 100

void background(char** commands, int i); 
void redirection(char** commands, int i);
void execute(char** commands);
char** tokenize(char* buf);

int main(){
	puts("32211544_MunChangJun_Shell");
	while(1){
		char buf[MAX] = {0};
		char PWD[MAX] = {0};
		char** command;
		int read_size;
		int errno;
		getcwd(PWD, MAX);
		write(STDOUT_FILENO, "@Shell:~", 20);
		write(STDOUT_FILENO, PWD, MAX);
		write(STDOUT_FILENO, "# ",3);
		read_size = read(STDIN_FILENO, buf, MAX); // 사용자가 입력한 커맨드 읽기
		if(read_size == 1) continue;
		command = tokenize(buf); // 명령어 parsing
		if(strcmp(command[0], "quit") == 0){
			printf("FINISH Shell\n");
			exit(1);
		}
		else execute(command); // 명령어 실행
		}
	}

char** tokenize(char* buf){ // 명령어 parsing 함수
    char **ary; // 저장할 배열
    char *token; 
    int count = 0;
    buf[strlen(buf) - 1] = '\0'; // 문자열 길이 재설정
    ary = malloc((sizeof(char*))*MAX); // 배열 길이 할당
    token = strtok(buf, " "); // "" 구분자 기준으로 parsing
    while(token != NULL){
        ary[count] = malloc(strlen(token)+1); // \0문자를 위해 +1
        strncpy(ary[count], token, strlen(token)+1);
        token = strtok(NULL, " ");
        count++;
    }
    ary[count] = NULL;
    
    return ary;
}


void execute(char **commands){ // 명령어 실행 함수
	int process;
	int status;
	int i=0;
	char *dir;
	if(strcmp(commands[0], "cd") == 0){
		if(commands[1] == NULL){
			printf("USEAGE : cd directory \n");
		}
		chdir(commands[1]); // 두번째 입력받은 명령어로 작업 디렉토리 변경
	}
	else if(strcmp(commands[0], "pwd") == 0){
		printf("%s\n", getcwd(dir, MAX)); // 현재 작업 경로 출력
	}
	else{
		if((process = fork()) == 0){ // 자식 프로세스 생성
			while(commands[i]){ 
				if(strcmp(commands[i], ">") == 0){
					redirection(commands, i);
				}
				else if(strcmp(commands[i], "&") == 0){
					background(commands, i);
				}
				i++;
			}
			if(execvp(commands[0], commands)==-1){
				printf("%s : command not found\n", commands[0]);
				exit(1);
			}
		}
		else if(process<0){
			printf("fork failed\n");
			exit(1);
		}
		else
			wait(&status); // wait을 통해 부모 프로세스가 자식 프로세스가 종료될 때까지 대기
	}
}

void redirection(char **commands, int i){ // redirection 함수
	char *depart = commands[i+1]; // 목적지 함수
	int fd;
	
	fd = open(depart, O_WRONLY | O_CREAT, 0641);
	if(fd < 0){
		printf("open error\n");
		exit(1);
	}
	dup2(fd, STDOUT_FILENO); // STDOUT_FILENO가 fd를 가리키게 함
	commands[i] = NULL;
	if(execvp(commands[0], commands)==-1){
		printf("No such commands : %s\n", commands[0]);
		exit(1);
	}
}

void background(char **commands, int i){ // background 함수
	commands[i] = NULL;
	int process;
	if((process = fork()) == 0){
		if(execvp(commands[0], commands) == -1){
			printf("No such commands : %s\n", commands[0]);
			exit(1);
		}
	}
	else if(process < 0){
		printf("fork failed\n");
		exit(1);
	}
	else exit(1); // wait() 함수를 쓰지 않음
}
