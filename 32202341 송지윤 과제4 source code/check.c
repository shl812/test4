/**
* check.c : check commands function
* author : Jiyun Song
* @email : 32202341@dankook.ac.kr
* @version : 1.0
* date : 2022.12.5
**/

#include "setting.h"

// 입력받은 문자열이 공백인지 확인하는 함수
int checkCommand(char buffer[]){ 
    if(strcmp(buffer, "")!=0 && strcmp(buffer, "\t")!=0 && strcmp(buffer, " ")!=0)
        return 0;
    return 1;
}
