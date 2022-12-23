/**
 * allinOne.c	: file that includes all files relate with myShell program at once
 * @author	: Lee Heon (32207734)
 * @email	: knife967@gmail,com
 * @version	: 1.0
 * @date	: 2022.11.21 ~ 2022.12.09
**/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>

#define MAX 255

void *displayPrompt(char charBuffer[]) {
	char hostNameBuffer[MAX], userNameBuffer[MAX], dirNameBuffer[MAX];
	void *line;
	
	// user@host:directories# 출력을 위해 해당 값 얻어오기
	gethostname(hostNameBuffer, MAX);
	getlogin_r(userNameBuffer, MAX);
	getcwd(dirNameBuffer, MAX);
	
	// 색상 설정 (십의 자리 3 -> 9 사용 시, 기존 터미널과 동일 색상으로 출력 가능)
	printf("\x1b[36m[myShell]");
	printf("\x1b[32m%s@%s", userNameBuffer, hostNameBuffer);
	printf("\x1b[37m:");
	printf("\x1b[34m@%s", dirNameBuffer);
	printf("\x1b[37m# ");

	// 명령어 라인 받아오기
	line = fgets(charBuffer, MAX, stdin);

	// '\n'를 null character값으로 치환함으로써 엔터 입력 시 String으로 인식되도록 전환
	if (charBuffer[strlen(charBuffer) - 1] == '\n') {
		charBuffer[strlen(charBuffer) - 1] = '\0';
	}

	return line;
}

int getDataNum(char charBuffer[]) {
	int i, dataNum = 0;
	int len = strlen(charBuffer);
	
	//	명령어의 개수 찾기
	for (i = 0; i < len; i++) {
		if (charBuffer[i] == ' ') {
			dataNum++;
		}
	}
	
	dataNum++; 	// (단어(명령어)의 개수) = (공백의 개수) + 1

	return dataNum;
}

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

int main() {
	char charBuffer[MAX];
	char command[16];
	int i, dataNum, status;
	pid_t forkReturn;
	
	while (displayPrompt(charBuffer)) {
		int redirectionFlag = 0;			// Redirection 플래그 초기화
		int pipeFlag = 0;					// pipe 플래그 초기화
		char *arg[16] = {0};				// arg 포인터 배열을 NULL값으로  초기화
		
		dataNum = getDataNum(charBuffer);
		
		// 띄어쓰기 기준으로 라인 내 명령어를 분할하여 저장
		for (i = 0; i < dataNum; i++) {
			if (i == 0) {arg[i] = strtok(charBuffer, " ");}
			else {arg[i] = strtok(NULL, " ");}
			
			// 리다이렉션 확인
			if (   (strcmp(arg[i], ">") == 0)  \
				|| (strcmp(arg[i], ">>") == 0) \
			    || (strcmp(arg[i], "<") == 0)) {redirectionFlag = i;}
			
			// 파이프 확인
			if (strcmp(arg[i], "|") == 0) {pipeFlag = i;}
		}
		
		// 상황에 따라 프로세스 실행
		if (redirectionFlag != 0) {				// 리다이렉션 명령어가 있을 때
			forkReturn = myRedirection(arg[0], arg[1], arg[2], arg[3], redirectionFlag);
			waitpid(forkReturn, &status, 0);
		}
		else if (pipeFlag != 0) {				// 파이프 명령어가 있을 때
			forkReturn = myPipe(arg[0], arg[1], arg[2], arg[3], pipeFlag);
			waitpid(forkReturn, &status, 0);
		}
		else {
			// 백그라운드 프로세싱 구현
			if (strchr(arg[0], '&') != NULL) {	// 백그라운드 프로세싱 기호가 명령어에 붙어있다면
				printf("==========Background Processing Start==========\n");
				
				for (i = 0; i < (strlen(arg[0]) - 1); i++) {	// '&' 기호 삭제 후 명령어만 따로 저장
					command[i] = arg[0][i];
				}
	
				// fork() 및 exec()
				if ((forkReturn = fork()) < 0) {
					perror("fork error");
				}
				else if (forkReturn == 0) {
					execWithErrorCheck(command, NULL, NULL, NULL);
				}
				wait(NULL);										// wait을 안함으로써 백그라운드 프로세싱 구현
				printf("[Process ID: %d]\n", forkReturn);
				printf("==========Background Processing End============\n");
				
			}
			else {							// 그 외 일반적인 명령어 사용 시
				forkReturn = forkNexec(arg[0], arg[1], arg[2], arg[3], dataNum);
				waitpid(forkReturn, &status, 0);
			}
		}
	}
}
