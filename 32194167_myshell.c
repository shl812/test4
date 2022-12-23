/*
report_1.c: shell basic logic, background processing, redirection example program
author: wonjun Jung
email: 32194167@dankook.ac.kr
version:1.2
date:2022.12.06
*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include<fcntl.h>
#include<errno.h>
#include<unistd.h>
//shell script 만들기
/*
1. parsing, fork(), execve() 기능 구현
2. 백그라운드로 돌아가는 거 구현
3. redirection 구현
*/
#define MAX 255

void * prompt(char Buf4[]) //입력값을 Buf4에 읽히고 이를 ret으로 반환함. 
{
  char Buf1[MAX], Buf2[MAX], Buf3[MAX];
  char * now;
  void * ret;
  gethostname(Buf1, MAX);
  getlogin_r(Buf2, MAX);
  getcwd(Buf3, MAX);
  printf("%s@%s:%s#", Buf1, Buf2, Buf3);
  ret = fgets(Buf4, MAX, stdin);
  if (Buf4[strlen(Buf4) - 1] == '\n') {
    Buf4[strlen(Buf4) - 1] = 0;
  }
  return ret;
}

int main() 
{
  char Buf4[MAX];
  char * arg;
    char *arg2;
  pid_t pid;
  int sta;
  while (prompt(Buf4)) 
  {
    if ((pid = fork()) < 0) //fork() 기능 o
    {
      perror("fork error");
    }
    	if (strcmp(Buf4, "exit") == 0) //exit 
    	{
      	exit(0);
    	} 
		else if (pid == 0) 
		{
    		strtok(Buf4, " "); //문자열을 띄어쓰기가 나올 때 까지 읽고 띄어쓰기 부분을 \0로 변경
    		arg = strtok(NULL, " "); //\0부터 다음 띄어쓰기까지 읽음. 모든 문자를 읽으면 종료
      		//printf("%s, %s\n", Buf4, arg); 읽은 문자 확인
      
			if (arg == NULL) 
			{
        		execlp(Buf4, Buf4, (char * ) 0); //ls와 같은 단일 명령어로 받을 경우
				printf("this is error\n");
      		}
			else
	  		{
        		if (strcmp(Buf4, "cd") == 0) //cd를 받을 경우 다음에 오는 arg로 이동
        		{
          			chdir(arg);
          			_exit(0);
        		} 
				else if (strcmp(arg, ">") == 0) // 꺽쇠일 경우 redirection
        		{
       				int fd, fd1, read_size, write_size;
        			char buf[MAX];
        			fd = open(Buf4, O_RDONLY); //파일 디스크립터를 열어 Buf4 읽음
      				arg2 = strtok(NULL, " "); //\0부터 다음 띄어쓰기까지 읽음. 모든 문자를 읽으면 종료
        			fd1 = open(arg2, O_RDWR | O_APPEND); // 파일 디스크립터를 열어 arg[2], 꺽쇠로 대상이 되는 파일을 읽음
        			dup2(fd1, STDOUT_FILENO); // fd1으로 복사하여 대상이 되는 파일에 붙여넣기 해줌
        			while (1) 
					{
            			read_size = read(fd, buf, MAX);
            			if (read_size == 0) 
						{
              				break;
            			}
            			write_size = write(STDOUT_FILENO, buf, read_size); //fd1을 씀
          			}
          			close(fd);	
        		} 
				else 
				{
          			execlp(Buf4, Buf4, arg, (char * ) 0); //ls -al 같은 경우, arg를 인자로 받음
					printf("this is error\n");
				}
      		}
    	}
	waitpid(pid, &sta, 0); //자식 프로세스 대기 
	//exit(0); //넣을 경우 종료되면 shell 나가짐. 넣지 않을 경우 exit 입력 전까지 반복
  }
}	