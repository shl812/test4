#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#define MAX_INPUT 64

/**
 * myshell.c     : myshell program
 * author      : 이종혁_32193483_시스템프로그래밍 3분반
 * email       : dkdlelqkrtm@gmail.com
 * date	       : 2022. 12. 09
**/

int main(int argc, char **argv) {
        char buf[MAX_INPUT]; // 64개 INPUT을 가질 수 있는 BUFFER
        pid_t pid;
        printf("MYSHELL \n");
        while(1)
        {
                memset(buf, 0x00, MAX_INPUT); // 버퍼 값 0으로 초기화
                fgets(buf, MAX_INPUT - 1, stdin);
                // 키보드 입력을 63바이트만큼 가져옴 (마지막 엔터 고려)

    
                if(strncmp(buf, "exit\n", 5) == 0) {
                        break;
                }
                buf[strlen(buf) - 1] = 0x00; // 마지막 입력 엔터를 0으로 초기화

                pid = fork();
                if(pid == 0) { // 자식 프로세스
                        if(execlp(buf, buf, NULL) == -1) {
                                        printf("command execution is failed \n");
                                        exit(0);
                        }// execlp 함수를 이용해 분기한 프로세스 -> 실행 프로세스
                }
                if(pid > 0) { //  부모 프로세스
                        wait(NULL);
                }
        } 
        return 0;
}