/**
* mycat:	make shell
*@author:	Sanga Kim
*@email:	cometokr1@naver.com
*@version:	1.0
*@date:		2022.12.09
**/

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

void help() {

   printf("exit\t: write 'exit' to exit this shell\n");
   printf("help\t: show this help\n");
   printf("&\t: background processing\n");
   printf(">\t: redirection\n");
   printf("cp(copy directory)\t: cp [원본] 복사할 파일\n");
   printf("cat(catenate)\t: cat [파일]  \n");
   printf("ls\t: list segment  \n");
   printf("pwd\t: print working directory \n");
    
}
// 문자열을 공백 단위로 tokenize하는 함수
int tokenize(char* buf, char* delims, char* tokens[], int maxTokens) {
    //명령어 입력시 각 파라미터를 구분
   int token_count = 0;
   char* token;

   token = strtok(buf, delims);    //delims 기준으로 문자열 자르기

   while(token != NULL && token_count < maxTokens) {    
      tokens[token_count] = token;
      token_count++;   
      token = strtok(NULL, delims); //delims를 기준으로 문자열 잘라냄
   }
      //  parsing
   tokens[token_count] = NULL;
   return token_count;
}

bool run(char* line) { //명령어 exec하는 함수
   int i;
   int fd;
   int re = 0;
   int bg = 0;
   int re_check = 0;
   int bg_check = 0;    //초기화
   int token_count;
   pid_t child;
   char delims[] = " \r\t\n";    //공백, 탭, 개행으로 이루어진 구분자 선언
   char* tokens[128];
   
   token_count = tokenize(line, delims, tokens, sizeof(tokens)/sizeof(char*)); //token 수 count
   
   if(token_count == 0) //token 수가 0이면 true리턴
      return true;
   
    //strcmp 함수는 문자열이 같은지를 체크, 같으면 0을 반환
   if(strcmp(tokens[0], "exit") == 0) //exit 명령어
      return false;
   

   
   if(strcmp(tokens[0], "help") == 0){ //help 명령어
      help();    //안내문구 출력
      return true;
   }
   
   

   for(i = 0; i < token_count; i++) {
      if(strcmp(tokens[i],">") == 0) {    //redirection
         re = i;
         re_check = 1;
         break;
      }
   
      if(strcmp(tokens[i],"&") == 0) {    //background processing
         bg = i;
         bg_check = 1;
         break;
      }

   }   

   child = fork();        //자식프로세스 생성
   if(child < 0) { //fork error
      printf("fork error\n");   
      return false;
   }
   else if(child == 0) { //자식 프로세스인 경우 
      if(bg_check) {    //백그라운드 실행일때
         tokens[bg] = '\0';    
      }
   
      if(re_check == 1) {
         fd = open(tokens[re + 1], O_CREAT | O_WRONLY | O_APPEND, 0664);    //O_APPEND로 리다이렉션 구현
         close(STDOUT_FILENO);
         dup2(fd, STDOUT_FILENO);   //std 출력을 fd에 복사
         tokens[re] = '\0';
      }
      execvp(tokens[0], tokens);        //exec로 원하는 것을 실행
      printf("execvp error\n"); 
      
      return false;
   }
   else if(bg_check == false) {    //백그라운드 프로세싱이 아닐때
      wait(NULL);
   }
   
   return true;
}   

int main() {
   char line[1024];
    printf("SangA's shell\n");    //본인이 만든 쉘이라고 표시해주기
   
   while(1) {
      printf("%s $ ", get_current_dir_name());
      fgets(line, sizeof(line) - 1, stdin);    //stdin을 linesize-1만큼 가져온다
                                               //-1만큼 가져오는 이유는 마지막에 엔터가 입력되기 때문
      if(run(line) == false)
         break;
   }
   
   return 0;
}

