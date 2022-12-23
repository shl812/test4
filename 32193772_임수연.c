/**
* myshell.c  : shell implement program
* @author    : Sooyeon Lim 32193772
* @email     : 32193772@dankook.ac.kr
* @date      : 22. 12. 09
**/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#define MAX_BUF 64
int parsing(char* buf, char* delims, char* tokens[], int maxTokens);
int myrd(char* line);

// 입력받은 명령어를 적절히 parsing해 token 배열로 저장하는 함수
int parsing(char* buf, char* delims, char* tokens[], int maxTokens) {  
	char* token;   // 분리된 token
	int t_count=0;
	token = strtok(buf, delims);    // strtok함수 이용해 첫번째 인자 분리
	while(token!=NULL && t_count<maxTokens) {
		tokens[t_count++]=token;    // 분리된 인자를 배열에 저장
		token=strtok(NULL, delims); // NULL이 리턴되면 다음 인자 찾음
	}
	tokens[t_count]=NULL;           // 배열 마지막에 NULL
	return t_count;                 // 분리된 인자 개수 리턴
}

// 입력받은 명령어를 strcmp해 redirection과 background processing 수행하는 함수
int myrd(char* line) {
	char* tokens[512];
	char* temp[512];
	char* delims="\n";
	pid_t child=0;
	int status;
	
	int t_count = parsing(line, delims, tokens, (sizeof(tokens) / sizeof(char*)));
	
	int fd;
	int flag = 0;
	int i,j;   // iterator 변수
	
	for (i=0;i<t_count;i++) {
		// 새로운 파일에 덮어쓰는 ">"
		if(!strcmp(tokens[i],">")) {
			flag = O_WRONLY|O_TRUNC|O_CREAT;
			break;
		}
		// 기존 파일의 마지막에 쓰는 ">>"
		else if (!strcmp(tokens[i],">>")) {
			flag = O_APPEND|O_WRONLY|O_CREAT;
			break;
		}
	}
	if(t_count==0) return 0;    // token이 하나도 없을 때
	else if((child=fork())<0) {    // fork 실패했을 때
		printf("fork failed");
		return -1;
	}
	else if(child==0) {            // 자식 process
		for (j=0;j<i;j++) {
			temp[j]=tokens[j];     // 토큰 하나 임시저장
		}
		if(flag>0) {               // ">" 만나면 file에 출력
			fd=open(tokens[i+1],flag,0664);  // user, group, others의 권한 설정
			close(STDOUT_FILENO);
			dup2(fd, STDOUT_FILENO);         // dup2() 이용해 덮어쓰기
		}
		execve(tokens[0], temp, NULL);
		exit(0);
	}
	else {                         // 부모 process
		if(strcmp(tokens[t_count-1],"&")) {     // 자식 process 종료되지 않았다면
			wait(&status);                      // 부모가 background에서 wait
		} 
	}
	close(fd);
	return 0;
}

int main() {
	char line[512];
	while(1) {
        fgets(line, sizeof(line) - 1, stdin);    // 사용자로부터 dir 이름 입력받기
        if(!(strcmp(line, "exit\n"))) {          // 사용자가 exit 입력하면 프로그램 종료
			break;
		}
		else {
			myrd(line);                          // exit이 아니라면 redirection 함수 호출
		}
    }
    return 0; 
}
