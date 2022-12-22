/*
 * 학과 : 소프트웨어학과
 * 학번 : 32192406
 * 이름 : 심의진
 * 작성 : 2022.12.09
 */

#define _GNU_SOURCE
#include <stdio.h>	// 표준 입출력 라이브러리
#include <string.h> // 문자열 관련 라이브러리

#include <unistd.h>	// 파일 제어 관련 라이브러리
#include <fcntl.h>	// 파일 제어 관련 라이브러리
#include <errno.h>	
#include <stdbool.h>	// 불리언 자료형 라이브러리


int tokenize(char* str, char* argv[]) {	// 문자열 파싱 함수.
	int cnt = 0;	// 토큰의 수
	char* token = strtok(str, " \n");	

	while(token != NULL) {
		argv[cnt] = token;
		cnt++;	
		token = strtok(NULL, " \n");
        
	}
	argv[cnt] = NULL;
	return cnt;
}	// strtok()을 이용하여 " " 혹은 "\n" 문자를 기준으로 문자열을 파싱 후 토큰의 개수를 반환한다.

bool run(char* line) {	// myshell 실행 함수. 비정상 작동하거나 종료할 경우 false를 반환하고 myshell을 종료한다.

	int fd;
    pid_t fork_return;	// myshell 작동을 위한 자식 프로세스 할당 변수
    
	int RD_ptr = 0;	//Redirection 구현을 위한 변수 (파일 포인터 용도)
	bool RD_check = false;	//Redirection 구현을 위한 변수 (flag 용도)
	int token_cnt;
	char* tokens[255];
	
	token_cnt = tokenize(line, tokens);
	
	for(int i = 0; i < token_cnt; i++) {
		if(strcmp(tokens[i],">") == 0) {	// Redirection 명령어 입력 확인
			RD_ptr = i;
			RD_check = true;
			break;
		}
        if(strcmp(tokens[i], "exit") == 0){	// myshell 종료 명령어 입력 확인
            return false;
        }
	}	

	if((fork_return = fork()) == -1) {
		printf("fork error\n");	// fork 오류 메시지
		return false;
	}
    else if(fork_return == 0) {
	    if(RD_check == true) {
			fd = open(tokens[RD_ptr + 1], O_RDWR|O_CREAT, 0664);
				// Redirection 명령어 토큰 뒤의 토큰 포인터를 파일명으로 인식하토록
            if(fd < 0){
                printf("Can't open %s file with errno %d", tokens[RD_ptr + 1], errno);
            }	// 파일을 여는데 실패했을 경우 오류 메시지
			close(STDOUT_FILENO);
			dup2(fd, STDOUT_FILENO);
			tokens[RD_ptr] = NULL;	// Redirection 명령어 토큰 삭제
		}
		execvp(tokens[0], tokens);	// 명령어 토큰을 실행하는 실행함수
		printf("execution error\n");	// 명령어 토큰의 비정상 작동 오류 메시지
		return false;
	}

	return true;
}	

int main() {
	char line[1024];
	
	while(1) {
		printf("%s# ", get_current_dir_name());	// myshell 명령어 입력창
		fgets(line, sizeof(line) - 1, stdin);	// 파일의 문자열을 읽어오는 함수. 사용자의 입력으로부터 run()이 실행할 명령어를 전달.
		if(run(line) == false)
			break;	//	myshell이 종료되는 경우(run()이 false를 반환하는 경우) 무한 loop를 종료하고 main함수또한 종료.
	}
	
	return 0;
}