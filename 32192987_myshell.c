/* 시스템프로그래밍 4차 과제
name : 이관현 
Student ID : 32192987 
date : 2022-12-08  */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>   
#include <fcntl.h>  
#include <unistd.h> // dup2 함수 사용을 위한 라이브러리
#include <stdbool.h> // bool 사용을 위한 라이브러리
#include <string.h> // str관련 함수 사용을 위한 라이브러리

#define MAX_SIZE 1024 // 1024크기를 지정하는 MAX_SIZE

void cmd_help(); // 쉘 명령어를 설명해주는 함수
int tokenize(char *buf, char *delims, char *tokens[], int maxTokens); // 입력받은 값들을 구분해주는 함수
void change_Dir(char *Dirname[]); // 현재 작업 디렉토리를 변경해주는 함수
bool run(char *line); // 명령어를 실행하는 함수

int main() {
    char line[MAX_SIZE];
    while (1) {
        printf("%s$ ", getcwd(line, MAX_SIZE));  //getcwd함수를 사용해서 현재 디렉토리를 알려주고 실패하면 -1 반환
        fgets(line, sizeof(line) - 1, stdin); // 성공하면 string 버퍼를 가리키는 포인터를 리턴한다.
        if (run(line) == false) break; //run 함수에서 리턴 받은 값이 false면 종료
    }
    return 0;
}

void cmd_help() { // 쉘 명령어를 설명해주는 함수
    printf("help : help를 다시 보여준다.\n");
    printf("exit : 쉘을 종료.\n");
    printf("> : 리다이렉션\n");
    printf("& : background processing\n");
}

int tokenize(char *buf, char *delims, char *tokens[], int maxTokens) {
	int token_Count = 0;
	char *token;
	token = strtok(buf, delims); // strtok 함수를 사용해서 delims을 기준으로 buf값을 문자열 자르기를 한다.
	while (token!=NULL && token_Count<maxTokens) { // 토큰을 끝까지 읽기 전까지
		tokens[token_Count] = token; //token을 각 인덱스에 저장
		token = strtok(NULL, delims); // 명령어를 분리시킴
		token_Count=token_Count+1; // token_Count값을 하나 증가한다.
	}
	tokens[token_Count] = '\0'; // 마지막 인덱스 널 문자 추가
	return token_Count;
}

void change_Dir(char *Dirname[]) {
	char *path; // 경로 변수
	char buf[MAX_SIZE]; // 임시 저장 변수
	
	path = getcwd(buf, MAX_SIZE); //getcwd함수를 사용해서 현재 디렉토리를 알려주고 실패하면 -1 반환
	if (path != NULL) { // 경로가 비어있지 않는다면
		path = strcat(path, "/"); // path 뒤에 /를 붙인다.
		path = strcat(path, Dirname[1]); // path 뒤에 목적 경로를 붙인다. 
		chdir(path); //chdir 함수를 사용해서 현재 작업 디렉토리를 변경.
	}
}

bool run(char *line) {
	char *cmd[MAX_SIZE]; // 명령어를 담을 버퍼 배열
	int i, index = 0;
	int fork_result; // fork값을 조정할 변수
	// 해당 작업을 할 지 지정하는 플래그 변수 선언 
	bool bd_flag = false; // 백그라운드
	bool redir_flag = false; // 리다이렉션
	// 리다이렉션 위치를 변경하기 위한 변수
	int redirIndex = -1;

	int t_Cnt = tokenize(line, " \n", cmd, (sizeof(cmd) / sizeof(char *))); // tokenize함수를 호출한 값 저장 t_Cnt 

	if (t_Cnt == 0) { return true; }
	else if (strcmp(cmd[0], "help") == 0) { // help를 입력하면 cmd_help함수 실행
		cmd_help();
		return true;
	}
	else if (strcmp(cmd[0], "cd") == 0 && t_Cnt == 2) { // cd를 입력하면 change_Dir함수 실행
		change_Dir(cmd);
		return true;
	}
	else if (strcmp(cmd[0], "exit") == 0) { return false; } // exit를 실행하면 false 반환
	
	for (i = 0; i<t_Cnt; i++) {
		if (strcmp(cmd[i], "&") == 0) {
			bd_flag = true; // 백그라운드 값 true
			cmd[i] = NULL; // &의 위치를 NULL로 바꿈
		}
		else if (strcmp(cmd[i], ">") == 0) {
			redir_flag = true; // 리다이렉션 값 true
			redirIndex = i + 1; // > 다음에 argument로 오프셋 설정
			cmd[i] = NULL; // >의 위치 NULL로 바꿈
		}
	}

	if ((fork_result = fork()) < 0) {
		perror("fork 에러"); // perror함수를 통해 오류 메시지를 출력한다.
		printf("쉘을 종료합니다.\n");
		exit(1);
	}
	
	else if (fork_result == 0) {
		if (redir_flag == true) {	
			int fd = open(cmd[redirIndex], O_RDWR | O_CREAT, 0664); // 파일 디스크립터에 지정된 파일 이름을 받아 파일을 생성
			dup2(fd, 1); // 앞 있는 값을 뒤에 있는 값에 연결(모든 출력)
			close(fd); // fd 연결 종료
		}
		execvp(cmd[0], cmd); // PATH에 등록된 디렉토리에 있는 프로그램을 실행
		exit(1);
	}
	
	else { // 백그라운드 값이 false면 부모 프로세스 wait() 실행
		if (bd_flag == false) { wait(); }
	}
	return true;
}