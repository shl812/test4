/**
* token.c : tokenization function
* author : Jiyun Song
* @email : 32202341@dankook.ac.kr
* @version : 1.0
* date : 2022.12.5
**/

#include "setting.h"

// 입력받은 문자열을 parsing 하는 함수
int tokenized(char buffer[], char* arg[] ,char delim[]){
    char* saveptr;
    char* s;
    int num = 0;
    
	// 첫 번째 명령어 token화
    s = strtok_r(buffer, delim, &saveptr);
    while(s) {
		// NULL 반환될 때까지 다음 명령어 token화 실행
        arg[num++] = s;
        s = strtok_r(NULL, delim, &saveptr);
    }
    
    arg[num] = (char *)0;
    
    return num;
}

