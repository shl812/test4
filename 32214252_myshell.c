/**
* mysh.c	: Shell
* @author	: hyeyoung Jung
* @email	: junghy@dankook.ac.kr
* @version	: 1.0
* @date		: 2022.12.09
**/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h> //strcmp
#include <stdbool.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h> //시그널 핸들러

pid_t child;												//pid
int exit_status = -1;										//자식의 상태

//command 구조체
struct cmd {	
	char* name;												//cmd 명령어 이름
	char* desc;												//명령어 설명
	bool(*func)(int argc, char* argv[]);					//명령어 함수
};

//cmd 나가기
bool cmd_exit(int argc, char* argv[]);	
//cmd 명령어 도움말
bool cmd_help(int argc, char* argv[]);						

//cmd 구조체 배열
struct cmd work_cmd[] = {									
	{ "exit",	"exit this shell",	cmd_exit },
	{ "quit",	"quit tihs shell",	cmd_exit },
	{ "help",	"show this help",	cmd_help },
	{ "?",		"show this help",	cmd_help }
};

int size = sizeof(work_cmd) / sizeof(struct cmd);			//work_com의 크기

//cmd를 나가는 함수
bool cmd_exit(int argc, char* argv[]) {						
	return false;
}

//cmd의 명령어 이름과 설명을 출력하는 함수
bool cmd_help(int argc, char* argv[]) {						
	int i = 0;
	for (i = 0; i < size; i++) {
		printf("%-10s: %s\n", work_cmd[i].name,
				work_cmd[i].desc);
	}
	return true;
}

//신호 handling 
void handler(int signo){								        
    while((child = waitpid(-1, &exit_status, WNOHANG)>0)){}    
	//임의의 자식 프로세스를 기다림 
}

//redirect구현 함수
int redirection(int t, int idx, char* argv[]){				
	int fd,i;
	if(t==0){													//">"인 경우
		if ((fd = open(argv[idx+1], O_RDWR | O_CREAT, 0641)) < 0) {
		printf("Can't open %s file with erron %d\n", argv[idx+1], errno);
		exit(-1);
		}
	}else {													//">>"인 경우 append로 offset을 맨 마지막으로 옮기기
		if ((fd = open(argv[idx+1], O_RDWR | O_CREAT | O_APPEND, 0641)) < 0) {
		printf("Can't open %s file with erron %d\n", argv[idx+1], errno);
		exit(-1);
		}
	}
	dup2(fd, STDOUT_FILENO);								//dup2함수로 경로를 재지정 해준다.
	close(fd);
	for(i = idx; argv[i] !=NULL; i++){
		argv[i] = argv[i+2];
	}
	argv[i] = NULL;											//">",">>"부분을 NULL처리한다.
	return 0;
}

//입력된 문자열을 delims 기준으로 분리하여 tokens 배열에 저장하는 함수
int tokenize(char* buf, char* delims, char* tokens[], int maxTokens) {
	int token_count = 0;
	char* token;
	
	token = strtok(buf, delims);
	while (token != NULL && token_count < maxTokens) {
		tokens[token_count] = token;
		token_count++;
		token = strtok(NULL, delims);
		
	}
	tokens[token_count] = NULL;
	return token_count;
}

//실행 함수
bool run(char* line) {
	char delims[] = " \r\n\t";								//문자열을 줄바꿈,탭키, 캐리지 단위로 나눔 
	char* tokens[128];										//분리된 문자열 저장하는 배열
	int token_count;										//분리된 개수
	int i;													//반복문 변수
	int maxTokens = sizeof(tokens) / sizeof(char*);			//최대 분리 가능 개수
	bool background = false;								//background 수행 여부, 기본은 false
	int t  = -1;											//redirection 타입, >인지 >>인지
	int idx  = 0;											//tonkens배열에서 >,>>이 있는 위치 
	
	for(i = 0; i<strlen(line); i++){
		if(line[i]=='&'){
			background = true;								//입력 line배열에 &가 있으면 background processing 한다. 
			line[i] = '\0';
			break;
		}
	}
	
	//토큰 분리 함수 실행
	token_count = tokenize(line, delims, tokens, maxTokens); 
	
	//분리된게 없으면 true반환하여 다시 시작
	if (token_count == 0) {
		return true;
	}
	//분리된 토큰에 >, >>문자열 있는지 확인
	for(i = 0; i < token_count; i++){
		if(strcmp(tokens[i],">")==0){
			t = 0;
			idx = i;
			break;
		}else if(strcmp(tokens[i],">>")==0){
			t = 1;
			idx = i;
			break;
		}
	}
	
	//cmd 함수 호출
	for (i = 0; i < size; i++) {
		if (strcmp(work_cmd[i].name, tokens[0]) == 0)
			return work_cmd[i].func(token_count, tokens);
	}
	
	child = fork();											//fork 자식프로세스 만듦
	if (child == 0) {
		if(t>=0){
			redirection(t, idx, tokens);					//t값이 주어져 있으면 redirection함수로 출력 경로 재지정해줌
		}
		execvp(tokens[0], tokens);							//자식은 tokens[0] 프로그램을 덮어씌움
		printf("No such file\n");
	} else if (child < 0) {
		printf("Failed to fork()\n");
		exit(1);
	} else if(background==false) {							//background가 true면 wait 하지 않음
		wait(&exit_status);
	}
	
	return true;
	
}

int main() {
	char line[1024];
	while(1) {
		//signal 함수를 통해 handler 함수 호출하여 SIGCHLD 신호 처리
		//SIGCHLD는 프로세스가 종료되었을때 부모에게 전달하는 신호로, background로 실행시 좀비프로세스가 되지 않도록 함
		signal(SIGCHLD,handler);							
		printf("%s$ ", get_current_dir_name());
		fgets(line, sizeof(line) - 1, stdin);
		if (run(line) == false) {
			break;
		}
	}
	return 0;
}
