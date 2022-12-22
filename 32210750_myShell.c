/**
* shell.c		:	Command interpreter and etc.(works like shell)
* @author		:	Seeun Kim
* @eamil		:	rlaserma1014@dankook.ac.kr
* @version		:	1.0
* date			:	2022.12.04
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#define SIZE 1024 // 입력 받을 문자열의 최대 크기 지정

void help();
void cd(int argc, char* argv[]);
void input_redirction(char* tokens[], int pos);
void output_redirction1(char* tokens[], int pos);
void output_redirction2(char* tokens[], int pos);
bool run(char* line);
int tokenize(char* buf, char* del, char* tokens[], int max);

int main(void){
	char cmd[SIZE]; // 입력 받은 문자열(명령어)을 저장할 문자열 선언
	do{
		printf("[Serma shell]: %s $ ", get_current_dir_name()); // 파일 경로와 쉘 이름 출력
		fgets(cmd, sizeof(cmd)-1, stdin);
	}while(run(cmd) != false); // 만약 run() 함수의 반환값이 false이면 do-while문 종료

	return 0;
}


void help(){ // 프로그램에 구현된 명령어 나열 및 사용법 설명
	printf("/***********COMMANDS**********/\n");
	printf("All commands in shell is available\n");
	printf("Ex:\n");
	printf("  [command] > [file]: save output to file\n");
	printf("  [command] >> [file]: add output");
	printf("  [command] < [file]: get input from file\n");
	printf("  [command] &: run on background\n");
	printf("  cd [directory]: change directory\n");
}

void cd(int argc, char* argv[]){ // cd 명령어 구현
	if(argc == 1)
		chdir(getenv("HOME")); // 인자가 1개 입력 되었으면(cd) HOME 디렉토리로 이동
	else if(argc == 2){ // 인자가 2개 입력 되었을 때
		if(chdir(argv[1])) // 해당 디렉토리로 이동 및 오류 처리
			printf("Can't find directory: '%s'\n", argv[1]); // chdir() 의 반환값이 0보다 크면 오류 메시지 출력
	}
	else
		printf("Wrong format! Enter 'help' to get help\n"); // 인자가 1개 혹은 2개가 아니라면 오류 메시지 출력
}

void input_redirction(char* tokens[], int pos){ // 입력 재지정 구현
	int fd = open(tokens[pos + 1], O_RDONLY); // fd 생성(읽기 전용 옵션)
			
			if(fd < 0){ // 예외 처리
				printf("Can't open '%s' with errno %d\n", tokens[pos + 1], errno);
				close(fd);
				exit(-1);
			}
			
			dup2(fd, STDIN_FILENO); // dup2로 STDIN_FILENO에 fd 복사
			close(fd);
}

void output_redirction1(char* tokens[], int pos){ // 출력 재지정 '>'
	int fd = open(tokens[pos + 1], O_WRONLY | O_CREAT | O_TRUNC, 0641); // fd 생성(쓰기 전용, 파일이 없으면 생성, 있으면 삭제하고 생성하는 옵션)
			
			if(fd < 0){ // 예외 처리
				printf("Can't open '%s' with errno %d\n", tokens[pos + 1], errno);
				close(fd);
				exit(-1);
			}
			
			dup2(fd, STDOUT_FILENO); // dup2로 STDOUT_FILENO에 fd 복사
			close(fd);
}

void output_redirction2(char* tokens[], int pos){ // 출력 재지정 '>>'
	int fd = open(tokens[pos + 1], O_WRONLY | O_CREAT | O_APPEND, 0641); // fd 생성(쓰기 전용, 파일이 없으면 생성, 있으면 파일 끝 부분에 추가하는 옵션)
			
			if(fd < 0){ // 예외 처리
				printf("Can't open '%s' with errno %d\n", tokens[pos + 1], errno);
				close(fd);
				exit(-1);
			}
			
			dup2(fd, STDOUT_FILENO); // dup2로 STDOUT_FILENO에 fd 복사
			close(fd);
}

bool run(char* line){ // 명령어 해석기 구현
	char del[] = " \n"; // 문자열이 끝남을 알리는 공백 문자
	char* tokens[128]; // 입력 받은 문자열을 공백을 기준으로 자른 결과를 저장할 문자열
	int tok_cnt, i, status;
	// tok_cnt : 잘린 문자열 개수(=argc)
	// i: 반복문에 들어갈 변수
	// status: 프로세스의 상태를 전달받을 변수
	int pos = 0; // &, <, >, >> 를 찾은 위치를 저장할 변수
	int cmd_flg = 0; // 어떤 명령어인지 구분할 플래그 변수
	pid_t pid; // 프로세스 ID가 저장된 변수
	
	tok_cnt = tokenize(line, del, tokens, sizeof(tokens)/sizeof(char*)); // 문자열을 자르는 함수를 호출하여 반환된 값을 tok_cnt에 저장
	
	if(tok_cnt == 0){ // 예외 처리
		return true;
	}
	
	if(strcmp(tokens[0], "exit") == 0) // exit 가 입력되면 false 반환
		return false;
	
	if(strcmp(tokens[0], "quit") == 0) // quit 가 입력되면 false 반환
		return false;
	
	if(strcmp(tokens[0], "help") == 0){ // help 가 입력되면 help() 함수를 호출하고 true 반환
		help();
		return true;
	}
	
	if(strcmp(tokens[0], "cd") == 0){ // cd 가 입력되면 cd() 함수를 호출하고 true 반환
		cd(tok_cnt, tokens);
		return true;
	}
	
	for(i=0; i<tok_cnt; i++){ // tok_cnt 만큼 반복 하며 &, <, >, >> 찾기
		if(strcmp(tokens[i], "&") == 0){
			cmd_flg = 1;
			tokens[i] = NULL;
			break;
		}

		if(strcmp(tokens[i], "<") == 0){
			cmd_flg = 2;
			tokens[i] = NULL;
			pos = i;
			break;
		}

		if(strcmp(tokens[i], ">") == 0){
			cmd_flg = 3;
			tokens[i] = NULL;
			pos = i;
			break;
		}
		
		if(strcmp(tokens[i], ">>") == 0){
			cmd_flg = 4;
			tokens[i] = NULL;
			pos = i;
			break;
		}
	}
	
	pid = fork(); // 자식 프로세스 생성
	
	if(pid < 0){ // 예외 처리
		perror("fork error");
		exit(-1);
	}
	else if(pid == 0){ // 자식 프로세스일 때 실행 
		
		if(cmd_flg == 2){ // < 가 tokens에 포함되어 있을 때 실행
			input_redirction(tokens, pos); // 입력 재지정 함수 호출
		}
		else if(cmd_flg == 3){ // > 가 tokens에 포함되어 있을 때 실행
			output_redirction1(tokens, pos); // 출력 재지정1 함수 호출
		}
		else if(cmd_flg == 4){ // >> 가 tokens에 포함되어 있을 때 실행
			output_redirction2(tokens, pos); // 출력 재지정2 함수 호출
		}
		
		execvp(tokens[0], tokens); // 현재 프로세스를 중단하고 tokens[0]를 실행 tokens를 argv로 받는 새로운 프로세스 생성
		printf("Can't find command: '%s'\n", tokens[0]); // 예외처리
		exit(-1);
	}
	else{
		if(cmd_flg != 1) // 백그라운드 프로세스(&)가 아닐 때 실행
			waitpid(pid, &status, 0); // 부모 프로세스가 자식 프로세스를 기다림(0)
		else // 백그라운드 프로세스일 때
			waitpid(pid, &status, WNOHANG); // 부모 프로세스가 자식 프로세스를 기다리지 않음(WNOHANG)
	}
	
	return true; // 모든 실행이 끝나면 true 반환
}

int tokenize(char* buf, char* del, char* tokens[], int max){ // 문자열 parsing 구현
	int cnt = 0; // 잘린 문자열의 개수를 저장할 변수
	char* token; // 잘린 문자열을 저장할 변수
	token = strtok(buf, del); // 첫 토큰 가져오기

	while (token != NULL && cnt < max) { // 문자열이 끝날 때 반복 종료
		tokens[cnt++] = token; // 잘린 문자열을 tokens에 저장
		token = strtok(NULL, del);			// 다음 문자열을 가져옴
	}
	tokens[cnt] = NULL;				// tokens 마지막에 NULL을 추가해 문자열이 끝남을 알림

	return cnt; // cnt 반환
}
