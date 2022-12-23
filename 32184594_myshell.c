/** 
  * makeShell.c
  *@author: Seongjin Choi
  *@email : chltjdwls294@naver.com
  *version = 1.0
  *date = 22.12.09
 **/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#define MAX 100

int main(int argc, char **argv)
{
        char buffer[MAX]; 
        pid_t pid;
        printf("나만의 쉘 시작(exit 입력시 종료)");
        while(1)
        {
                memset(buffer, 0x00, MAX); 
                fgets(buffer, MAX - 1, stdin);
                

        
                if(strncmp(buffer, "exit\n", 5) == 0)
				{
                        break;
                }
                buffer[strlen(buffer) - 1] = 0x00; 

                pid = fork();
                if(pid == 0)
				{ 
                        if(execlp(buffer, buffer, NULL) == -1)
						{
                                        printf("Failed \n");
                                        exit(0);
                        }
                }
                if(pid > 0)
				{ 
                        wait(NULL);
                }
        }
        return 0;
}
