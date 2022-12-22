#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
/*
student_number: 32191976
name: 배건우
date: 2022-12-09
*/

int BA = 0;
int RE = 0;
int tokenize(char* buf, char* delims, char* tokens[], int maxTokens) {  //토큰화 함수
  char* token;  //토큰 문자열
  int token_cnt = 0;  //토큰 카운트변수 생성
  token = strtok(buf, delims);  //delims를 기준으로 buf를 토큰화한다
  while (token != NULL && token_cnt < maxTokens) {
    tokens[token_cnt++] = token;
    token = strtok(NULL, delims);
  }
  tokens[token_cnt] = '\0'; //문자열로 만들기위해 NULL을 마지막에 추가해준다
  return token_cnt; //토큰카운트 변수 리턴
}

bool run(char* line) {  //명령어를 실행하는 함수
  int token_cnt;  
  char* tokens[1024]; 
  char* delims =" ";  //delims는 빈칸으로 지정
  int fd1, fd2;
  char* tokencp[1024];
  pid_t fork_return = 0;  //fork를했을때의 리턴값
  BA = 0;
  RE = 0;
  token_cnt = tokenize(line, delims, tokens, sizeof(tokens) / sizeof(char*));
  for (int i = 0;i < token_cnt;i++) {
    if (!strcmp(tokens[i], ">"))  //redirction일경우 
      RE++; //전역변수 RE 1증가
    if (!strcmp(tokens[i], "&"))  //background일경우
      BA++; //전역변수 BA 1증가
    printf("%s \n", tokens[i]); //tokens[i]를 출력
  }
  if (strcmp(tokens[0], "cd") == 0) { //명령어가 cd일경우
    if (chdir(tokens[1]) != 0)
      perror("cd");
    return 0;
  }
  
  if (!BA) {  //BA가 0인경우
    if ((fork_return = fork()) == 0) {  //자식 프로세스를 실행
      execvp(tokens[0], tokens);  //execvp메소드 사용
      exit(0);
    }
    wait();
  }
  if (BA) { //BA가 1이상인 경우
      if ((fork_return = fork()) == 0) {
        fd2 = open("/dev/null", O_RDONLY);  //읽기 전용으로 파일을 염
        dup2(fd2, STDOUT_FILENO);//fd2파일을 복제해 출력한다
        execvp(tokens[0], tokens);  //execvp메소드 사용
        exit(0);  //종료
      }
    }
  else if (RE) {  //RE가 1 이상인경우
    if ((fork_return = fork()) == 0) {  //자식프로세스를 실행
      fd1 = open(tokens[3], O_RDWR | O_CREAT | O_TRUNC, 0641); //읽기 쓰기가 모두 가능하고 파일이 없으면 생성하는 open함수 사용
      dup2(fd1, STDOUT_FILENO); //fd1파일을 복제해 출력한다
      close(fd1); //fd1닫기
      execvp(tokens[0], tokens);//execvp메소드 사용
      exit(0);  //종료
    }
    wait();
  }
  memset(tokens, '\0', 1024);   //할당 메모리 해제
  return 1; //true리턴
}


int main() {
  char line[1024];  //명령어를 입력받는다
  while (1) {
    printf("[%s] $", get_current_dir_name()); 
    fgets(line, sizeof(line) - 1, stdin); //명령어를 sizeof(line)-1만큼 입력받는다
    if (run(line) == false) //명령어오류일경우
      break;  //break
  }
  return 0;
}