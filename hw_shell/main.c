/*
* This is homework 'my shell' : shell program
* @author : Donghwi Kim
* @student's ID : 32190483
* @date : 2022.12.08
*/

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h> 
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "myshell.h"

int main()
{
    char line[1024];

	printf("--------------- MY Shell Program --------------\n");
    printf("\t간단한 shell 프로그램입니다.\n");
    printf("\t아래는 명령어 사용법입니다.\n");
    printf("\texit\t: shell을 나갑니다\n");
    printf("\thelp\t: 도움말을 켜줍니다.\n");
	printf("\tcd\t: 디렉토리의 위치를 변경합니다.\n");
	printf("-----------------------------------------------\n");
	
	while(1)
    {             
		printf("%s $", get_current_dir_name()); //현재 디렉토리 위치를 출력해서 shell로 보이게 해주었다.
		fgets(line, sizeof(line) - 1, stdin); //입력한 문자열을 line에 저장하도록 함.
		
		if(!(strcmp(line, "exit\n"))) //exit를 입력하면 종료하도록
			break;
		if(!(strcmp(line, "help\n"))) 	//help를 입력하면 도움말이 나오도록
 			cmd_help(); 				//함수 호출해줌
		if (run(line) == false)  //run이 실패하면 함수를 빠져나오게 해줌
			break;
    }
    return 0;
}
