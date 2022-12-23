/**
 *  myshell.c : 명령어를 번역하는 쉘을 구현한 파일
 *  @author : kim tae su
 *  @student ID: 32191216
 *  @email :kim00566@naver.com
 *  @version : 1.0
 *  @date : 2022.12.04
 **/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>


int main(){
	char str[256];	    // 사용자 입력 문자열
	char * cmd1[4]; //앞쪽 명령어
	char * cmd2[5]; //중간 명령어
	char * cmd3[5]; //마지막 명령어 
	char * fn;
	
	int fd;
	int signIdx; // 백그라운드 사용 시 입력 인수에서 '&'를 삭제하기 위한 변수
	int i ;
	char *pch;//parsing할 때 임시 저장

	
	pid_t pid;

	while(1){

		printf("myshell# ");
		
		//프롬프트가 변수 초기화
		for(i=0 ; i < 256 ; i++){    
			str[i]='\0';
		}
		
		fgets(str,sizeof(str),stdin);
		str[strlen(str)-1] ='\0';
		
		//background
		if(strchr(str,'&')!=NULL) {
		    printf("------------------------------------------------------------------------------------------\n");

		    signIdx = -1;
		    str[strlen(str)] = '\0';
		    i=0;
		    pch = strtok(str," ");	// 공백을 기준으로 명령어를 나누는 parsing입니다
			printf("%c",pch);
		    while(pch != NULL && i<3)
		    {
			    if(strcmp(pch,"&")==0)	// '&'를 찾으면 해당 idx를 저장
				    signIdx = i;
			    cmd1[i] = pch;
			    pch = strtok(NULL," ");
			    i++;
		    }
		    cmd1[i] = '\0';
		    cmd1[signIdx] = NULL;	// 사용자 입력 명령어문자열에서'&' 기호를 삭제

		    pid = fork();
		    if(cmd1[0]!=NULL) {
			if(pid == 0) {
				execvp(cmd1[0],cmd1);
				exit(0);
			}
			wait(NULL);
		    }
		    printf("------------------------------------------------------------------------------------------\n");
		    //backgroud로 실행할 경우 부모의 pid 값을 출력한다.
		    printf("[Process id %d]\n",pid); 
		}
		
		
		//redirect
		else if(strstr(str, ">>") != NULL){
		    str[strlen(str)] = '\0';
		    i=0;
		    pch = strtok(str," ");	// 공백을 기준으로 명령어를 나누는 parsing입니다

		    while(pch != NULL && i<3)
		    {
			    if(strncmp(pch, ">>", 2)==0)	// '>>'를 찾으면 해당 idx를 저장
				    signIdx = i;			/
			    cmd1[i] = pch;
			    pch = strtok(NULL," ");
			    i++;
		    }
			fn = cmd1[2];
			cmd1[signIdx] = cmd1[2];
			cmd1[i-1] = NULL;
		    cmd1[i] = '\0'; 
			pid = fork();
			if (pid == 0) {
			fd = open (fn , O_CREAT | O_WRONLY | O_APPEND, 0644);
			if (fd < 0) {
				perror("error");
				exit(-1);
			}
			dup2(fd, STDOUT_FILENO);
			close(fd);
			
			cmd1[signIdx] = NULL;
			if(signIdx != 0){
				execvp(cmd1[0], cmd1);
				fprintf(stderr, "myshell : command not found\n");
				exit(127);
			}
			
			exit(0);
			
			}
			waitpid(pid, NULL, 0);
		}
		
		else if(strchr(str,'>') != NULL){
		    str[strlen(str)] = '\0';
		    i=0;
		    pch = strtok(str," ");	// 공백을 기준으로 명령어를 나누는 parsing입니다

		    while(pch != NULL && i<3)
		    {
			    if(strncmp(pch, ">" ,1)==0)	// '>'를 찾으면 해당 idx를 저장
				    signIdx = i;
			    cmd1[i] = pch;
			    pch = strtok(NULL," ");
			    i++;
		    }
			
			fn = cmd1[2];
			cmd1[signIdx] = cmd1[2];
			cmd1[i-1] = NULL;
		    cmd1[i] = '\0';
			
			pid = fork();
			if (pid == 0) {
			fd = open (fn , O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if (fd < 0) {
				perror("error");
				exit(-1);
			}
			dup2(fd, STDOUT_FILENO);
			close(fd);
			
			cmd1[signIdx] = NULL;
			if(signIdx != 0){
				execvp(cmd1[0], cmd1);
				fprintf(stderr, "myshell : command not found\n");
				exit(127);
			}
			
			exit(0);
			}
			waitpid(pid, NULL, 0);
			
		}
		
		//redirect in
		else if(strchr(str,'<') != NULL){
			str[strlen(str)] = '\0';
		    i=0;
		    pch = strtok(str," ");	// 공백을 기준으로 명령어를 나누는 parsing입니다

		    while(pch != NULL && i<3)
		    {
			    if(strcmp(pch, "<")==0)	// '<'를 찾으면 해당 idx를 저장
				    signIdx = i;
			    cmd1[i] = pch;
			    pch = strtok(NULL," "); 
			    i++;
		    }
			cmd1[signIdx] = cmd1[2];
			cmd1[i-1] = NULL;
		    cmd1[i] = '\0'; 
			
			pid = fork();
			if(pid == 0){
				fd = open(cmd1[signIdx], O_RDONLY);
				if(fd < 0){
					perror("error");
					exit(-1);
				}
				dup2(fd, 0);
				close(fd);
				
				cmd1[signIdx] = NULL;
				if (signIdx != 0){
					execvp(cmd1[0], cmd1);
					fprintf(stderr, "myshell: command not found\n ");
					exit(127);
				}
				exit(0);
			}
			waitpid(pid, NULL, 0);
		
		}
		else{// background, pipe가 아닌 일반 명령어인 경우 

			str[strlen(str)]='\0';
			if(strcmp(str,"exit") == 0) //사용자 입력이 "exit"일 경우 쉘을 종료함
				exit(0);
			else 
		    str[strlen(str)] = '\0';
		    i=0;
		    pch = strtok(str," ");	// 공백을 기준으로 명령어를 나누는 parsing입니다
		    while(pch != NULL && i<2)
		    {
			    cmd1[i] = pch;
			    pch = strtok(NULL," ");
			    i++;
		    }
		    cmd1[i] = '\0';
		    pid = fork();
		    if(cmd1[0]!=NULL) {
			if(pid == 0) {
				execvp(cmd1[0],cmd1);
				exit(0);
			}
			wait(NULL);
		    }	
		}
		

	}
	return 0;
}


