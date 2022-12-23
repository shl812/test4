/**
  * HW3-1.c		: System Programming Homework #4, 
  * @auther		: Jeemin Choi
  *	@email		: jeemin1987@dankook.ac.kr
  *	@version	: 1.1
  * @date		: 2022. 12. 09
**/

# include <stdio.h>
# include <stdlib.h>
# include <stdbool.h>
# include <string.h>
# include <unistd.h>

bool canExecute(char *_command) { 	// shell에 입력된 명령어가 유효한지 확인하는 함수
    if (strstr(_command, "sh") != NULL) return false;
    if (strstr(_command, "cat") != NULL) return false;
    if (strstr(_command, "flag") != NULL) return false;
     if (strstr(_command, "?") != NULL) return false;
    if (strstr(_command, "*") != NULL) return false;
    if (strstr(_command, "[") != NULL) return false;
    return true;
}

void setup_environment() { 			// stdin, stdout에 대해 버퍼링 하지 않음
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
}

int main() {
    setup_environment();

    char command[0x100] = { '\0' };
    
    printf("[+] Input command\n");
    printf("> ");
    read(0, command, 0xff); 		// 명령어 입력받음
    if (canExecute(command)) { 		// 유효하다면
        system(command); 			// 해당 명령어 실행
    } else {
        printf("[!] invalid commend.\n"); 		// 에러 메시지 출력
    }

    return 0;
}