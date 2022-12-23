/**
* myshell.c : make a shell
* @author : 시스템프로그래밍3분반/서현우/32192139/컴퓨터공학
* @email : gusdntj@gmail.com
* @version : Ubuntu 18.04.2
* date : 2022-12-05
**/
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include <fcntl.h>
#include<stdbool.h>

//콘솔창을 지우기 위해  ANSI이스케이프코드 활용해서 clear() 구현
#define clear() printf("\033[H\033[J")

// 백그라운드 실행, 리다이렉션 체크를 위한 전역 변수
int background = 0, redirection1 = 0, redirection2 = 0;;

// 쉘 진입시 화면
void ShellOn()
{
	clear();
	printf("\n\n\n\n--------------------------------------------------"
		"\n***********************************************");
	printf("\n\n\n\t WELCOME TO MY SHELL");
	printf("\n\n\n\n***********************************************"
		"\n--------------------------------------------------");
	printf("\n");
	sleep(2);
	clear();
}

//입력받은 문자들을 토큰화
int tokener(char* buf, char* splits, char* tokens[], int maxsize){
	int count=0;
	char* token;
	
	token = strtok(buf, splits);
	while(count < maxsize && token != NULL){
		tokens[count++] = token;
		token = strtok(NULL, splits);
	}
	tokens[count] = NULL;
	return count;
}

// 입력받은 명령어들 각각의 실행방법 지정
bool play(char* buf){
	int count, out, bg, re1, re2, i;
	char* tokens[512];
	char* splits = " \n\t";
	pid_t pid = 0;
	background = 0; 
	redirection1 = 0; 
	redirection2 = 0;
	
	count = tokener(buf, splits, tokens, sizeof(tokens) / sizeof(char*));
	// 백그라운드 실행이나 리다이렉션이 있는지 검사 후 특수 문자들의 인덱스 저장
	for(i=0; i < count; i++){	
		if(strcmp(tokens[i], "&") == 0){
			background++;
			bg = i;
		}
		if(strcmp(tokens[i], ">") == 0){
			redirection1++;
			re1 = i;
		}
		if(strcmp(tokens[i], ">>") == 0){
			redirection2++;
			re2 = i;
		}
	}
	
	// cd 명령어 구현
	if(strcmp(tokens[0], "cd") == 0){
		if(chdir(tokens[1]) != 0)
			perror("error");
		return 1;
	}
	
	// exit 명령어 구현
	if(strcmp(tokens[0], "exit") == 0){
		printf("GOOD BYE\n");
		return false;
	}
	
    if(background == 1){ // 백그라운드 실행 & 구현
		if((pid = fork())== 0){
			tokens[bg] = NULL;
			printf("pid = %d \n", getpid());
			execvp(tokens[0], tokens);
			exit(0);
		}
		//기다리지 않음
	} else if(redirection1 == 1){ // 리다이렉션 > 구현
		printf("> 실행\n");
		if((pid = fork())==0){
			out = open(tokens[3], O_RDWR | O_CREAT | O_TRUNC, 0644);
			dup2(out, 1);
			close(out);
			tokens[re1] = NULL;
			execvp(tokens[0], tokens);
			exit(0);
		}
		wait(NULL);
	} else if(redirection2 == 1){ // 리다이렉션 >> 구현
		printf(">> 실행\n");
		if((pid = fork())==0){
			out = open(tokens[3], O_RDWR | O_APPEND, 0644);
			dup2(out, 1);
			close(out);
			tokens[re2] = NULL;
			execvp(tokens[0], tokens);
			exit(0);
		} 
		wait(NULL);	
	} else if(background == 0){ // 그 외 명령어들 구현
		if((pid = fork())==0){
			execvp(tokens[0], tokens);
			exit(0);
	    }
	}
	sleep(1);
	memset(tokens, '\0', 512);
	return 1;	
}


int main(){
	char buf[512];
	char cwd[512];
	ShellOn();
	
	/* 현재 경로를 보여주고 사용자의 입력을 기다리는 동작을 반복하다
	exit 명령어를 입력받으면 종료 */
	while(1){
		getcwd(cwd, sizeof(cwd)); // 현재 디렉토리 가져오기
		printf("\nroot@hyeonu: %s", cwd);
		printf("\n>>> ");
		fgets(buf, sizeof(buf)-1, stdin);
		if(play(buf) == false)
			break;
	}
	return 0;
}