/**
 * myshell : shell program works as conventional bash
 * @author : 김형중 32191305 소프트웨어학과
 * @class  : 시스템 프로그래밍(SW) 2분반
 * @email  : gudwnd1113@naver.com
 * @version: 1.0
 * date    : 2022.11.30
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "command.h"
#include "str_process.h"

int main(){
	char command[128];
	char** env;
	int path_count;
	int i;	
	char* line;
	
	// 프로그램 실행시 시간을 출력하기 위한 구조체 선언
	time_t timer;
	struct tm* t;
	timer = time(NULL);
	t = localtime(&timer);
	
	printf("\t/**\n\
	* myshell : shell program works as conventional shell\n\
	* @author : 김형중 32191305 소프트웨어학과\n\
	* @class  : 시스템 프로그래밍(SW) 2분반\n\
	* @email  : gudwnd1113@naver.com\n\
	* @version: 1.0\n\
	* date    : %d.%d.%d\n\
	**/\n\n", t->tm_year+1900, t->tm_mon+1, t->tm_mday);
	
	path_count = set_environment_path(&env);
	// exit를 입력할 때까지 명령어를 계속 처리
	while(1){
		printf("%s $ ", get_current_dir_name());
		fgets(command, sizeof(command)-1, stdin);
		if(!strcmp(command, "exit\n")){
			break;
		}
		line = command;
		execute(line, env);
	}
	
	// 동적할당한 환경변수 2차원 배열 메모리 해제
	for(i=0; i<path_count; i++)
		free(env[i]);
	free(env);
	
	return 0;
}
