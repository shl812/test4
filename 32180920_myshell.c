#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
int main(){
char str[256]; // 사용자 입력 문자열
char strSave[256]; // 임시저장용
char *pipeFront = NULL;
char *pipeMiddle = NULL;
char *pipeEnd = NULL;
char *cmd1[4]; //앞쪽 명령어
char *cmd2[5]; //중간 명령어
char *cmd3[5]; //마지막 명령어
int ampersandIdx; // 백그라운드 사용 시 입력 인수에서 '&'를 삭제하기 위한 변
int i ;
char *pch;//문장나눌때 임시 저장
pid_t pid;
//signal 처리
sigset_t block; // 막아놓을 시그널 목록
sigemptyset(&block);
sigaddset(&block, SIGINT); //목록에 해당 ^C시그널 추가
sigaddset(&block, SIGQUIT); //^\시그널 추가
sigprocmask(SIG_BLOCK, &block, NULL);
while(1){

printf("jyShell$ ");
//프롬프트가 변수 초기화
for(i=0 ; i < 256 ; i++){
str[i]='\0';
}
fgets(str,sizeof(str),stdin);
str[strlen(str)-1] ='\0';
/************************************* 명령어를 background로

실행할 경우 **********************************/

if(strchr(str,'&')!=NULL) {


ampersandIdx = -1;
str[strlen(str)] = '\0';
i=0;
pch = strtok(str," ");// 공백을 구분으로 구분
while(pch != NULL && i<3)
{
if(strcmp(pch,"&")==0) // '&'를 찾으면 해당 idx를 저장

ampersandIdx = i;
cmd1[i] = pch;
pch = strtok(NULL," ");
i++;
}
cmd1[i] = '\0';
cmd1[ampersandIdx] = NULL; // 사용자 입력 명령어문자열에


pid = fork();
if(cmd1[0]!=NULL) {
if(pid == 0) {
execvp(cmd1[0],cmd1);
exit(0);
}

wait(NULL);
}


//backgroud로 실행할 경우 부모의 pid 값을 출력한다.
printf("[Process id %d]\n",pid);
}
/************************************************** 단일 명령

어 수행 *************************************************/
else{ // background가 아닌 일반 명령어인 경우
str[strlen(str)]='\0';
if(strcmp(str,"logout") == 0) //사용자 입력이 "logout"일 경우 쉘을 종료함

exit(0);
else
system(str);

}
}
return 0;
}