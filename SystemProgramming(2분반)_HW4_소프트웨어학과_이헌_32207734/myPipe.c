/**
 * myPipe.c		: file that include fork and execute function which relate to Pipe
 * myPipe()		: function that implements Pipe
 * @author		: Lee Heon (32207734)
 * @email		: knife967@gmail,com
 * @version		: 1.0
 * @date		: 2022.11.21 ~ 2022.12.09
**/

#include "myshell.h"


pid_t myPipe(char *arg1, char *arg2, char *arg3, char *arg4, int operatorIndex) {
	pid_t forkReturn, pipeForkReturn;
	int fd[2];
		
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
			pipe(fd);
			
			if ((pipeForkReturn = fork()) > 0) {	// 부모 프로세스일 경우
				wait(0);
				dup2(fd[0], 0);
				close(fd[1]);
				close(fd[0]);

				if (operatorIndex == 1) {			// '|' 연산자가 2번째 위치에 있을 때
					execWithErrorCheck(arg3, arg4, NULL, NULL);
				} else {							// '|' 연산자가 3번째 위치에 있을 때
					execWithErrorCheck(arg4, NULL, NULL, NULL);
				}
				

			}
			else if (pipeForkReturn == 0) {			// 자식 프로세스일 경우
				dup2(fd[1], 1);
				close(fd[1]);
				close(fd[0]);
				
				if (operatorIndex == 1) {			// '|' 연산자가 2번째 위치에 있을 때
					execWithErrorCheck(arg1, NULL, NULL, NULL);	
				} else {							// '|' 연산자가 3번째 위치에 있을 때
					execWithErrorCheck(arg1, arg2, NULL, NULL);
				}
				
			}
		}
	}
	
	return forkReturn;
}
