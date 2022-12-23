/**
 * myshell.c		: shell program
 * @author			: Hyunggyun Kim
 * @email			: 32211307.dankook.ac.kr
 * @version			: 1.0
 * @studentID		: 32211307
 * @date			: 2022. 12. 09
**/

#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#define MAX_BUF 16

char* token;  // 토큰을 끊을 때 잠시 저장해둘 문자열
int token_count;  // 토큰 갯수
char delims[] = " \t\n";  // 문자열 나누는 기준
char* tokens[100];  // 토큰들을 저장할 배열
pid_t pid;  // 프로세스 아이디
int i;  // 인덱스 값

// parsing
// delimn를 기준으로 입력받은 문자열을 끊어서 tokens의 저장하고 토큰 갯수 반환
int tokenize(char* buf, char* delims, char* tokens[], int maxTokens){  
	token_count = 0;  // 초기화
	token = strtok(buf, delims);  // delims를 기준으로 슬라이싱
	while(token != NULL && token_count < maxTokens){  // 토큰을 끊어가며 tokens에 저장
		tokens[token_count] = token;
        token = strtok(NULL, delims);
		token_count++;
	}
	return token_count;  // 토큰 수 반환
} 

bool run(char* line){
	bool background = false;  // 백그라운드 실행 여부를 나타내는 변수
	int fd1,size;
	// tokenize 호출해서 토큰 저장 및 토큰 갯수 반환 받기
	token_count = tokenize(line, delims, tokens, sizeof(tokens) / sizeof(char*));  
	if(!strcmp(tokens[0],"exit"))  // 처음에 exit를 입력한 경우 false 반환
		return false;
	// fork()
	pid = fork();
	if(pid < 0){  // fork 실패시 강제 종료
		perror("fork error");
		exit(1);
	} else if(pid == 0){  // 자식 프로세스
		// 백그라운드 실행
		for(i=0;i<token_count;i++){
			if(!strcmp(tokens[i],"&")){  // 토큰에 "&"이 있는 경우
				tokens[i] = NULL;  // &이 저장된 부분 NULL로 변경
				background = true;  
				break;
			}
		}
		// redirection
		for(i=1;i<token_count;i++){
			if(!strcmp(tokens[i],">")){  // '>'을 입력
				fd1 = open(tokens[i+1],O_RDWR);  // '>' 다음에 오는 토큰을 이름으로 하는 파일 열기
				if(fd1<0){  // 파일이 없는 경우
					fd1 = open(tokens[i+1],O_RDWR|O_CREAT,0641);  // 해당 토큰 이름으로 파일 생성
				} 
				dup2(fd1, STDOUT_FILENO);  // 출력 경로 fd1으로 변경
				close(fd1);  
				// 쓰레기 값 제거
				tokens[i] = NULL;  
				tokens[i+1] = NULL;
				for(i = i; tokens[i] != NULL; i++){  
					tokens[i] = tokens[i+2];
				}			
				break;
			}
			if(!strcmp(tokens[i],">>")){  // '>>'을 입력
				fd1 = open(tokens[i+1],O_RDWR|O_APPEND);  // 원래 있던 값에 이어 작성할 수 있도록 O_APPEND옵션 추가해서 열기
				if(fd1<0){
					fd1 = open(tokens[i+1],O_RDWR|O_CREAT,0641);
				} 
				dup2(fd1, STDOUT_FILENO);
				close(fd1);
				tokens[i] = NULL;
				tokens[i+1] = NULL;
				for(i = i; tokens[i] != NULL; i++){
					tokens[i] = tokens[i+2];
				}			
				break;
			}
			if(!strcmp(tokens[i],"<")){  // '<'을 입력
				fd1 = open(tokens[i+1],O_RDWR);
				if(fd1<0){
					perror(tokens[i+1]);
					return false;  // 오류 발생시 false 리턴
				} 
				dup2(fd1, STDIN_FILENO);  // 입력 경로 fd1으로 변경
				close(fd1);
				// 쓰레기 값 제거
				for(i = i; tokens[i] != NULL; i++){
					tokens[i] = tokens[i+2];
				}			
				tokens[i] = NULL;
				break;
			}
		}
		// execve()
		execvp(tokens[0], tokens);  
	} else if(background == false) {  // 백그라운드 실행이 아닌 경우는 부모 프로세스가 자식 프로세스의 종료를 기다리고, 백그라운드 실행의 경우 wait()작동 X
		wait(); 
	}
	return true;  // 문제 없이 진행된 경우 true 반환
}

int main(){
	char line[1024];  // 문자열을 입력 받을 배열
	while(true){
		printf("%s $ ", get_current_dir_name() );  // 디렉토리 이름 출력
		fgets(line, sizeof(line) - 1, stdin);  // 입력 받은 문자열 저장
		if(run(line) == false)  // run함수 호출해서 false를 리턴할 때까지 반복
			break;
	}
	return 0;
}
