// 32211058 김준명 - 쉘 프로그래밍
// 2022-12-03
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

// 버퍼의 사이즈
#define BSIZE 100

int main()
{
	printf("Made by 32211058 김준명 - DKU\n");
	// 입력받은 명령어를 담을 배열
	char command[BSIZE];

	while(1) {
		// 커맨드 배열 초기화
		command[0] = '\0';
		// 프롬프트 출력
		printf("$ ");
		// 명령어 읽기
		fgets(command, BSIZE - 1, stdin);

		// quit을 입력하면 종료
		if (strncmp(command, "quit\n", 5) == 0) {
			printf("bye...\n");
			break;
		}

		// 명령어 문자열의 마지막 문자 \n을 제거
		command[strlen(command) - 1] = '\0';

		pid_t pid = fork();
		if (pid == 0) {
			if (execlp(command, command, NULL) == -1) {
				printf("Command failed.\n");
				exit(0);
			}
		}
		if (pid > 0)
			wait(NULL);
	}

	return 0;
}
