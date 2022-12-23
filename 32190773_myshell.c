/**
* myshell.c      :  make a shell program
* @author        :  Seunggyeom Kim
* @email         :  32190773@danook.argc.kr
* @version       :  1.0
* @date          :  2022.12.07	
**/
#include <stdio.h>	
#include <unistd.h>	
#include <string.h> 
#include <sys/wait.h> 
#include <stdlib.h> 

#define MAX 255

void *prompt(char cBuf[]){
	void *ret;

	printf("myshell > ");
	ret = fgets(cBuf, MAX, stdin);

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

	while(prompt(cBuf)){	// 문자열을 prompt에 입력했을 때
		if((pid=fork())<0){
			perror("fork error");
		}
		else if(pid ==0){
			// 명령어 공백기준 분할
			arg[0] = strtok(cBuf," ");
			// 자른 문자 다음부터 구분자 또 찾기
			arg[1] = strtok(NULL, " ");
		
			if(arg[1] == NULL){					//	argment가 null값일 때
				if(execvp(arg[0], arg) < 0){
					printf("%s: command not found\n",arg[0]);
					exit(0);
				}
			}
			else if(*arg[1] != '|'){			// argment가 하나 있을 때
				arg[2] = 0;
				// cd command
				if(strcmp(arg[0], "cd")==0){
					chdir(arg[1]);
				}
				else{	// another command
					if(execvp(arg[0], arg) < 0){
						printf("%s: command not found\n", arg[0]);
						exit(0);
					}
				}
					
			}
			else{							
				arg[2] = strtok(NULL, " ");
				arg[3] = 0;
				
				pipe(fd);
				
				if((pipe_p1=fork())==0){
					close(1);		// 표준 출력인 stdout 닫는다
					close(fd[0]);	// pipe read를 닫는다
					dup(fd[1]);		
					close(fd[1]);	
					
					arg_p[0] = arg[0]; arg_p[1] = 0;
					
					if(execvp(arg[0],arg_p) < 0){
						printf("%s: command not found\n", arg[0]);
						exit(0);
					}
				}
				else if(pipe_p1<0){
					perror("fork error");
				}

				if((pipe_p2=fork())==0){
					close(0);		
					close(fd[1]);	
					dup(fd[0]);		
					close(fd[0]);
					
					arg_p[0] = arg[2]; arg_p[1] = 0;
					
					if(execvp(arg[2],arg_p) < 0){
						printf("%s: command not found\n", arg[0]);
						exit(0);
					}
					
					perror("child2 execvp failed");
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
