/*
 * myShell.c : make myShell program
 * @ author : Youngmin Koo
 * @ email : 32190192@dankook.ac.kr
 * @ version : 1.0
 * @ date : 2022.12.03
 * */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include <errno.h>

extern char **environ;// 환경변수

void help() { // 명령어를 help해주는 함수

   printf("exit\t: exit this shell\n");
   printf("help\t: show this help\n");
   printf("&\t: background processing\n");
   printf("cat\t: show this file\n");
	printf("rm\t: remove this file\n");
	printf("date\t: show present time\n");
	printf("ps\t: show process\n");
	
}

int parsing(char* buf, char* delims, char* tokens[]) {
	char* token; // strtok로 받은 값을 저장할 변수
	int cnt = 0; // 토큰의 개수를 세어줌

   token = strtok(buf, delims); // 토큰을 받습니다.

   while(token != NULL) { // 토큰이 마지막일 때 까지
      tokens[cnt] = token; // tokens 배열에 토큰별로 넣어줍니다.
      cnt++;   // 다음 index로 이동
      token = strtok(NULL, delims); // 다음 토큰을 받습니다.
   }
   tokens[cnt] = NULL; // 맨 마지막에 NULL값을 넣어줍니다.
   return cnt; // 토큰의 개수 리턴
}

bool exitFucn(){ // myShell 프로그램을 종료해주는 함수
	return false; // false를 리턴하면 프로그램종료
}

bool play(char* input) {
	 int  idx;
   int flag = 0;
   int i;
   int fd;
   int re = 0;
   int bg = 0;
   int re_check = 0;
   int bg_check = 0;
   int token_count;
   pid_t child;
   char delims[] = " \r\t\n";
   char* tokens[128];
   
   token_count = parsing(input, delims, tokens); // 파싱을 수행하여 파싱을하고 토큰의 개수를 리턴 받습니다.

   if(token_count == 0) // 아무것도 없으면
      return true; // return true
   if(strcmp(tokens[0], "exit") == 0) // 입력 값이 exitFucn()를 호출 false 리턴하며 프로그램 종료
   {return exitFucn();} // false 리턴합니다.
	if(strcmp(tokens[0], "help") == 0){ // help를 입력하면
      help(); // help 함수 호출
      return true;}
	if(strcmp(tokens[0], "clear") == 0){system("clear");} // clear해줍니다.
	

   for(i = 0; i < token_count; i++) { // 토큰의 개수만큼 반복합니다.
      if(strcmp(tokens[i],">") == 0) { // 토큰에 > 가 있으면
         re = i; // 인덱스를 저장해줍니다.
         re_check = 1; // rediretion이라고 확인합니다.
		 flag=O_WRONLY | O_CREAT | O_TRUNC; // O_TRUNC로 파일을 덮어씁니다.
         break; // for문 종료
      }
	   if(strcmp(tokens[i],">>") == 0) { // 토큰에 >> 가 있으면
         re = i; // 인덱스를 저장해줍니다.
         re_check = 1; // rediretion이라고 확인합니다.
		 flag = O_WRONLY | O_CREAT | O_APPEND; // O_APPEND로 파일 뒤에 추가해줍니다.
         break; // for문 종료
      }
   
      if(strcmp(tokens[i],"&") == 0) { // 토큰에 &가 있으면
         bg = i; // 인덱스를 저장해줍니다.
         bg_check = 1; // background processing이라고 확인합니다.
         break; //for문 종료
      }

   }   

   child = fork(); // 자식프로세스 생성
   if(child < 0) { //fork error 발생
		printf("fork error\n");
	   	return false; // 에러가 발생하였으므로 프로그램을 종료합니다.
   }
   else if(child == 0) { // 자식 프로세스 정상적으로 실행
     	if(bg_check==1) { // 백그라운드 프로세싱이면
         tokens[bg] = '\0';
        }
   
      if(re_check == 1) { // 리다이렉션이면
         fd = open(tokens[re + 1], flag, 0664); // 리다이렉션이면 O_APPEND로 아니면 O_TRUNC로 파일을 엽니다.
		if(fd < 0){ // 파일 열기 오류
			printf("Can't open file"); // 파일을 읽을수 없으면
			exit(-1); // 프로그램 종료
		}
         close(STDOUT_FILENO); // 모니터에 있는 값을 닫음
         dup2(fd, STDOUT_FILENO); // Standard output을 위에서 create한 fd로 가도록 설정
		  // dup2 실행 후, 자식의 모든 출력은 fd로 저장됨
         tokens[re] = '\0';
      	}
	if(strcmp(tokens[0], "rm") == 0){ // rm 명령어 수행
		execve("/bin/rm", tokens, NULL); //다른 프로그램 실행하고 자신은 종료
    	printf("execve error\n"); // 이 메시지가 보이면 실행 문제가 있는것입니다.
    	return false;
	}
	if(strcmp(tokens[0], "date") == 0){ // date 명령어 수행
		execve("/bin/date", tokens, NULL); //다른 프로그램 실행하고 자신은 종료
    	printf("execve error\n"); // 이 메시지가 보이면 실행 문제가 있는것입니다.
    	return false;
	}
	if(strcmp(tokens[0], "cat") == 0){ // cat명령어 수행
		execve("/bin/cat", tokens, NULL); //다른 프로그램 실행하고 자신은 종료
    	printf("execve error\n"); // 이 메시지가 보이면 실행 문제가 있는것입니다.
    	return false;
	}
	if(strcmp(tokens[0], "ps") == 0){ // ps명령어 수행
		execve("/bin/ps", tokens, NULL); //다른 프로그램 실행하고 자신은 종료
    	printf("execve error\n"); // 이 메시지가 보이면 실행 문제가 있는것입니다.
    	return false;
	}  
	   execvp(tokens[0],tokens); // 나머지 명령어를 수행합니다.
	   printf("execvp error"); // 이 메시지가 보이면 실행 문제가 있는것입니다.
	   return false;
	   
}
   else if(bg_check == false) { // 백그라운드 프로세싱이 아니면
      wait(NULL); // 자식 프로세스가 끝날때까지 기다립니다.
   } // 백그라운드 프로세싱이면 부모는 자식을 기다리지 않고 독립적으로 수행합니다.
   
   return true;
}   

int main() {
   char input[100]; // 명령어를 저장할 공간
   while(1) { // 사용자가 exit를 입력할 때까지 무한 반복합니다.
      printf("%s $ ",get_current_dir_name()); // 명령어 입력 전에 출력합니다.
      fgets(input, sizeof(input) - 1, stdin); // 사용자에게 입력을 받습니다.
      if(play(input) == false) // pasing함수가 false를 리턴하면 break; 하므로 프로그램 종료
         break;
   }
   
   return 0;
}
