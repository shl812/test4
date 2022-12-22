
/**
program = myshell
date = 2022-12-09
email = yjwiop227@naver.com
author = Jae Won Yu
**/

#include <stdio.h>	// printf
#include <unistd.h>	// fork, chdir
#include <string.h> // perror, strlen
#include <sys/wait.h> // waitpid
#include <stdlib.h> // exit
#define MAX 255

void *prompt(char cBuf[]){//프롬프트의 구성
	void *ret;//fgets 함수의 반환값 저장하는 곳
	printf("myshell > ");//myshell을 실행시 나타날 프롬프트의 모양
	ret = fgets(cBuf, MAX, stdin);/**fgets() 함수는 char *fgets (char *string, int n, FILE *stream); 이런 모양으로 현재 stream 위치에서 
    어느 것이 먼저 오건 첫 번째 줄 바꾸기 문자(\n)까지, 스트림의 끝까지 또는 읽은 문자 수가 n-1과 같을 때까지 문자를 읽습니다. 
    fgets() 함수는 결과를 string에 저장하고 스트링 끝에 널(null) 문자(\0)를 추가합니다. string은 줄 바꾸기 문자를 포함합니다(읽은 경우). n이 1이면 string이 비어 있습니다.
    **/
	if(cBuf[strlen(cBuf)-1] == '\n')
		cBuf[strlen(cBuf)-1] = 0;
	return ret;
}

int main()
{
	char cBuf[MAX];
	char *arg[4];
	char *arg_p[2];
	int pid, pipe_p1, pipe_p2;
	int status;
	int fd[2];

	while(prompt(cBuf)){	// 성공적인 입력 시, while로 돌입, while을 쓰는 이유는 입력은 한 번 하고 끝나는 게 아니기 때문이다.
		if((pid=fork())<0){//fork()가 0보다 작다는 건 fork error로 2개로 분리되지 않았다는 뜻이다.
			perror("fork error");
		}
		else if(pid ==0){	// 자식일때를 의미, pid가 0이면 자식 프로세스이므로
			// 명령어를 공백기준으로 분할
			arg[0] = strtok(cBuf," ");
			// 자른 문자 다음부터 구분자를 또 찾음
			arg[1] = strtok(NULL, " ");
		
			if(arg[1] == NULL){					//	(1) 인자가 없는 경우
				if(execvp(arg[0], arg) < 0){
					printf("%s: command not found\n",arg[0]);
					exit(0);
				}
			}
			else if(*arg[1] != '|'){			// (2) 인자가 하나인 경우를 의미
				arg[2] = 0;
				// change directory 명령어
				if(strcmp(arg[0], "cd")==0){
					chdir(arg[1]);
				}
				else{	// 또다른 명령어 일떄
					if(execvp(arg[0], arg) < 0){
						printf("%s: command not found\n", arg[0]);
						exit(0);
					}
				}
					
			}
			else{								// (3) 파이프
				arg[2] = strtok(NULL, " ");
				arg[3] = 0;
				
				pipe(fd);
				
				if((pipe_p1=fork())==0){
					close(1);		// 표준 출력 종료
					close(fd[0]);	// 파이프 읽기 종료
					dup(fd[1]);		// 표준출력으로 루트 바꾸기
					close(fd[1]);	//fd1 종료
					
					arg_p[0] = arg[0]; arg_p[1] = 0;
					
					if(execvp(arg[0],arg_p) < 0){
						printf("%s: command not found\n", arg[0]);
						exit(0);
					}
				}
				else if(pipe_p1<0){
					perror("포크 에러");
				}

				if((pipe_p2=fork())==0){
					close(0);		// stdin 종료
					close(fd[1]);	// 파이프 쓰기 종료
					dup(fd[0]);		// 리다이렉션
					close(fd[0]);  //fd0 종료
					
					arg_p[0] = arg[2]; arg_p[1] = 0;
					
					if(execvp(arg[2],arg_p) < 0){
						printf("%s: command not found\n", arg[0]);
						exit(0);
					}
					
					perror("자식 실행 실패");
					exit(1);
				}
				else if(pipe_p2<0){
					perror("fork error");
				}
				close(fd[1]);
				close(fd[0]);
				wait(0);
				wait(0);
			}
		}
		waitpid(pid, &status, 0);		
	}
	return 0;
}