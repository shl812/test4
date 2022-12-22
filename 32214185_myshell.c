#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

#define MAXL 50


char* start;
char* end;

int Fw;
int Fr;

// 리다이렉션 out command
// ex) program > file
// >는 program의 출력물을 file이라는 파일에 기록. 
// >> 는 파일의 끝에 내용을 덧붙이기
// >>인 경우 flag를 1로 설정하여 나중에 파일에 O_APPEND를 할 수 있도록
// 파일의 끝에 자료를 추가할 수 있도록 함.
// 그렇지 않은 경우 파일이 이미 존재한다면 내용을 다 지우고 쓰게끔 o_trunc를 해줌
// 파일 소유자는 읽기 및 쓰기 권한을 가지고
// 그룹 구성원과 시스템의 다른 사용자는 읽기 권한만 가지도록 함
// 앞의 것은 공백으로 분리해서 나눠서 저장해준다.
// 이는 이후 execvp에서 쓰이게 된다
// 포크를 하면 자식프로세서에는 0을 반환하게 되므로
// 괄호 안에 내용을 진행
// dup2 STDOUT_FILENO 서술자를 fr로 바꿈
// 예를 들어 이를 호출한 후에 printf를 하게 되면 이를 터미널이 아닌
// fr에 쓰게 된다.
// 
// 예를 들어 "ls al 디렉토리"를 한다면 arg_str 에는 ls가 가게되고
// execvp 가 ls 에 가서 arg_str인 ls -al str 을 싱행할 수 있도록 함.
// 
void redirectionOut_Command(char* str){
	char* inputC;
	int flag=0;
	if((inputC=strstr(str,">>"))!=NULL)
		flag=1;
	
	start=strtok(str,">");
	end=strtok(NULL,">");
	
	char* arg_str[45];
	int i=0; 
	arg_str[i] = strtok(start, " "); 
	while (arg_str[i]) {
		i++;
		arg_str[i] = strtok(NULL, " ");
	}
	end=strtok(end," ");

	if(fork()==0){
		if(flag==1)
			Fw=open(end,O_WRONLY|O_CREAT|O_APPEND,0644);
		else if(flag==0)
			Fw=open(end,O_WRONLY|O_CREAT|O_TRUNC,0644);
		
		dup2(Fw,STDOUT_FILENO);

		close(Fw);
		
		execvp(arg_str[0],arg_str);
		exit(0);
	}
	wait(NULL);
}
// 위의 함수와 같음
// 위와 달리 <<는 없으므로 나누진 않고 진행

void redirectionIn_Command(char* str){
	start=strtok(str,"<");
	end=strtok(NULL,"<");
	
	char* arg_str[45];
	int i=0;
	arg_str[i] = strtok(start, " "); 	
	while (arg_str[i]) {
		i++;
		arg_str[i] = strtok(NULL, " ");
	}
	end=strtok(end," ");
	if(fork()==0){
		Fr=open(end,O_RDONLY|O_CREAT,0644);
		dup2(Fr,STDIN_FILENO); 
		close(Fr);
		
		execvp(arg_str[0],arg_str);
		exit(0);
	}
	wait(NULL);
}

// system함수를 대신 할 system2 함수
// 리눅스의 명령어같은 경우 system을 이용하게 되면
// 따로 명령어 함수를 만들지 않아도
// 내장된 함수를 이용할 수 있다
// system함수는 유닉스 운영체제에는 모두 지원하며,
// 입력받은 명령어의 문자열을 실제로 실행시켜주는 함수
// 하지만 그러지 않고 직접 명령어를 구현하였음
// 구현하지 못한 명령어는 system2를 통해 실행될 수 있또록 함
// 아무것도 입력받은 게 없다면 넘기고
// 포크를 한 값이 0보다 작다면 시스템 에러 처리
// 즉 포크에 실패
// 자식 프로세스라면 그대로 실행
// 부모 프로세스라면 기다리게 한다.
// 그리고 상태를 반환
// execvp를 사용하여 실행하게 해주고 인자값은 위와 같이 넘김
// 부모는 포크 호출을 실패하거나 waitpid 가 다른 값을 넘겨주면 오류 처리
// 오류 발생시 상태값 -1 하고 탈출
// 그리고 마지막으로 상태값 반환

int system2(char *cmd){
	
    pid_t pid;
	
    int situation;
	char* arg_str[45];
	int i = 0;
	arg_str[i] = strtok(cmd, " "); 	
	while (arg_str[i]) {
		i++;
		arg_str[i] = strtok(NULL, " ");
	}

    if(cmd == NULL) return 1; 
    if((pid = fork()) < 0){ //포크 생성 오류
		
        situation = -1;    
    }
	else if(pid == 0){    // 자식
        execvp(arg_str[0],arg_str);
        exit(127);		
    }
	else{                  
        while(waitpid(pid, &situation, 0) < 0){
            if(errno != EINTR){	
                situation = -1;
                break;
            }
        }
	}
    return situation;
}

// 파이프를 사용할 경우 int형의 크기가 2인 배열이 필요한데
// 2개가 있을 수도 있으니 2개 생성
// 백 그라운드 실행
// 입력된 문자열이 &인 경우
// 이후 입력받은 명령에 따라
// 포크하여 백 그라운드 프로세스가 될 수 있도록 해준다.

// 파이프 처리
// 파이프가 2개를 쓸 수 있으니
// 해당 부분을 나눠준다
// 첫 번재 파이프의 입력값과 출력값을 따로 저장한다.
// pipeF1[0]은 함수를 호출 한 후 pipeF1[0]에 데이터를 입력 받을 수 있는 파일 디스크립터가 담긴다
//  파이프 출구 역할은 한다
//  반대로 pipeF1[1]은 데이터를 출력할 수 잇는 파일 디스크립터가 담기며
// 파이프 입구역할을 한다.
// 이를 이용해 진행한다.

int main(){
	char str[256];	 // 입력값
	char tempStor[256]; // 넘어가게 되면 임시로 넣어짐
	
	char *commandFirst[5]; // 처음 명령어
	char *commandSecond[5]; // 두번째 명령어
	char *commandThrid[5]; // 세번째 명령어
	
	char *pipeFirst = NULL;
	char *pipeSecond = NULL;
	char *pipeThird = NULL;
	
	int delAnd; // 백그라운드 때 & 삭제
	int i ;
	char *cha; // 문장 임시저장
	
	int pipeF1[2]; // 파이프 처리
	int pipeF2[2]; // 파이프 처리 (파이프가 두 개일 경우)
	
	pid_t pid;

	while(1){

		printf("JJH-Shell$ ");
		
		// 입력값 초기화해줌
		for(i=0 ; i < 256 ; i++){    
			str[i]='\0';
		}
		
		fgets(str,sizeof(str),stdin);
		str[strlen(str)-1] ='\0'; //개행문자 null로 치환

		//==================================================== 백 그라운드 실행 ====================================================//
		if(strchr(str,'&')!=NULL) {
		    printf("=======================================================================================\n");

		    delAnd = -1;
		    str[strlen(str)] = '\0';
		    i=0;
		    cha = strtok(str," ");	// 공백을 구분으로 구분

		    while(cha != NULL && i<3)
		    {
			    if(strcmp(cha,"&")==0)	// '&'를 찾으면 해당 idx를 저장
				    delAnd = i;
			    commandFirst[i] = cha;
			    cha = strtok(NULL," ");
			    i++;
		    }
		    commandFirst[i] = '\0';
		    commandFirst[delAnd] ='\0';	// & 문자 삭제 배열

		    pid = fork();
			
		    if(commandFirst[0]!=NULL) {
			if(pid == 0) {
				execvp(commandFirst[0],commandFirst);
				exit(0);
			}
			
		    }
		    
		    // 백 그라운드로 실행할 때 부모의 프로세스 아이디도 같이 출력
			
		    printf("[부모 프로세스 ID  = %d]\n",pid);
			printf("=======================================================================================\n");

		}



		//==================================================== 파이프 처리 ====================================================//

		else if(strchr(str,'|')!=NULL){
			// 파이프 기호 '|'를 기준으로 명령어를 구분한다. 
			pipeFirst = strtok (str,"|");
			pipeSecond = strtok (NULL, "|");
			pipeThird = strtok (NULL, "|");

			strcat(pipeFirst,"\0"); // 문자열 연결 \0 넣어줌
			strcat(pipeSecond, "\0");
			if(pipeThird != NULL) // 파이프가 한 개라면 null이 나올 거다.
				strcat(pipeThird,"\0");
			
			i=0;//초기화
			cha = strtok (pipeFirst," "); // 첫 번재거를 임시 저장
			while (cha != NULL && i<3){
				commandFirst[i]=cha; // 그리고 그걸 첫 번째 명령어에 저장한 후 끝 값 넣어주기
				cha = strtok (NULL, " ");
				i++;
			}
			commandFirst[i]='\0';
			
			i=0;//초기화
			cha = strtok (pipeSecond," "); // 첫번째값과 마찬가지
			while (cha != NULL && i<3){
				commandSecond[i]=cha;
				cha = strtok (NULL, " ");
				i++;
			}
			commandSecond[i]='\0';

			if(pipeThird != NULL) { // 파이프가 한개라면 null이 나와 안 들어간다
				i=0;
				cha = strtok(pipeThird," ");
				while(cha != NULL && i<3){
					commandThrid[i]=cha;
					cha = strtok(NULL," ");
					i++;
				}
				commandThrid[i]='\0';
			}
			// 파이프 함수가 성공적으로 실행되었다면 0을 아니라면 -1을 반환한다
			if(pipe(pipeF1) == -1){	    // pipeF1[0]은 읽기용, pipeF1[1]은 쓰기용
				printf("첫 번째 파이프 명령어를 불러오는데 실패하였습니다.\n");
				exit(1);
			}
			if(pipeThird != NULL) {	    // 파이프가 2개인지 확인해주고 pipeF2[0] 읽기용, pipeF2[1] 쓰기용 
				if(pipe(pipeF2) == -1) {
					printf("두 번째 파이프 명령어를 불러오는데 실패하였습니다.\n");
					exit(1);
				}
			}
			switch(fork()){ // 먼저 첫번째 명령어 프로세스 만들기
				case -1 : 
					perror("fork() 함수 오류"); 
					break;
				case 0 :
					  dup2(pipeF1[1],1); 
					 

					  close(pipeF1[0]);
					  close(pipeF2[0]);
					  close(pipeF2[1]);
					  close(pipeF1[1]);	  

					  execvp(commandFirst[0], commandFirst);
					  exit(0);  
			}
			switch(fork()) // 두 번째 명령어 프로세스
			{
				case -1 : perror ("fork() 함수 오류"); break;
				case 0 :
					  dup2(pipeF1[0],0);   // 그 전의 결과값을 가져와 그 다음 파이프의 입력으로 쓰게끔 해준다.
					  dup2(pipeF2[1],1);  

					  
					  close(pipeF1[0]);
					  close(pipeF2[0]);
					  close(pipeF2[1]);
					  close(pipeF1[1]);	  

					  execvp(commandSecond[0], commandSecond);
					  exit(0);  
			}
		    
			if(pipeThird != NULL) {

			    switch(fork()) // 세번째 프로세스가 있다면 생성
			    {
			    	case -1 : perror("fork() 함수 오류"); break;
			    	case 0 :

					  dup2(pipeF2[0],0);  // 두 번째 파이프의 출력물을 가져와 명령문을 실행한다.

					  close(pipeF1[0]);
					  close(pipeF2[0]);
					  close(pipeF2[1]);
					  close(pipeF1[1]);	   

					  execvp(commandThrid[0], commandThrid);
					  exit(0);
			    }
			}   
			
			close(pipeF1[0]);
			close(pipeF2[0]);
			close(pipeF2[1]);
			close(pipeF1[1]);
				       	
			while(wait(NULL) != -1);
		}
		//==================================================== 리다이렉션 처리 ====================================================//
		else if(strchr(str,'>')!=NULL){
			redirectionOut_Command(str);
		}
		else if(strchr(str,'<')!=NULL){
			redirectionIn_Command(str);
		}
		
		//==================================================== 종료 혹은 다른 명령어 실행 ====================================================//
		else{       

			str[strlen(str)]='\0';
			if(strcmp(str,"exit") == 0) //사용자 입력이 "exit"일 경우 쉘을 종료함
				exit(0);
			else 
				system2(str);
		    }   

	}
	return 0;
	
}

