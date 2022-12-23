#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

void help() { // help() 호출시 명령어들에 대한 정보 출력
   printf("---------------Mini shell---------------");
   printf("help\t: 간단한 명령어 도움말 확인.\n");
   printf("exit\t: 종료.\n");
   printf("ls\t: 지금 위치한 파일/디렉토리 목록 출력.\n");
   printf("date\t: 현재의 날짜와 시간 출력\n");
   printf("pwd\t: 실행한 디렉토리 절대경로 출력\n");
   printf("mkdir [옵션] [파일이름]: 새로운 디렉토리 생성\n");
   printf("rmdir [옵션] [파일이름]: 해당 디렉토리 삭제\n");	
   printf("cat [옵션] [파일이름]: 파일의 내용 출력\n");
   printf("&\t: background processing\n");
   printf(">\t: redirection(내용복사)\n");
   printf(">>\t: redirection(내용복사 후 끝에 덧붙여 쓰기)\n");
   printf("/-----------------------------------------/\n");
}
//문자열을 자르기 위한 함수
int parse(char* buffer, char* delims, char* tokens[], int tmax) {
//인자로 자를 문자열, 문자를 기준을 정하는 구분자, 분할한 문자열을 저장할 배열, 최대 크기를 받음.
   int tcount = 0;//처음에는 토큰 개수를 0으로 초기화
   char* token;//char형 포인터 선언

   token = strtok(buffer, delims); //strtok함수를 이용하여 문자열을 자름.
	//buffer가 자를 문자열, delims는 구분자.

   while(token != NULL && tcount < tmax) { //token 이 null이 이니고 최대 개수보다 적을 때
      tokens[tcount] = token;//tokens[tcount] 위치에 자른 문자열 저장
      tcount++;   //tcount변수 1증가
      token = strtok(NULL, delims);//첫번째 인자에 NULL을 넣음으로써 남은 문자열을 대상으로 parsing 진행.
   }
 	// parshing 진행
	tokens[tcount] = NULL;
	return tcount;
}

int exec(char* line) {
   int i;
   int fd;//file descriptor
   char delims[] = " \r\t\n";//구분자
   char* tokens[128];
   char* token;
   int re_n = 0;//redirection
   int re_check = 0;//redirection check	
   int bg_n = 0;//background
   int bg_check = 0;//background check
   int tcount;
   pid_t child;//자식프로세스
	
   tcount = parse(line, delims, tokens, sizeof(tokens)/sizeof(char*)); //tcount변수에 parse함수를 호출하여 값을 저장
	
   if(tcount == 0) //아무런 입력이 없을 경우 
      return 1;
   
   // tokens[0]에 위치한 문자열이 exit라면
   if(strcmp(tokens[0], "exit") == 0) 
      return 0;  //0을 리턴

   // tokens[0]에 위치한 문자열이 help라면
   if(strcmp(tokens[0], "help") == 0){
      help();//help()함수 호출 후
      return 1;//1을 리턴
   }

   
   // redirection 과 background processing
	for(i = 0; i < tcount; i++) {//tcount 까지 반복
      if(strcmp(tokens[i],">") == 0) {//tokens[i]에 위치한 문자열이 >라면
         re_n = i;//re변수에 현재 i를 저장
         re_check = 1;//redirection check 변수에 1을 저장
         break;
      }
      if(strcmp(tokens[i],">>") == 0) {//tokens[i]에 위치한 문자열이 >>라면
         re_n = i;//re변수에 현재 i를 저장 
         re_check = 2;//redirection check 변수에 2를 저장
         break;
      }
   
      if(strcmp(tokens[i],"&") == 0) {//tokens[i]에 위치한 문자열이 &라면
         bg_n = i;//bg변수에 현재 i를 저장
         bg_check = 1;//background check 변수에 1을 저장
         break;
      }

   }   

   child = fork();//프로세스를 생성하기 위해 fork함수 이용
   if(child < 0) { //fork error발생시
      printf("fork 처리중 에러 발생\n");//fork 에러가 발생했다는 문구 출력 
      return 0;
   }
   else if(child == 0) { //자식프로세스이면(pid = 0)
      if(bg_check) { //background check 가 1이라면 
         tokens[bg_n] = '\0';//bg_n변수 위치의 tokens에 null값 
      }
      if(re_check == 1) {// redirection check 변수가 1이라면 
         fd = open(tokens[re_n + 1], O_WRONLY|O_TRUNC|O_CREAT, 0664);
		 //File descriptor 에 open함수를 이용하여 re_n + 1 에 위치한 tokens를 파일 명으로 가져오고
		 // 쓰기 전용, 기존의 파일 내용을 모두 삭제해서 새로 파일을 생성
		 //접근 권한은 0644(110 110 100)
         close(STDOUT_FILENO);//파일 close
         dup2(fd, STDOUT_FILENO);//dup2 함수를 이용하여 fd로 연 파일에 표준출력.
         tokens[re_n] = '\0';//re_n변수 위치의 tokens에 null값
      }
      if(re_check == 2) {// >> redirdection check 변수가 2라면
         fd = open(tokens[re_n + 1], O_RDWR|O_APPEND, 0664);
		 //File descriptor 에 open함수를 이용하여 re_n + 1 에 위치한 tokens를 파일 명으로 가져오고
		 //읽기전용, 기존의 파일에 덧붙임
		 //접근 권한은 0664(110 110 100)
         close(STDOUT_FILENO);//file close
         dup2(fd, STDOUT_FILENO);//dup2 함수를 이용하여 fd로 연 파일에 표준출력.
         tokens[re_n] = '\0';//re_n 변수 위치의 tokens 에 null 값
      }
      if(bg_check) {//background check 가 1이라면
         printf("\n");//줄바꿈
         tokens[bg_n] = '\0';//bg_n 변수 위치의 tokens 에 null 값
         }
      
      //bin에는 아주 기본적인 프로그램들이 위치해있다. 리눅스가 돌아가기 위해 가장 최소로 필요한
	  //프로그램들이 보관되어있다. 바이너리 파일들이 저장되어 있는데 흔히 쓰는
	  //ls 나 cat 등과 같은 명령어등이 위치한다. 
	  //strcmp함수를 이용하여 문자열 비교 
      if(strcmp(tokens[0],"ls")==0){//tokens[0]에 위치한 문자열이 ls라면
           execve("/bin/ls",tokens,NULL);//exceve함수를 이용하여 bin 디렉토리 밑에 있는 ls 실행 
      }else if(strcmp(tokens[0],"date")==0){//tokens[0]에 위치한 문자열이 date 라면
           execve("/bin/date",tokens,NULL);//execve함수를 이용하여 bin 디렉토리 밑에 있는 date 실행
         }else if(strcmp(tokens[0],"pwd")==0){//tokens[0]에 위치한 문자열이 pwd라면
           execve("/bin/pwd",tokens,NULL);//execve함수를 이용하여 bin 디렉토리 밑에 있는 pwd 실행
         }else if(strcmp(tokens[0],"mkdir")==0){//tokens[0]에 위치한 문자열이 mkdir라면
           execve("/bin/mkdir",tokens,NULL);//execve함수를 이용하여 bin 디렉토리 밑에 있는 mkdir 실행
         }else if(strcmp(tokens[0],"ps")==0){ //tokens[0]에 위치한 문자열이 ps 라면
           execve("/bin/ps",tokens,NULL);//execve함수를 이용하여 bin 디렉토리 밑에 있는 ps 실행
      }else if(strcmp(tokens[0],"cat")==0){//tokens[0]에 위치한 문자열이 cat이라면 
           execve("/bin/cat",tokens,NULL);   //execve함수를 이용하여 bin 디렉토리 밑에 있는 cat 실행
         }else
           execvp(tokens[0],tokens);
      
      printf("execve 처리중 에러 발생\n"); //이 문장 출력시 execve가 제대로 안됨 (이어서 출력한다는 것은 다른프로그램이 실행되지 않았음을 의미)
      return 0;
   }
   else if(bg_check == 0) {//bg_check 가 0이라면
      wait(NULL);//부모 프로세스 wait 하고 자식 프로세스를 실행
   }
   
   return 1;
}   

int main() {
   char arr[1024];
   
   while(1) {//exit가 입력될때 까지
      printf("%s %% ", getcwd(arr, 1024));//getcwd 함수를 이용하여 현재 작업 디렉토리의 이름 반환.
      fgets(arr, sizeof(arr) - 1, stdin);
	  //fgets함수를 이용하여e 파일로 부터 문자열을 가지고 옴
	  //arr-1 크기 만큼의 문자열을 가지고 와서 arr에 문자열을 저장
	  //표준입력으로 처리
      if(exec(arr) == 0)//arr이 0이되면 멈춤.
         break;
   }
   
   return 0;
}
