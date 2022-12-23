/*
	이름 : 박민규
	학번 : 32191597
	날짜 : 2022-12-03
*/

#include "myshell.h"

// 사용자 입력 토큰화
int tokenized(char buff[], char* arg[] ,char delim[]){
    char* saveptr;
    char* s;
    int num = 0;
    
    s = strtok_r(buff, delim, &saveptr);
    while(s) {
        arg[num++] = s;
        s = strtok_r(NULL, delim, &saveptr);
    }
    
    arg[num] = (char *)0;
    
    return num;
}
