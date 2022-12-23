//32139419 이재민

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>  
#include <errno.h>   
#include <fcntl.h>  

#define MAX_SIZE 256 //배열 버퍼의 최대 크기 지정

int main() {
	
    char cmd[MAX_SIZE]; //명령어를 받아 담을 버퍼 배열
	char path[MAX_SIZE]; //경로 위치를 담을 버퍼 배열
	
    while (1){
		
    	realpath(".", path); //현재 절대 경로 위치를 path 배열에 저장
		printf("%s# ",path); // 현재 경로 출력
		
        fgets(cmd, sizeof(cmd) - 1, stdin); //명령어를 입력받음
		
        char *tokenArray[128]; //명령어의 arg를 token의 개념으로 분리하여 tokenArray에 담음
		
		int tokenNum = 0; //명령어로부터 분리된 token의 갯수

		char *token; //명령어로부터 분리된 token
		token = strtok(cmd, " \n"); //cmd 배열에 담긴 명령어 중 arg0에 해당하는 부분을 token에 임시적으로 담음 
		
		while(token != NULL){
			tokenArray[tokenNum] = token; //tokenArray에 token을 순차적으로 담음
			token = strtok(NULL, " \n"); //지속적으로 명령어로부터 arg를 분리시킴
			tokenNum++; //토큰갯수 +1
		} //모든 토큰을 담으면 반복문을 빠져나옴
		
		tokenArray[tokenNum] = '\0'; //마지막 인덱스에 널 문자 추가

		
		if (tokenNum == 0){ //만약 token이 없으면 처음으로 돌아감
			continue;
		}
		else if (strcmp(tokenArray[0], "help") == 0) //첫번째 token (arg0)가 help라면 
		{
			//이 쉘의 정보를 보여줌
			printf("& : background processing\n");
			printf("> : redirection\n");
			printf("help : show help\n");
			printf("exit : exit shell\n");
		}
		else if (strcmp(tokenArray[0], "exit") == 0)  //첫번째 token (arg0)가 exit라면 
		{	
			//숼을 나감
			printf("Good bye! \n");
			exit(0);
		}
		else if ((strcmp(tokenArray[0], "cd")) == 0 && tokenNum == 2) //첫번째 token (arg0)가 cd이고 arg2가 존재한다면
		{
			char *path; //경로를 담을 변수 포인터
			char buf[MAX_SIZE]; //임시적으로 담을 배열

			//현재 디렉토리 이름 복사 후 실패시 NULL 반환.
			path = getcwd(buf, sizeof(buf)); //buf에 경로를 담아와 path에 저장
			if (path != NULL) //path가 비어있지 않다면
			{
				path = strcat(path, "/"); //절대 경로로 지정
				path = strcat(path, tokenArray[1]); //arg2에 해당하는 목적 경로를 path에 붙이고
				chdir(path); // 해당 디렉토리로 이동
			}
		}
		
		int bg_flag = 0; //백그라운드 작업을 할건지 확인해주는 flag
		int re_flag = 0;//리다이렉션 작업을 할건지 확인해주는 flag

		int reTokenNum = -1; 
		//tokenArray에 담겨있는 ">" 문자열을 offset을 찾기 위해 사용할 변수 -> arg2를 가리키기 위해 사용할 것

		for (int i = 0; i < tokenNum; i++) //토큰의 갯수만큼 반복
		{
			if (strcmp(tokenArray[i], "&") == 0) //백그라운드 기호가 들어가는 문자열을 발견하면
			{
				bg_flag = 1; //백그라운드 작업 flag를 켜주고
				tokenArray[i] = NULL; //해당 "&"가 담긴 위치로 가서 NULL 로 바꿈
			}
			else if (strcmp(tokenArray[i], ">") == 0) //리다이렉션 기호가 들어가는 문자열을 발견하면
			{
				re_flag = 1; //리다이렉션 플레그를 켜주고
				reTokenNum = i + 1; //">" 다음에 있는 arg3으로 offset을 맞춤
				tokenArray[i] = NULL; // ">"문자열을 NULL로 바꿈
			}
		}
		
		int fork_process; //fork를 핸들링하기 위한 변수
		
		if((fork_process = fork()) == 0) { //fork 성공 여부 확인
			
			if (re_flag == 1){	//리다이렉션 flag가 켜져있다면
				int fd; //파일 디스크립터
				fd = open(tokenArray[reTokenNum], O_RDWR | O_CREAT, 0664); 
				//지정된 토큰의 arg3에 해당하는 파일 이름을 받아 파일을 생성
				dup2(fd, 1); // 자식에서 실행한 명령어의 모든 출력은 fd에 저장됨
				close(fd); // 파일디스크립터 반환
			}
			
			execvp(tokenArray[0], tokenArray); //명령어를 실행하고 프로세스 종료
			exit(1);
			
		} else if(fork_process < 0){ //fork_process가 0보다 작으면 fork 실패
			printf("fork Error");
			exit(-1);
		} else { // 그 외의 경우 wait를 해줌
			wait(NULL);
		}
		
    }
    return 0;
}

