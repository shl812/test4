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


void help() { // help()ȣ��� ���� ���
   printf("/------------------Shell---------------------/\n");
   printf("������ shelló�� ����� �� �ֽ��ϴ�.\n\n");
   printf("����Ǿ��ִ� ��� ��ɾ�� �Ʒ��� �����ϴ�.\n");
   printf("help\t: ���� ������ ����մϴ�.\n");
   printf("ls\t: ���� ������ ����� ǥ���մϴ�.\n");
    printf("rm\t: �����̳� ���丮�� �����մϴ�.\n");
   printf("mkdir\t: ���丮�� �����մϴ�.\n");
   printf("cp -a\t: ������ �����մϴ�.\n");
   printf("cat\t: ������ ������ ����մϴ�\n");
   printf("&\t: background processing\n");
   printf(">\t: ��ɾ� > ����  (����� redirection)\n");
   printf(">>\t: ��ɾ� >> ���� (�ٿ����� redirection)\n");
   printf("exit\t: shell�� �����մϴ�.\n");
   printf("/--------------------------------------------/\n");
}
// ���� 5��ppt 20p ����
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

   if(token_count == 0) //�ƹ��� �Է��� ���� ��� 
      return true;
   
   if(strcmp(tokens[0], "exit") == 0) // tokensp[0]�� ���� ���ڿ��� exit�̸� shell�� �����Ѵ�.
      return false;
   
   if(strcmp(tokens[0], "help") == 0){ // tokensp[0]�� ���� ���ڿ��� help�̸� ������ ����Ѵ�.
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

//Background Processing�� Redirection�� �ؾ� �ϴ��� �˻� 

   pid = fork();
   if(pid < 0) { //fork error
      printf("fork Fail!\n");
      return false;
   }
   else if(pid== 0) { //�ڽ�
      
      if(check_x == 1) {// > �Է½� ��� ������
         fd = open(tokens[x + 1], O_WRONLY|O_TRUNC|O_CREAT, 0664);
         close(STDOUT_FILENO);
         dup2(fd, STDOUT_FILENO);
         tokens[x] = '\0';
      }
      if(check_x == 2){ // >> �Է½� ��� ������
         fd = open(tokens[x + 1], O_RDWR| O_APPEND, 0664 ); // >>�� �����(redirection)�̹Ƿ� O_CREAT��� O_APPEND�� ���ش�.
         close(STDOUT_FILENO);
         dup2(fd, STDOUT_FILENO);
         tokens[x]='\0';
      }
      if(check_y == 1) {
         printf("\n");
         tokens[y] = '\0';
      }
      
      
      if(strcmp(tokens[0],"ls") == 0){ // strcmp�Լ��� �̿��Ͽ� tokens[0] ��ġ�� ���� ���ڿ��� ���Ͽ� execve()����
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
      }else // �׿��� ���(gcc��) execvp()�� �����Ѵ�.
          execvp(tokens[0],tokens);

      
      printf("����Ұ�\n"); //�� ���� ��½� execvp�� ����� �ȵ�
      
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
      
      printf("%s $ ",getcwd(*buf,MAX)); //���� �۾� ���丮 ��ġ�� �˷���
      fgets(line, sizeof(line) - 1, stdin);
      if(run(line) == false)
         break;
   
   }
   return 0;
}