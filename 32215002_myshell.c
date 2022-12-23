/**
* 32215002shell.c
* @author	:	Yunha Hwang (32215002)
* @email	:	onlyunha@dankook.ac.kr
* @date	:	2022.12.09
**/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#define MAX_BUF 64


void cm(char* t) {
	if (chdir(t) == -1) {
		perror("cd");
	}
}



// 백그라운드 실행
void bgex(char* t[]) {
	pid_t pid;
	int f;
	if ((pid = fork()) == 0) {
		f = open("/dev/null", O_RDONLY);
		dup2(f, STDOUT_FILENO);
		execvp(t[0], t);
		exit(0);
	}
}



// 토큰 개수 세기
int tokenCount(char* buf, char* arr[]) {
	char* t;
	int count = 0;
	t = strtok(buf, " ");
	while (t != NULL) {
		arr[count++] = t;
		t = strtok(NULL, " ");
	}
	arr[count] = NULL;
	return count;
}



// redirection 구현
void redi(char* t[]) {
	pid_t pid;
	int f;

	if ((pid = fork()) == 0) {
		f = open(t[3], O_RDWR | O_CREAT | O_TRUNC, 0641);
		dup2(f, STDOUT_FILENO);
		close(f);
		execvp(t[0], t);
		exit(0);
	}
	wait();
}




// 실행
bool ex(char* c) {
	int count;
	int i = 0;
	int flag = 0;
	char* t[MAX_BUF];
	pid_t pid;

	// 토큰 개수 세기
	count = tokenCount(c, t);

	if (strcmp(t[0], "cd") == 0) {
		cm(t[1]);
		return;
	}

	for (i = 0; i < count; i++) {
		if (!strcmp(t[i], ">")) {
			flag++;
			redi(t);
		}

		// backgorund 실행일 경우
		if (!strcmp(t[i], "&")) {
			flag++;
			bgex(t);
		}
	}

	if (!flag) {
		if ((pid = fork()) == 0) {
			execvp(t[0], t);
			exit(0);
		}
		wait();
	}
	memset(t, '\0', MAX_BUF);
	return 1;
}




// 메인
int main() {
	char c[MAX_BUF];

	while (1) {
		printf("%s$ ", get_current_dir_name());
		fgets(c, sizeof(c) - 1, stdin);

		if (strncmp("exit", cm, 4) == 0) {
			printf("Shell end");
			break;
		}
		else ex(c);
	}
}