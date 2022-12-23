#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#define MAX 2048

// 사용자 입력을 받는 입력부분이다. strlok를 이용하여 띄어쓰기단위로 단어가 몇개 입력되었는지를 count으로 알 수 있고
// tokens로 입력받은 단어를 띄어쓰기 단위로 저장했다.
int input(char *buf,char * tokens[]){ 
 char * token;
 int Count = 0;
 token = strtok(buf, " ");
 while(token != NULL) {
     tokens[Count++] = token;
     token = strtok(NULL, " ");
 }
 tokens[Count] = NULL;
 return Count; //입력받은 토큰 개수가 몇개인지 리턴
}

//chdir함수를 이용하여 현재 작업디렉토리를 바꾸는 cd를 구현
void InnerCmd(char *token) {
 if(chdir(token) == -1) perror("cd");
}

// dup2()함수를 이용해 리다이렉션을 구현 
void Redirection(char *token[]) {
 pid_t pid;
 int fd_redir;
 printf("redirection완료\n");
	
 if((pid = fork()) == 0) { //자식프로세스
     fd_redir = open(token[3], O_RDWR | O_CREAT | O_TRUNC, 0641); //리다이렉션할 파일 열기
     dup2(fd_redir, STDOUT_FILENO); //화면출력내용을 리다이렉션할 폴더에저장
     close(fd_redir); //리다이렉션한 파일 닫기
     execvp(token[0], token);
     exit(0); 
 }
 wait(); 
}

void BackgroundExec(char *token[]) {
 pid_t pid;
 int fd_background;
 printf("backgound %s\n",token[0]);
	
 if((pid = fork()) == 0) {// 자식프로세스에서
     fd_background = open("/dev/null", O_RDONLY);
	 //dev/null쪽으로 가는 모든 정보는 null값이 되어버지리만 성공처리
     dup2(fd_background, STDOUT_FILENO); // dev/null로 리다이렉션된 내용은 저장이 되지 않는다. 
     execvp(token[0],token);
     exit(0);
 }
}

bool Execute(char *cmd) { // 사용자 입력을 인수로 받는 함수
 int tokenCount;
 int i = 0;
 char * tokens[MAX];
 pid_t pid;
 int Flag = 0;
 
 tokenCount = input(cmd,tokens); // input함수는 받은 입력 토큰의 갯수를 리턴한다.
	
 if(strcmp(tokens[0], "cd") == 0) { //첫번째 입력이 cd일경우
     InnerCmd(tokens[1]);
     return;
 }
	
 for(i = 0; i < tokenCount; i++) {
     if(!strcmp(tokens[i],">")) { // 입력토큰중에 ">"가 있을경우 
        Flag++;
     	Redirection(tokens);// 리다이렉션 실행
     }
     if(!strcmp(tokens[i],"&")) { // 입력토큰중에 "&"가 있을경우
        Flag++;
        BackgroundExec(tokens);// 백그라운드 실행
     } 
 }
	
 if(!Flag) { //Flag가 0이 아닌경우 실행
   if((pid = fork()) == 0) { // 자식프로세스
   	execvp(tokens[0],tokens); // 입력받은 명령어 실행
	exit(0);
   }
   wait();
 }
 memset(tokens, '\0', MAX); // 실행 후 입력 토큰의 값을 NULL로 초기화해준다.
 return 1;
}

int main() {
 char cmd[MAX];
 while(1) {
	 printf("%s$ ",get_current_dir_name());
	 fgets(cmd, sizeof(cmd) - 1, stdin); //fgets로 사용자 입력을 받는다.
     if(strncmp("exit",cmd,4) == 0) { //end입력시 종료
     printf("end, BYE");
     break;
     }
     Execute(cmd); // 위에 정의한 Execute 함수 실행
 }
}
