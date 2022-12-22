/*
* myshell : 나만의 쉘 만들기
* Name : 차민서
* Number : 32214442
* Date : 2022.12.09
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#define MAXSIZE 64

int background=0, redirection =0; // 뒤에 background processing 인지, redirection 인지 판별하기 위한 변수

//공백을 기준으로 토큰화 하는 함수
int tokenize(char* buf, char* delimeters, char* tokens_buf[],int maxtoken){
	int token_count = 0;
	char* token_res;
	
	// strtok: c언어 내장함수로, 문자열을 문자로 나누어 저장함
	token_res = strtok(buf, delimeters);
	
	while(token_res !=NULL && token_count<maxtoken){
		tokens_buf[token_count]=token_res;
		token_count++;
		token_res=strtok(NULL,delimeters);
	}
	tokens_buf[token_count]=NULL;
	
	return token_count;
}


// 사용자로부터 입력받은 함수르 분석,
// 토큰화와 fork()를 진행하는 함수
bool parse(char* buf){
	char delimeters[] =" "; // " " 기준으로 토큰화
	char *tokens_buf[200]; 
	int count;
	int status;
	pid_t child;
	
	// redirection과 background processing 핸들링
	for(int i=0;i<count;i++){
		if(!strcmp(token_buf[i],">")){
			redirection++;
		}
		if(!strcmp(token_buf[i],"&")){
			background++;
		}
	}
	
	//공백을 기준으로 토큰화 해주는 tokenize 함수 실행
	count = tokenize(buf, delimeters, tokens_buf, sizeof(tokens)/sizeof(char*));
	
	if(count==0){
		return ture;
	}
	
	// fork() 실행
	if((child=fork())==0){ // 자식 프로세스일 경우
		execvp(tokens_buf[0], tokens_buf); // execlp 와 같은 역할
		printf("No such file\n"); // 제대로 실행되면 이 줄부터는 실행되지 않음
		exit(0);
	}
	else if((child=fork)<0){ // 오류 일 경우
		printf("Fail\n");
		exit(0);
	}
	else{ // 부모 프로세스일 경우
		wait(&status);
	}
	return true;
}

//main
int main(){
	char buf[MAXSIZE];
	while(1){
		// 마지막에 \n 이 입력되기 때문에 63 만큼만 가져옴
		fgets(buf, MAXSIZE-1, stdin);
		
		if(parse(buf) == false){ // parse 함수 호출, false 일 경우 break
			break;
		}
	}./
	return 0;
}
