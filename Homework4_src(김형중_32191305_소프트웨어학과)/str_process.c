#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "str_process.h"

// 명령어(문자열)을 argument 단위로 parsing 후 배열에 삽입
// 인자로 명령어(문자열)과 parsing 결과가 저장될 문자열 배열의 포인터 전달
int parse(char* str, char*** tokens){
	// parsing된 명령어 인자 저장할 배열 선언
	char** parsed_args = (char**)malloc(sizeof(char*));
	char* ptr = strtok(str, " \n");  // 명령어를 인자 단위로 split
	int count = 0;
	
	while (ptr != NULL){
		// 기존에 할당된 메모리에 추가적으로 재할당
		parsed_args = (char**)realloc( parsed_args, sizeof(char*) * (count+2) );
		parsed_args[count++] = ptr;  // 배열에 parsing된 명령어 인자 저장
		ptr = strtok(NULL, " \n");
	}
	parsed_args[count] = NULL;

	*tokens = parsed_args;  // 인자로 전달된 배열에 parsing된 명령어 배열 전달
	return count;  // 명령어 인자 개수 반환
}

// 문자열 끝에 '/'를 추가한 문자열 반환
char* add_slash(char* str){
	int len = strlen(str)+2;  // 기존의 문자열에 '/'와 널문자를 고려해 문자열 크기 재설정
	char* res = (char*)malloc(sizeof(char) * len);
	int i;
	
	for(i=0; i<len-1; i++){
		if(str[i])
			res[i] = str[i];  // argument로 전달받은 문자열을 그대로 복사	
		else{
			res[i] = '/';  // 모두 복사되면 끝에 / 추가
		}
	}
	res[i] = '\0';  // 널문자를 추가해 문자열의 끝을 알림
	return res;
}

// 명령어가 위치한 경로를 찾기 위해 PATH 환경변수를 참조
// PATH의 내용을 파일 경로 형태로 parsing하여 배열에 입력
int set_environment_path(char*** arr){  // 2차원 배열의 포인터를 매개변수로 전달
	char** env_path = (char **)malloc(sizeof(char*));  // PATH의 값들을 저장할 문자열 배열
	char* path_value = getenv("PATH");  // PATH의 값을 문자열로 반환하는 getenv()함수 사용
	char* elem;
	int count = 0;
	
	elem = strtok(path_value, ":");  // ':'을 기준으로 split
	env_path[count] = add_slash(elem);  // 경로 끝에 '/'를 붙여준다.
	
	while(elem != NULL){  // PATH를 끝까지 읽을 때까지 반복
		elem = strtok(NULL, ":");
		if(elem == NULL)
			break;
		//기존에 할당된 메모리에 추가적으로 재할당
		env_path = (char **)realloc( env_path, sizeof(char*) * ((++count)+2) );
		env_path[count] = add_slash(elem);
	}
	*arr = env_path;  // 매개변수로 전달된 배열에 parsing된 PATH 배열 저장
	return count+1;  // PATH에 있는 목록 수 반환
}

// PATH변수와 명령어를 결합한 명령어의 절대주소를 찾는 함수
// 환경변수 배열과 parsing한 명령어 배열 전달
char* get_cmd_path(char **env_path, char** command){
	int i = 0;
	int c;
	char* cmd;
	char* path_of_cmd;
	struct stat	file_stat;  // 파일 정보 저장하는 구조체

	cmd = command[0];  // cmd변수에 명령어 저장
	if (!cmd) // 명령어 없이 엔터만 입력할 경우 NULL 반환.
		return (NULL);
	
	// PATH의 경로들과 하나하나 비교
	while (env_path && env_path[i])
	{
		// 문자열을 (PATH상 경로 + 명령어) 길이만큼 메모리 할당
		path_of_cmd = (char*)malloc(sizeof(char) * (strlen(env_path[i])+strlen(cmd)));
		strcpy(path_of_cmd, env_path[i]);
		strcat(path_of_cmd, cmd);
		
		//stat() 함수로 파일이 존재하는 확인
		if(stat(path_of_cmd, &file_stat) == 0){
			// directory일 경우 오류 반환.
			if (S_ISDIR(file_stat.st_mode)){
				printf("%s is not command, but directory\n", path_of_cmd);
				// exit(-1);
				path_of_cmd = NULL;
			}
			return path_of_cmd;
		}
		
		// 메모리 해제 후 다른 경로와 비교
		free(path_of_cmd);
		path_of_cmd = NULL;
		i++;
	}
	// 모든 PATH 경로에 명령어가 존재하지 않을 경우 NULL 반환
	return path_of_cmd;
}
