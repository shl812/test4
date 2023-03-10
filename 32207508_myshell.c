#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#ifndef TRUE
#define TRUE	1
#endif

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64
#define TOKEN_CNT 64

typedef enum {false, true} bool;
typedef struct { // 커맨드 구조체
    char* name;
    char* desc;
    int ( *func )( int argc, char* argv[] ); // 함수포인터. 사용할 함수들의 매개변수를 맞춰줌
} COMMAND;

// 문자열을 공백 단위로 tokenizing해 char *배열로 return하는 함수
char **tokenize(char *line)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token_now = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int token_index = 0;  // tokens에서의 idx (현재 몇번째 token인가)
  int char_index = 0;   // tokens[token_index]에서의 idx
  int i;
	for (i =0; i < strlen(line); i++) {  // line 순회
    char ch = line[i];
    if (ch == ' ' || ch == '\n' || ch == '\t' || ch == '\0') {  // 공백문자일 경우 tokenize
      if (char_index != 0) {
        token_now[char_index] = '\0';  // 현재 token 마지막 null문자 추가
        tokens[token_index] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));  // 다음 token 동적 할당
        if (!strcmp("pps", token_now) || !strcmp("ttop", token_now)) {   // 상대 경로로 변경
          strcpy(tokens[token_index], "./");
          strcat(tokens[token_index++], token_now);  // 현재 tokens[token_index]에 복사
        }
        else {
          strcpy(tokens[token_index++], token_now);  // 현재 tokens[token_index]에 복사
        }
        char_index = 0;
      }
    }
    else {
      token_now[char_index++] = ch;
    }
  }
  free(token_now);
  tokens[token_index] = NULL;
  return tokens;
}

// 파이프 처리하는 함수
void do_pipe(char **tokens, int pipe_index[TOKEN_CNT], int pipe_count)
{
  int pipes[TOKEN_CNT][2] = {0, };  // 파이프 목록
  int pid;
  int status;
  /*****  1번째 명령어 실행  *****/
  pipe(pipes[0]);        // 파이프 생성
  if ((pid = fork()) < 0) {
    fprintf(stderr, "fork() error\n");
    exit(1);
  }
  else if (pid == 0) {
    close(STDOUT_FILENO);  // stdout close
    dup2(pipes[0][1], STDOUT_FILENO);   // 0번째 pipe stdout에 복사
    close(pipes[0][1]);    // pipe close
    execvp(tokens[pipe_index[0]], &tokens[pipe_index[0]]);  // exec
    fprintf(stderr, "execvp() error\n");
  }
  close(pipes[0][1]);     // pipe close
  wait(&status);          // exec 종료까지 대기
  if (WIFSIGNALED(status) || WIFSTOPPED(status)) {
    exit(1);
  }
  /*****  마지막 명령어 제외 모두 실행  *****/
  int i;
  for (i = 0; i < pipe_count - 1; i++) {
    pipe(pipes[i+1]);     // 파이프 생성
    if ((pid = fork()) < 0) {
      fprintf(stderr, "fork() error\n");
      exit(1);
    }
    else if (pid == 0) {
      close(STDIN_FILENO);  // stdin close
      close(STDOUT_FILENO); // stdout close
      dup2(pipes[i][0], STDIN_FILENO);    // pipe stdin에 복사
      dup2(pipes[i+1][1], STDOUT_FILENO); // pipe stdout에 복사
      close(pipes[i][0]);   // pipe close
      close(pipes[i+1][1]); // pipe close
      execvp(tokens[pipe_index[i+1]], &tokens[pipe_index[i+1]]);  // exec
      fprintf(stderr, "execvp() error\n");
    }
    close(pipes[i+1][1]); // pipe close
    wait(&status);        // exec 종료까지 대기
    if (WIFSIGNALED(status) || WIFSTOPPED(status))
      exit(1);
  }
  /*****  마지막 명령어 실행  *****/
  if ((pid = fork()) < 0) {
    fprintf(stderr, "error\n");
    exit(1);
  }
  else if (pid == 0) {
    close(STDIN_FILENO);  // stdin close
    dup2(pipes[pipe_count-1][0], STDIN_FILENO); // pipe stdin에 복사
    close(pipes[pipe_count-1][0]);  // pipe close
    close(pipes[pipe_count-1][1]);  // pipe close
    execvp(tokens[pipe_index[pipe_count]], &tokens[pipe_index[pipe_count]]);  // exec
  }
  wait(&status);  // exec 종료까지 대기
  if (WIFSIGNALED(status) || WIFSTOPPED(status)) {
    exit(1);
  }
  return;
}

// 명령어 exec하는 함수
void runcommand(char **tokens)
{
  bool has_pipe = false;
  int pipe_count = 0;
  int start_idx = 0;
  int len = 0;
  int pipe_index[TOKEN_CNT] = {0, };
  int i;
  for (i = 0; tokens[i] != NULL; i++, len++) {  // 파이프 있을 경우 token NULL로 변경
    if (!strcmp(tokens[i], "|")) {
      tokens[i] = NULL;
      pipe_index[++pipe_count] = i+1; // pipdIdx에 pipe로 구분되는 token들 시작 index 저장
      has_pipe = true;
    }
  }
  if (has_pipe) {
    do_pipe(tokens, pipe_index, pipe_count);
  }
  else {  // 파이프 없을 경우
    if (execvp(tokens[0], tokens) < 0) {
      if (errno == 2) {
        fprintf(stderr, "%s: error\n", tokens[0]);
        exit(1);
      }
      else {
        fprintf(stderr, "error! errno: %d\n", errno);
        exit(1);
      }
    }
    int status;
    wait(&status);
    if (WIFSIGNALED(status) || WIFSTOPPED(status)) {
      exit(1);
    }
  }
  exit(0);
}

int main(int argc, char* argv[])
{
  char  line[MAX_INPUT_SIZE];           
  char  **tokens;             
  int i;
  FILE* fp;
  if (argc == 2) {  // batch mode인 경우
    if ((access(argv[1], F_OK)) < 0) {
      fprintf(stderr, "not exists.\n");
      return -1;
    }
    if ((fp = fopen(argv[1],"r")) < 0) {
      fprintf(stderr, "error %s\n", argv[1]);
      return -1;
    }
  }
  while (1) { // 무한 반복
    /* 입력 시작 */
    bzero(line, sizeof(line));
    if (argc == 2) { // batch mode인 경우
      if (fgets(line, sizeof(line), fp) == NULL) { // file 모두 읽었을 경우
        break;  
      }
      line[strlen(line) - 1] = '\0';
    }
    else { // interactive mode인 경우
      printf("myshell : [%s] $ ",get_current_dir_name());
      scanf("%[^\n]", line);
      getchar();
    }
    /* 입력 완료 */
    line[strlen(line)] = '\n';  // 개행 문자 추가
    tokens = tokenize(line);    // tokenizing
    if (!tokens[0]) {           // 입력 x 처리
      continue;
    }
    int pid;
    if ((pid = fork()) < 0) {   // 자식 process 생성
      fprintf(stderr, "error!\n");
      exit(1);
    }
    else if (pid == 0) {        // 자식 프로세스인 경우
      runcommand(tokens);
    }
    else {  // 본 프로세스인 경우
      int status;
      waitpid(pid, &status, 0);
      if (WIFSIGNALED(status) || WIFSTOPPED(status) || WEXITSTATUS(status) == 1) {  // 에러 발생 처리
        fprintf(stderr, "error\n");
      }
      for (i=0;tokens[i]!=NULL;i++) { // 동적 할당 해제
        free(tokens[i]);
      }
      free(tokens);
    }
  }
  return 0;
}

int c_exit( int argc, char* argv[] ){
    printf("!!! Exit !!!\n");
    exit(0);
    
    return TRUE;
}

static COMMAND builtin_cmds[] =
{
    { "exit", "exit this shell", c_exit },
    { "quit", "quit this shell", c_exit },
    
};
