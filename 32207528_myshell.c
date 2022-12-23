/**
* myshell.c	: make a shell
* @author	: JiSoo Yang
* @email	: yjs229@dankook.ac.kr
* @version	: 1.0
* @date		: 2022.12.09
**/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

void help() { // help() 호출시 명령어들에 대한 정보 출력
	printf("------------------------------<My Shell>------------------------------\n");
	printf("My Shell에서 기본적으로 제공되는 명령어들은 다음과 같습니다.\n");
	printf("help\t: 도움말을 보여줍니다.\n");
	printf("exit\t: 실행중인 Shell을 종료합니다.\n");
	printf("ls\t: 현재 파일 또는 디렉토리의 목록을 출력해줍니다.\n");
	printf("date\t: 현재의 날짜와 시간을 보여줍니다.\n");
	printf("pwd\t: 최근에 실행된 디렉토리의 절대경로를 보여줍니다.\n");
	printf("mkdir [옵션명] [디렉토리명]: 새로운 디렉토리를 만들어줍니다.\n");
	printf("rmdir [옵션명] [디렉토리명]: 특정 디렉토리를 지워줍니다.\n");
	printf("cp [옵션명] [복사할파일명] [파일명]: 파일의 내용을 복사해줍니다.\n");
	printf("cat [옵션명] [파일명]: 파일의 내용을 보여줍니다.\n");
	printf("&\t: background processing\n");
	printf("[명령어] [복사할 파일명] > [새 파일명]: redirection(내용복사)\n");
	printf("[명령어] [복사할 파일명] >> [붙여넣을 파일명]: redirection(내용복사하여 붙여쓰기)\n");
	printf("----------------------------------------------------------------------\n");
}

int tokenize(char* buf, char* delims, char* tokens[], int maxtokens) {
	int tok_count = 0;
	char* tok;

	tok = strtok(buf, delims); 

	while(tok != NULL && tok_count < maxtokens) {
		tokens[tok_count] = tok;
		tok_count++;	
		tok = strtok(NULL, delims);
	}
	// parshing
	tokens[tok_count] = NULL;
	return tok_count;
}

bool run(char* input) {
	int i;
	int fd;
	int red = 0;
	int bg = 0;
	int red_check = 0;
	int bg_check = 0;
	int tok_count;
	pid_t child;
	char delims[] = " \r\t\n";
	char* tokens[128];
	char* tok;
	
	tok_count = tokenize(input, delims, tokens, sizeof(tokens)/sizeof(char*));

	if(tok_count == 0) // 아무런 입력이 없을 경우 
		return true;
	
	if(strcmp(tokens[0], "exit") == 0) // tokens[0]에 위치한 문자열과 exit을 서로 비교하여 두 문자열이 같다면 false를 반환
		return false;	

	if(strcmp(tokens[0], "help") == 0){ // tokens[0]에 위치한 문자열과 help를 서로 비교하여 두 문자열이 같다면
		help(); // help()메소드 호출
		return true;
	}
	
	// redirection, background processing
	for(i = 0; i < tok_count; i++) {
		if(strcmp(tokens[i],">") == 0) {
			red = i;
			red_check = 1;
			break;
		}
		if(strcmp(tokens[i],">>") == 0) {
			red = i;
			red_check = 2;
			break;
		}
		if(strcmp(tokens[i],"&") == 0) {
			bg = i;
			bg_check = 1;
			break;
		}
	}	

	child = fork();
	if(child < 0) { // fork error발생시
		printf("fork 실행오류\n");
		return false;
	}
	else if(child == 0) { // 자식프로세스이면
		if(bg_check) {
			tokens[bg] = '\0';
		}
		if(red_check == 1) { // > 입력시 출력 재지정
			fd = open(tokens[red + 1], O_WRONLY|O_TRUNC|O_CREAT, 0664); // 새로쓰기
			close(STDOUT_FILENO);
			dup2(fd, STDOUT_FILENO);
			tokens[red] = '\0';
		}
		if(red_check == 2) { // >> 입력시 출력 재지정
			fd = open(tokens[red + 1], O_RDWR|O_APPEND, 0664); // 이어서 쓰기
			close(STDOUT_FILENO);
			dup2(fd, STDOUT_FILENO);
			tokens[red] = '\0';
		}
		if(bg_check == 1) { // & 입력시 한줄을 띄우고 입력을 받음
         printf("\n");
         tokens[bg] = '\0';
      	}
		
		
		if(strcmp(tokens[0],"ls")==0){
        	execve("/bin/ls",tokens,NULL);
		}else if(strcmp(tokens[0],"date")==0){
        	execve("/bin/date",tokens,NULL);
      	}else if(strcmp(tokens[0],"pwd")==0){
        	execve("/bin/pwd",tokens,NULL);
      	}else if(strcmp(tokens[0],"rmdir")==0){
        	execve("/bin/rmdir",tokens,NULL);
      	}else if(strcmp(tokens[0],"mkdir")==0){
        	execve("/bin/mkdir",tokens,NULL);
      	}else if(strcmp(tokens[0],"ps")==0){ 
        	execve("/bin/ps",tokens,NULL);
      	}else if(strcmp(tokens[0],"cp")==0){
        	execve("/bin/cp",tokens,NULL);
		}else if(strcmp(tokens[0],"cat")==0){
        	execve("/bin/cat",tokens,NULL);   
      	}else{
			execvp(tokens[0],tokens);
		}
	
		printf("execvp 오류\n"); //이 문장 출력시 execvp가 제대로 안됨 (이어서 출력한다는 것은 다른프로그램이 실행되지 않았음을 의미)
		return false;
	}
	else if(bg_check == false) {
		wait(NULL);
	}
	return true;
}	

int main() {
	char line[1024];
	
	while(1) {
		printf("%s $ ", getcwd(line, 1024));
		fgets(line, sizeof(line) - 1, stdin);
		if(run(line) == false)
			break;
	}
	
	return 0;
}