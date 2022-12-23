/**
 * myRedirection.c		: file that include fork and execute function which relate to Redirection
 * redirectControl()	: function that differently execute according to ioFlag
 * myRedirection()		: function that implements Redirection
 * @author		: Lee Heon (32207734)
 * @email		: knife967@gmail,com
 * @version		: 1.0
 * @date		: 2022.11.21 ~ 2022.12.09
**/

#include "myshell.h"


void redirectControl(int fd, char *argIn, char *argOut, char *argOption, int ioFlag) {
	if (fd < 0) {
		printf("Can't open %s file with errno %d\n", argOut, errno);
		exit(1);
	}
	if (ioFlag == 0) {
		dup2(fd, STDOUT_FILENO); 	// ioFlag = 0
	} else {
		dup2(fd, STDIN_FILENO);		// ioFlag = 1
	}
	
	close(fd);

	if (execlp(argIn, argIn, argOption, NULL) < 0) {
		printf("%s: Command Not Found\n", argIn);
		exit(1);
	}
}

pid_t myRedirection(char *arg1, char *arg2, char *arg3, char *arg4, int operatorIndex) {
	pid_t forkReturn;
	int fd;
	
	if ((forkReturn = fork()) < 0) {
		perror("fork error");
	}
	else if (forkReturn == 0) {
		if (strcmp(arg1, "exit") == 0) {
			sleep(1);
			printf("exit\n");
			kill(forkReturn, SIGINT);
		} 
		else if (strcmp(arg1, "cd") == 0) {
			chdir(arg2);
		}
		else {
			if (operatorIndex == 1) {			// 리다이렉션 연산자가 2번째 위치에 있을 때
				if (strcmp(arg2, ">") == 0) {
					fd = open(arg3, O_WRONLY | O_CREAT | O_TRUNC, 0644);
					redirectControl(fd, arg1, arg3, NULL, 0);
				}
				else if (strcmp(arg2, ">>") == 0) {	// '>'와 '>>'의 차이는 O_APPEND 플래그의 유무
					fd = open(arg3, O_WRONLY | O_CREAT | O_APPEND, 0644);
					redirectControl(fd, arg1, arg3, NULL, 0);
				}
				else if (strcmp(arg2, "<") == 0) {
					fd = open(arg3, O_RDONLY, 0644);
					redirectControl(fd, arg1, arg3, arg4, 1);
				}
			}
			else if (operatorIndex == 2) {		// 리다이렉션 연산자가 3번째 위치에 있을 때
				if (strcmp(arg3, ">") == 0) {
					fd = open(arg4, O_WRONLY | O_CREAT | O_TRUNC, 0644);
					redirectControl(fd, arg1, arg4, arg2, 0);
				}
				else if (strcmp(arg3, ">>") == 0) {
					fd = open(arg4, O_WRONLY | O_CREAT | O_APPEND, 0644);
					redirectControl(fd, arg1, arg4, arg2, 0);
				}
				else if (strcmp(arg3, "<") == 0) {
					fd = open(arg3, O_RDONLY, 0644);
					redirectControl(fd, arg1, arg4, arg2, 1);
				}
			}
		}
	}

	return forkReturn;
}
