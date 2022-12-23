/**
 * @file     kooshell.c
 * @author   Jiwon Koo 32210240
 * @email    itkoo2000@dankook.ac.kr
 * @version  0.1
 * @date     2022-12-09
 **/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/wait.h>
#define MAX_LINE 1024
void printDescription();

int main(int argc, char **argv)
{
  char buffer[MAX_LINE];
  pid_t pid;

  // 1. 쉘 안내문 출력
  printDescription();

  while(1){
    // 2-1. 메모리 설정 
    memset(buffer, 0x00, MAX_LINE);

    // 2-2. 입력받기
    printf("> ");
    fgets(buffer, MAX_LINE - 1, stdin);

    // 2-3. exit 명령어 입력시 쉘 종료
    if(strncmp(buffer, "exit\n", 5) == 0){
      break;
    }

    // 2-4. 입력 받은 문자의 끝에는 항상 \n 가 존재하기 때문에, 문자열 끝을 나타내는 \0 처리
    buffer[strlen(buffer) - 1] = '\0';

    // 2-5. 프로세스 복사
    pid = fork();

    // 2-6. 프로세스의 상태에 따라 나눠서 처리 
    if(pid > 0){ // 2-6-1. 자식 프로세스일 경우
      wait(NULL);
    } else if(pid == 0){ // 2-6-2. 부모 프로세스(fork해서 자기자신과 동일)
      if(execlp(buffer, buffer, NULL) == -1){ // 2-6-3. 실행 결과 에러 일때 
        perror("Failed to excute command");
        exit(0);
      }
    }
  } // end of while

  return 0;
}

// 안내문을 출력하는 함수 
void printDescription(){
  printf("******** Jiwon Koo's Shell ********\n");
  printf("Welcome to koo shell :)\n");
  printf("If you want to exit this shell, enter \"exit\" !\n");
}