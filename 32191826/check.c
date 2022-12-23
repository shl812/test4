//이름 : 박주희
//학번 : 32191826
//날짜 : 2022.12.09

#include "myshell.h"

// input 예외처리
int checkInput(char buf[]){
    if(strcmp(buf, "")!=0 && strcmp(buf, "\t")!=0 && strcmp(buf, " ")!=0)
        return 0;
    return 1;
}
