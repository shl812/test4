/**
* main.c : shell program
* @author : Hyunwoo Kim
* @email : khwkhm1111@naver.com
* date : 2022.12.06
**/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#define MAXLINE 32

int main(int argc, char **argv){
	char buf[MAXLINE];
	pid_t pid;

	printf("KIM's Shell\n");

	while(1){
		
		memset(buf, 0x00, MAXLINE);
		printf(">>");
		fgets(buf, MAXLINE - 1, stdin);
		if(strncmp(buf, "exit\n", 5) ==0){
			break;
		}

		buf[strlen(buf) - 1] = 0x00;

		pid = fork();
		if(pid < 0){
			perror("fork error");
			exit(1);
		}
		else if(pid == 0){
			if(execlp(buf, buf, NULL) == -1){
				printf("명령어 실행 실패\n");
				exit(0);
			}
		}
		else if(pid > 0){
			wait(NULL);
		}
	}
	return 0;
}
