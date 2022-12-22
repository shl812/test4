/**
 * myshell.c	: implement simple shell program
 * @author		: Kyoungmin Jeon
 * @email		: yeon2002.kj@dankook.ac.kr
 * @version		: 1.0
 * @date		: 2022. 12. 08.
**/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>		// strtok() 이용을 위한 library
#define MAX_BUF 64


char cmd_input[MAX_BUF];
int my_argc = 0;
char* my_argv[MAX_BUF];
char* cur_path;
int redirection_idx = -1;
int background = 0;


void split();						// 명령어 분할 함수
void myexit();						// exit 명령어 실행 함수
void mycd(int argc, char* argv[]);	// cd 명령어 실행 함수
void redirection(char* argv[]);		// redirection 명령어 실행 함수


int main(int argc, char *argv[]) {
	pid_t pid;
	cur_path = malloc(MAX_BUF);						// 현재 working directory 경로를 저장 할 변수
	char* cmd = malloc(MAX_BUF);					// 입력받은 명령어의 첫 단어를 저장 할 변수
	
	getcwd(cur_path, MAX_BUF);						// 현재 working directory를 획득
	
	while(1) {
		printf("%s > ", cur_path);					// 현재 working directory를 출력
		
        fgets(cmd_input, MAX_BUF, stdin);			// 명령어를 입력
		cmd_input[strlen(cmd_input) - 1] = NULL;	// 마지막 개행 문자를 NULL로 변환
		
		split();									// 문자 parsing
		
		cmd = my_argv[0];
		
		if (cmd != NULL) {							// fork 후 split(parsing)으로 인하여 cmd == NULL이 될 수 있으므로 방지
			if (strcmp(cmd, "exit") == 0) {			// exit, cd 명령어는 fork가 필요 없어서 포함하지 않음
				myexit();
			}else if (strcmp(cmd, "cd") == 0) {
				mycd(my_argc, my_argv);
			}else {
				switch (pid = fork()) {					// fork 진행: if-else 진행 시 종료되지 않는 문제로 switch 사용
					case -1:							// fork 안되는 경우 에러 발생 후 종료
						printf("fork error\n");
						exit(1);
					case 0:								// 자식 노드인 경우: redirection과 exec 진행
						if (redirection_idx != -1) {
							redirection(my_argv);
						}
						execvp(cmd, my_argv);			// 명령어 실행
						*my_argv = NULL;
						**my_argv = NULL;
					default:							// background 여부를 확인하여 부모의 경우 여부에 따라 기다리거나 기다리지 않음
						if (background == 0) {
							background = 0;
							wait();
						}
						redirection_idx = -1;
						break;
				}
			}
		}
		
		fflush(NULL);
		*cmd_input = NULL;
	}
	
	return 0;
}

void myexit() {
	exit(1);
}

void mycd(int argc, char* argv[]) {
	if (argc == 1) {										// cd 만 입력하는 경우 home으로 이동
		if (chdir("/home") != 0) {
			printf("Fail to change directory to /home\n");
		}
	}else {													// cd 뒤에 주소가 있는 경우 해당 주소로 이동
		if (chdir(argv[argc-1]) != 0) {
			printf("Fail to change directory to %s\n", argv[argc-1]);
		}
	}
	
	getcwd(cur_path, MAX_BUF);			// 현재 working directory를 획득
}

void split() {
	my_argc = 0;
	char* split_str = strtok(cmd_input, " ");		// strtok를 사용하여 split
	
	while (split_str != NULL) {						// split 한 문자가 NULL일 때까지 반복
		my_argv[my_argc] = split_str;				// split 한 문자 저장
		
		if (strcmp(split_str, ">") == 0) {			// redirection 명령어 확인
			redirection_idx = my_argc;
		}
		
		if (strcmp(split_str, "&") == 0) {			// background 실행 명령어 확인
			background = 1;
		}
		
		my_argc++;									// argument count 1 증가
		split_str = strtok(NULL, " "); 			    // 다음 문자 split
		
	}
}
	
void redirection(char* argv[]) {
	int fd;
	
	if (!argv[redirection_idx+1]) {						// redirection 뒤에 값이 없는 경우 종료
		printf("Cannot execute redirection command\n");
		return;
	}
	
	fd = open(argv[redirection_idx+1], O_RDWR|O_CREAT|O_EXCL, 0666);	// 파일 생성 (이미 존재하면 불가능하게)
	
	if (fd < 0) {
		printf("Cannot open %s file with errno %d\n", argv[redirection_idx+1], errno);
		exit(-1);
	}
	
	redirection_idx = -1;		// redirection idx -1로 하여 뒤에서 접근 못하게 설정
	
    dup2(fd, STDOUT_FILENO);	// file descripter에 작성
    close(fd);					// file descripter 종료
}
