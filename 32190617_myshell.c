/**
/myshell.c : implement my shell
/@author  : Kim Boseung
/@email   : kbskbs1102@gmail.com
/@version : 8.0.1453
/@date    : 2022.12.03
**/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <stdlib.h>
#define MAX 255

//날짜와 유저 정보, 작업위치를 출력하고 명령어와 인자를 입력받는 프롬프트
void *prompt(char cBuf[]){ 
	time_t rawtime;
	struct tm *timeinfo;
	char hBuf[MAX], uBuf[MAX], dBuf[MAX];
	char *now;
	void *ret;           //각종 변수들 선언

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	now = asctime(timeinfo);  //현재시간을 문자열 형식으로 변환

	now[strlen(now)-1] = 0; //마지막 \n을 널 문자로 바꿈

	gethostname(hBuf,MAX); //호스트이름을 가져와서 저장
	getlogin_r(uBuf, MAX); //유저 정보를 가져와서 저장
	getcwd(dBuf, MAX);     //현재 작업 위치를 가져와서 저장

	printf("[%s]%s@%s(%s)$ ", now, hBuf, uBuf, dBuf); //여러 정보를 출력

	ret = fgets(cBuf, MAX, stdin);  //명령어를 입력받아서 값을 변수에 저장

	if(cBuf[strlen(cBuf)-1]=='\n')  //문자열의 마지막 \n을 널문자로 변환
		cBuf[strlen(cBuf)-1]=0;

	if (strcmp(cBuf, "exit")==0){   //exit명령어를 입력받으면 프로그램 종료
		return 0;
	}

	return ret;
}

int main(){
	char cBuf[MAX];
	char *arg;
	pid_t pid;
	int status;        //각종 변수들 선언

	while(prompt(cBuf)){  //명령어를 성공적으로 입력받는 동안 반복
		if((pid = fork())<0){    //pid는 0이상이어야한다
			perror("fork error");
		}
		else if(pid==0){        //자식 프로세스일 경우
			strtok(cBuf, " ");       //" "를 기준으로 cBuf의 내용을 토큰화한다
			arg = strtok(NULL," ");  //strtok 함수를 이용해 남은 문자 토큰화
		
			if(arg ==NULL){ //인자가 없다면 
				execlp(cBuf, cBuf, (char*)0); 
			}
			else{ //인자가 있다면 
				if(strcmp(cBuf, "cd") == 0){ //명령어가 cd라면 디렉토리 이름이 arg인 곳으로 이동
					chdir(arg);
					_exit(0);
				}
				else{   //그 의외의 경우라면 
					execlp(cBuf, cBuf, arg, (char*) 0);
				}
			}
			perror("couldn't execute"); 
		}
		waitpid(pid, &status, 0);//프로세스가 끝날때까지 기다린다
	}

	exit(0);
}
