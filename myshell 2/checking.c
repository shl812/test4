/*
	이름 : 박민규
	학번 : 32191597
	날짜 : 2022-12-03
*/

#include "myshell.h"

// 사용자에게 받는 input의 예외를 checking해주는 함수
int checkInput(char buf[]){
    if(strcmp(buf, "")!=0 && strcmp(buf, "\t")!=0 && strcmp(buf, " ")!=0)
        return 0;
    return 1;
}
