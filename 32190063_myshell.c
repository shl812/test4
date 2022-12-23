#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>

#define MAX 256
void *prompt(char cBuf[]){ //프롬프트 창
	void *ret;

	printf("mysh > "); //shell
	ret = fgets(cBuf, 256, stdin); //입력 받기

	if(cBuf[strlen(cBuf)-1] == '\n') //마지막 입력 == 엔터
		cBuf[strlen(cBuf)-1] = 0; //엔터 제거
	return ret; //입력 받은 값 반환
}

int main(){
	char cBuf[MAX];	   		// 사용자 입력 문자열
	char Save[MAX]; 		// 임시저장용
	char *pipe_1 = NULL;	//첫 파이프 문
	char *pipe_2 = NULL;	//두번 째 파이프 문
	char *pipe_3 = NULL;   	//마지막 파이프 문
	char *redFirst = NULL;  //redirection 부등호 앞
	char *redSecond = NULL; //redirection 부등호 뒤
	char *cmd1[4]; 			//앞쪽 명령어
	char *cmd2[5]; 			//중간 명령어
	char *cmd3[5]; 			//마지막 명령어 
	
	int amper; 		// 백그라운드 사용 시 입력 인수에서 '&'를 삭제하기 위한 변수
	int i ;
	char *pch;		// 문장나눌때 임시 저장
	
	int fd1[2]; // 파이프 처리
	int fd2[2]; // 파이프 처리 (파이프가 두 개일 경우)
	int fd;
	pid_t pid;

	
	
	while(prompt(cBuf)){
		//----------------------  Background Part  ----------------------
		if(strchr(cBuf,'&')!=NULL) { 	//입력에 &가 포함되어 있는 경우
			
		    amper = -1;
		    cBuf[strlen(cBuf)] = '\0'; 	//
		    i=0;
		    pch = strtok(cBuf," ");		// 공백으로 구분

		    while(pch != NULL && i<3)
		    {
			    if(strcmp(pch,"&")==0)	// '&'를 찾으면 해당 idx를 저장
				    amper = i;
			    cmd1[i] = pch;          //cmd1의 배열에 명령어 저장
			    pch = strtok(NULL," "); //다음 공백까지
			    i++;
		    }
		    cmd1[i] = '\0';
		    cmd1[amper] = NULL;	// 사용자 입력 명령어문자열에서'&' 기호를 삭제

		    pid = fork(); 		//자식 프로세스 생성
		    if(cmd1[0]!=NULL) { //cmd1[0] = 명령어 본체 ex) cat, ls 등
				if(pid == 0) {  //정상적인 자식 프로세스라면
					execvp(cmd1[0],cmd1); //추가적인 설정 -> cmd1[0] 뒤에 붙어있는 -al 과 같은 부속 내용 = cmd1을 통해 해결
					exit(0);
				}
			wait(NULL); //부모는 자식을 기다림
		    }
		}



		//-------------------------- Pipe Part  --------------------------

		else if(strchr(cBuf,'|')!=NULL){ //명령어에 |가 있는 경우
			pipe_1 = strtok (cBuf,"|");  // '|' 기호를 기준으로 나눈다
			pipe_2 = strtok (NULL, "|");
			pipe_3 = strtok (NULL, " ");

			strcat(pipe_1,"\0");
			strcat(pipe_2, "\0");
			if(pipe_3 != NULL) 			// 마지막 세번째 명령어는 쓰이지 않을 경우도 있다
				strcat(pipe_3,"\0");
			
			i=0;//초기화
			pch = strtok (pipe_1," ");
			while (pch != NULL && i<3)		//첫 명령어 저장
			{
				cmd1[i]=pch;
				pch = strtok (NULL, " "); 	//공백으로 구분
				i++;
			}
			cmd1[i]='\0';
			
			i=0;//초기화
			pch = strtok (pipe_2," ");
			while (pch != NULL && i<3) 		//두 번째 명령어 저장
			{
				cmd2[i]=pch;
				pch = strtok (NULL, " "); 	//공백으로 구분
				i++;
			}
			cmd2[i]='\0';

			if(pipe_3 != NULL) { // 세 번째 명령어는 사용되지 않았을 수도 있으니 확인
				i=0;
				pch = strtok(pipe_3," ");
				while(pch != NULL && i<3) 	//세 번째 명령어 저장
				{
					cmd3[i]=pch;
					pch = strtok(NULL," "); //공백으로 구분
					i++;
				}
				cmd3[i]='\0';
			}

			if(pipe(fd1) == -1){	    // 파이프 생성 fd1[0] 읽기용, fd1[1] 쓰기용
				printf("fail to call pipe1\n");
				exit(1);
			}
			if(pipe_3 != NULL) {	    //파이프가 두 개일 경우 fd2[0] 읽기, fd2[1] 쓰기
				if(pipe(fd2) == -1) {
					printf("fail to call pipe2\n");
					exit(1);
				}
			}
			switch(fork())// pipe_1  명령어프로세스 생성
			{
				case -1 : 
					perror("fork error1"); 
					break;
				case 0 :
					  dup2(fd1[1], STDOUT_FILENO);  // 표준출력 파이프 연결
					 
					  close(fd1[0]);
					  close(fd2[0]);
					  close(fd2[1]);
					  close(fd1[1]);	  

					  execvp(cmd1[0], cmd1); //첫 명령어 실행
					  exit(0);  
			}
			switch(fork())// pipe_2 명령어 프로세스 생성
			{
				case -1 :
					perror ("fork error2");
					break;
				case 0 :
					  dup2(fd1[0], STDIN_FILENO);  //  표준입력 파이프 연결
					  dup2(fd2[1], STDOUT_FILENO);  //  표준출력 파이프 연결

					  close(fd1[1]);
					  close(fd2[0]);
					  close(fd1[0]);
					  close(fd2[1]);

					  execvp(cmd2[0], cmd2); //두 번째 명령어 실행
					  exit(0);  
			}
		    
			if(pipe_3 != NULL) { //세 번째 명령어가 있다면

			    switch(fork()) // pipe_3 명령어 프로세스 생성
			    {
			    	case -1 : 
						perror("fork error3"); 
						break;
			    	case 0 :
					  dup2(fd2[0],STDIN_FILENO);  // 표준입력 파이프 연결

					  close(fd2[1]);
					  close(fd1[0]);
					  close(fd1[1]);
					  close(fd2[0]); 

					  execvp(cmd3[0], cmd3); //세 번째 명령어 실행
					  exit(0);
			    }
			}   
			
			
			close(fd1[0]);
			close(fd1[1]);
			close(fd2[0]);
			close(fd2[1]);
				       	
			while(wait(NULL) != -1); //자식이 모두 끝날 때까지
		}
		
		//-------------------------  redirection part  -------------------------
		else if(strchr(cBuf,'>')!=NULL){	//부등호가 있다면 redirection으로 간주
			redFirst =strtok(cBuf,">");
			redSecond =strtok(NULL," ");
			
			strcat(cBuf, "\0");
			strcat(redFirst,"\0");
			strcat(redSecond, "\0");
			
			i=0;//초기화
			pch = strtok (cBuf," ");
			while (pch != NULL) //명령어 저장
			{
				cmd1[i]=pch;
				pch = strtok (NULL, " "); //공백으로 구분
				i++;
			}
			cmd1[i]='\0';
			
			switch (fork())
			{
				case -1 : 
					perror("fork");
					break;
				case 0:
					if (fd = open(redSecond, O_RDWR | O_CREAT, 0641) < 0)
						printf("Cannot open file...\n");
    				dup2(fd, STDOUT_FILENO);
					close(fd);
     				execvp(cmd1[0], cmd1);
     				exit(0);
			}
			wait(NULL);
		}
		
		else if(strstr(cBuf,">>")!=NULL){	//redirection 중 append형식
			redFirst =strtok(cBuf,">>");
			redSecond =strtok(NULL," ");
			
			strcat(cBuf, "\0");
			strcat(redFirst,"\0");
			strcat(redSecond, "\0");
			
			i=0;//초기화
			pch = strtok (cBuf," ");
			while (pch != NULL) //명령어 저장
			{
				cmd1[i]=pch;
				pch = strtok (NULL, " "); //공백으로 구분
				i++;
			}
			cmd1[i]='\0';
			
			switch (fork())
			{
				case -1 : 
					perror("fork");
					break;
				case 0:
					if (fd = open(redSecond, O_RDWR | O_CREAT | O_APPEND, 0641) < 0)
						printf("Cannot open file...\n");
    				dup2(fd, STDOUT_FILENO);
					close(fd);
     				execvp(cmd1[0], cmd1);
     				exit(0);
			}
			wait(NULL);
		}
		
		//단일 명령어
		else{ 
			cBuf[strlen(cBuf)]='\0';
			if(strcmp(cBuf,"quit") == 0) //quit 입력 시 mysh 종료
				exit(0);
			else 
				system(cBuf);
		    }   

	}
	return 0;
}