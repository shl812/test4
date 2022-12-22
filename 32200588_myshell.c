/**
 * myshell.c	: make a simple shell
 * @author		: Minji Kim
 * @email		: 32200588@dankook.ac.kr
 * @date		: 2022.12.09
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

// command 구조체
struct command_struct { 
	char* name;
	char* description;
	bool (*func)(int argc, char* argv[]); // 함수 포인터
};

// command 프로토타입 선언
bool cmd_cd(int argc, char* argv[]);
bool cmd_help(int argc, char* argv[]);
bool cmd_quit(int argc, char* argv[]);

struct command_struct cmd[] = {
	{"cd", "change deirectory", cmd_cd},
	{"help", "show help menu", cmd_help},
	{"quit", "quit program", cmd_quit}
};

bool cmd_cd(int argc, char* argv[]) {
	if(argc != 2) { // 잘못된 사용법
		printf("사용법: %s <경로 이름>\n", argv[0]);
	}
	int ret = chdir(argv[1]); // argv[1]로 내가 있는 디렉토리 위치 변경
	if( ret == -1) {
		printf("디렉토리 변경 실패\n");
	}
	return true;
}

bool cmd_help(int argc, char* argv[]) {
	printf("******************MJ_shell******************\n");
	printf("간단하게 shell을 구현한 프로그램입니다.\n");
	printf("사용할 수 있는 명령어의 예로 다음과 같은 것들이 있습니다.\n");
	int i;
	for(i=0; i<3; i++) {
		printf("%s : %s\n", cmd[i].name, cmd[i].description);
	}
	printf("*******************************************\n");
	return true;
}

bool cmd_quit(int argc, char* argv[]) { // shell 종료
	return false; 
}

// redirection, 입출력재지정
void cmd_redirction(int argc, char* argv[]) {
	int fd;
	if(argc == 4) {
		fd = open(argv[3], O_WRONLY | O_CREAT, 0664);
		// argv[3]의 파일을 열어 fd가 가리키게 한다.
		if(fd < 0) { // 에러 처리
			printf("%s file open errno : %d\n", argv[3], errno);
			exit(-1);
		}
		dup2(fd, STDOUT_FILENO); // 표준출력을 fd로 대체
		argv[2] = NULL; // '>'은 NULL로 바꾼다.
	}
	close(fd);
	return;
}

// 입력받은 문자열 line을 token으로 구분하는 함수 tokenize()
int tokenize(char* buf, char* delims, char* tokens[], int maxTokens) {
	char* t;
	int token_count = 0;
	t = strtok(buf, delims);
	
	while( t != NULL && token_count < maxTokens) {
		tokens[token_count++] = t;
		t = strtok(NULL, delims);
	}
	tokens[token_count] = NULL;
	
	return token_count;
}

bool run(char* line) {
	char delims[] = " \n\t";
	char* tokens[1024];
	int token_count;
	pid_t c_pid; // 자식 프로세스 pid
	int stat; // 프로세스 status
	
	// redirection, background processing 확인
	bool isRedirection = false;
	bool isBackground = false;
	if((strchr(line, '>')) != NULL) // '>' 있으면 redirection
		isRedirection = true;
	if((strchr(line, '&')) != NULL) // '&' 있으면 background processing
		isBackground = true;
	
	// 입력받은 명령어 line을 token으로 나누기
	token_count = tokenize(line, delims, tokens, sizeof(tokens)/sizeof(char*));
	if(token_count == 0) // token이 0개면, 빈 줄 입력됨
		return true;
	
	// cd, help, quit 인지 확인하고 맞으면 실행
	int i;
	for(i=0; i<3; i++) {
		if(strcmp(cmd[i].name, tokens[0]) == 0)
			return cmd[i].func(token_count, tokens);
	}
	
	// 외부 명령어 실행
	c_pid = fork();
	if( c_pid < 0) { // 에러 처리
		perror("fork eroor");
		exit(-1);
	}
	else if(c_pid == 0) { // 자식 process
		if(isRedirection == true) {
			cmd_redirction(token_count, tokens);
		}
		execvp(tokens[0], tokens);
	}
	
	if(isBackground == true) // background processing -> $ 바로 돌아옴
		waitpid(c_pid, &stat, WNOHANG);
	else
		waitpid(c_pid, &stat, 0);
	
	return true;
}

int main() {
	char line[1024];
	while(1) {
		printf(" %s : MJ_sh $ ", get_current_dir_name());
		fgets(line, sizeof(line) -1, stdin);
		if(run(line) == false)
			break;
	}
	return 0;
}
