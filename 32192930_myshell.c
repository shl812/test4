//32192930
//소프트웨어학과
//윤태현

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <wait.h>
#include <fcntl.h>

#define MAX 64

int main()
{
    pid_t fork_return,d_pid;
	char Buf[MAX];                        
    int process1, process2;
	int fd[2];
	int exit_stutus;
	char *argv[4];
	char *arg_p[2];
	
	while(1){
		memset(Buf,0x00,MAX);
		printf("myShell >"); //prompt
		fgets(Buf,MAX-1,stdin); //input
		
		if(Buf[strlen(Buf)-1]=='\n')
		   Buf[strlen(Buf)-1]=0; //줄바꿈 문자 없애기
		
		if(strcmp(Buf,"exit") == 0) //exit 명령 수행
			exit(0); //
		
		if((fork_return=fork())<0){ //fork()를 통해 자식 프로세스 시도 후 실패하면 에러 출력
			perror("fork error");
			exit(0);
		}
		else if(fork_return==0){ //자식프로세스 생성에 성공했을 때
			argv[0]=strtok(Buf," "); //parsing
			argv[1]=strtok(NULL," "); //다음 token 찾기
			argv[2]=strtok(NULL," ");
			if(argv[1]==NULL){ //인자가 없을 때
				if(execvp(argv[0],argv)<0){ //argv[0]을 다른 프로그램에서 실행에 실패했을 때
					printf("%s: command not found case1\n",argv[0]); 
					exit(0);
				}
			}
			
			else if(*argv[1] != '|'){ //파이프를 사용하지 않는 명령에 대한 것
				
				if(strcmp(argv[0],"cd")==0){
					chdir(argv[1]); //change directory
					continue; //다시 처음으로
				}
				else if (strcmp(argv[1],">")==0){ //redirection
					argv[1]=NULL; //">" 문자를 NULL로 바꾸기
					fd[0]=open(argv[2], O_CREAT | O_RDWR | O_TRUNC, 0664); //"argv[2] 파일을 열고 쓰기
					if(fd[0]<0) //open에 실패했을 때 출력
						printf("error\n");
  					dup2(fd[0], 1); //dup2를 활용해 fd[0]을 stdout에 덮어쓰기
  					close(fd[0]); //fd[0] close
					
					execvp(argv[0],argv); //argv[0]을 다른 프로그램에서 실행
					exit(0);
				}
				
				else{
					if(execvp(argv[0],argv)<0){ //이외에 다른프로그램에서 실행에 실패했을 때 오류 출력
						printf("%s: command not found case2\n",argv[0]);
						exit(0);
					}
				}
			}
			else{ //pipe가 필요할 때
				argv[2]=strtok(NULL," "); 
				
				
				pipe(fd); //pipe 활용
				if((process1=fork())==0){ //process1이 자식 프로세스라면
					close(1);		// stdout 닫기
					close(fd[0]);	// fd[0] 닫기
					dup(fd[1]);		// 출력되는 fd[0]에 입력하는 fd[1] 덮어쓰기
					close(fd[1]);	// fd[1] 닫기
					
					arg_p[0] = argv[0]; arg_p[1] = NULL; 
					
					if(execvp(argv[0],arg_p) < 0){ //argv[0] 명령실패하면
						printf("%s: command not found case3\n", argv[0]);
						exit(0);
					}
				}
				else if(process1<0){ //fork에 실패한다면
					perror("fork error");
				}

				if((process2=fork())==0){ //process2가 자식프로세스라면
					close(0);		// 위 과정과 반대로 수행한다.
					close(fd[1]);	
					dup(fd[0]);	
					close(fd[0]);   
					
					arg_p[0] = argv[2]; arg_p[1] = NULL;
					
					if(execvp(argv[2],arg_p) < 0){ //argv[2] 명령실패하면
						printf("%s: command not found case4\n", argv[0]);
						exit(0);
					}
					
					perror("child2 fail"); //오류뜨면 출력
					exit(0);
				}
				else if(process2<0){ 
					perror("fork error");
				}
				close(fd[1]); //다 수행하면 닫기
				close(fd[0]); //process1과 process2에서의 자식프로세스는 종료되지 않았고 백그라운드 프로세싱 중
			}
		}
		d_pid=wait(&exit_stutus); //다음 prompt를 위한 자식프로세스 종료를 기다림
		WEXITSTATUS(d_pid); //자식 프로세스가 잘 종료되었는지 확인
	}
	return 0;
}

