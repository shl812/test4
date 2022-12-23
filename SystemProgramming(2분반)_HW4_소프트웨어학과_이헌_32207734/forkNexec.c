/**
 * forkNexec.c			: file that include Basic fork and execution
 * execWithErrorCheck() : function that execute with error check
 * forkNexec()			: function that fork() and execlp() with Error Check
 * @author		: Lee Heon (32207734)
 * @email		: knife967@gmail,com
 * @version		: 1.0
 * @date		: 2022.11.21 ~ 2022.12.09
**/

#include "myshell.h"


void execWithErrorCheck(char *arg1, char *arg2, char *arg3, char *arg4) {
	if (execlp(arg1, arg1, arg2, arg3, arg4, NULL) < 0) {
		printf("%s: Command Not Found\n", arg1);
		exit(1);
	}
}

pid_t forkNexec(char *arg1, char *arg2, char *arg3, char *arg4, int dataNum) {
	pid_t forkReturn;
	
	if ((forkReturn = fork()) < 0) {
		perror("fork error");
	}
	else if (forkReturn == 0) {
		if (strcmp(arg1, "exit") == 0) {		// 종료 기능 구현
			sleep(1);							// 기존 쉘과 동일하게 텀을 두고 exit
			printf("exit\n");
			kill(forkReturn, SIGINT);
		} 
		else if (strcmp(arg1, "cd") == 0) {		// cd 명령어라면
			chdir(arg2);						// 자식 디렉토리에 추가
		}
		else {
			execWithErrorCheck(arg1, arg2, arg3, arg4);
		}
	}

	return forkReturn;
}
