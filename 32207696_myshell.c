/**
* myshell.c  : make a shell
* @author    : Joungwoo Lee
* @email     : xw21yog@dankook.ac.kr
* @version   : 1.0
* @date      : 2022-12-04
**/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/types.h>
#include<sys/wait.h>
#include <sys/stat.h>
#define MAX 1024


void help() { // help()호출시 도움말 출력
   printf("/------------------Shell---------------------/\n");
   printf("기존의 shell처럼 사용할 수 있습니다.\n\n");
   printf("내장되어있는 몇가지 명령어는 아래와 같습니다.\n");
   printf("help\t: 현재 도움말을 출력합니다.\n");
   printf("ls\t: 현재 파일의 목록을 표시합니다.\n");
    printf("rm\t: 파일이나 디렉토리를 삭제합니다.\n");
   printf("mkdir\t: 디렉토리를 생성합니다.\n");
   printf("cp -a\t: 파일을 복사합니다.\n");
   printf("cat\t: 파일의 내용을 출력합니다\n");
   printf("&\t: background processing\n");
   printf(">\t: 명령어 > 파일  (덮어쓰기 redirection)\n");
   printf(">>\t: 명령어 >> 파일 (붙여쓰기 redirection)\n");
   printf("exit\t: shell을 종료합니다.\n");
   printf("/--------------------------------------------/\n");
}
// 시프 5장ppt 20p 참조
int tokenize(char* buf, char* delims, char* tokens[], int maxTokens) {
   int token_count = 0;
   char* token;

   token = strtok(buf, delims);

   while(token != NULL && token_count < maxTokens) {
      tokens[token_count] = token;
      token_count++;   
      token = strtok(NULL, delims);
   }
   // parshing
   tokens[token_count] = NULL;
   return token_count;
}

bool run(char* line) {

   int fd;
   int x,y= 0;
   int check_x = 0;
   int check_y = 0;
   int token_count;
   pid_t pid;
   char delims[] = " \r\t\n";
   char* tokens[128];
   token_count = tokenize(line, delims, tokens, sizeof(tokens)/sizeof(char*));

   if(token_count == 0) //아무런 입력이 없을 경우 
      return true;
   
   if(strcmp(tokens[0], "exit") == 0) // tokensp[0]에 오는 문자열이 exit이면 shell을 종료한다.
      return false;
   
   if(strcmp(tokens[0], "help") == 0){ // tokensp[0]에 오는 문자열이 help이면 도움말을 출력한다.
      help();
      return true;
   }   
   //redirection, background processing
   
   for(int i = 0; i < token_count; i++) {
      if(strcmp(tokens[i],">") == 0){
         x = i;
         check_x = 1;
         break;
	  }
      if(strcmp(tokens[i],">>") == 0){
         x =i;
         check_x =2;
         break;
	  }
      if(strcmp(tokens[i],"&") == 0) {
         y = i;
         check_y = 1;
         break;
	  }
   }   

//Background Processing과 Redirection을 해야 하는지 검사 

   pid = fork();
   if(pid < 0) { //fork error
      printf("fork Fail!\n");
      return false;
   }
   else if(pid== 0) { //자식
      
      if(check_x == 1) {// > 입력시 출력 재지정
         fd = open(tokens[x + 1], O_WRONLY|O_TRUNC|O_CREAT, 0664);
         close(STDOUT_FILENO);
         dup2(fd, STDOUT_FILENO);
         tokens[x] = '\0';
      }
      if(check_x == 2){ // >> 입력시 출력 재지정
         fd = open(tokens[x + 1], O_RDWR| O_APPEND, 0664 ); // >>는 덮어쓰기(redirection)이므로 O_CREAT대신 O_APPEND를 써준다.
         close(STDOUT_FILENO);
         dup2(fd, STDOUT_FILENO);
         tokens[x]='\0';
      }
      if(check_y == 1) {
         printf("\n");
         tokens[y] = '\0';
      }
      
      
      if(strcmp(tokens[0],"ls") == 0){ // strcmp함수를 이용하여 tokens[0] 위치에 오는 문자열과 비교하여 execve()실행
        execve("/bin/ls",tokens,NULL);
      }else if(strcmp(tokens[0],"cat") == 0){
        execve("/bin/cat",tokens,NULL);
      }else if(strcmp(tokens[0],"rm") == 0){
         execve("/bin/rm",tokens,NULL);
      }else if(strcmp(tokens[0],"mkdir") == 0){
         execve("/bin/mkdir",tokens,NULL);
      }else if(strcmp(tokens[0],"ps") == 0){
         execve("/bin/ps",tokens,NULL);
      }else if(strcmp(tokens[0],"pwd") == 0){
         execve("/bin/pwd",tokens,NULL);
      }else if(strcmp(tokens[0],"date") == 0){
          execve("/bin/date",tokens,NULL);   
      }else // 그외의 경우(gcc등) execvp()를 실행한다.
          execvp(tokens[0],tokens);

      
      printf("실행불가\n"); //이 문장 출력시 execvp가 제대로 안됨
      
      return false;
      
   }
   else if(check_y == false) {
      wait(NULL);
   }
   
   return true;
}   

int main() {
   char line[1024];
   char *buf[MAX];


   while(1) {
      
      printf("%s $ ",getcwd(*buf,MAX)); //현재 작업 디렉토리 위치를 알려줌
      fgets(line, sizeof(line) - 1, stdin);
      if(run(line) == false)
         break;
   
   }
   return 0;
}