/**
 * main.c	: file which contains main function
 * @author	: Lee Heon (32207734)
 * @email	: knife967@gmail,com
 * @version	: 1.0
 * @date	: 2022.11.21 ~ 2022.12.09
**/

#include "myshell.h"


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
				
				for (i = 0; i < (int)(strlen(arg[0]) - 1); i++) {	// '&' 기호 삭제 후 명령어만 따로 저장
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
