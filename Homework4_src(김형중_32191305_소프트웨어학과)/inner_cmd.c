#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "inner_cmd.h"
#define MAX_BUF 64

void change_dir(char** tokens, int count){
	// cd만 입력 시 HOME directory로 이동
	if(count==1)
		chdir(getenv("HOME"));
	// cd [path] 입력시 해당 path로 이동
	else if(count==2)
		chdir(tokens[1]);
	else 
		perror("ERROR\n");
}

// e.g. test1.txt > test2.txt
// 명령어 토큰과 redirection 명령어가 위치한 index를 인자로 받음
void redirection(char** tokens, int index){
	int fd, read_size, write_size;
	
	// >의 경우 file decriptor open 시 truncate 옵션 추가 후 표준 출력 변환
	if(!strcmp(tokens[index], ">")){
		if( (fd = open(tokens[index+1], O_RDWR | O_TRUNC | O_CREAT, 0614)) < 0 ){
			printf("fail to open %s with errno %d\n", tokens[index+1], errno);
			exit(-1);
		}
		dup2(fd, STDOUT_FILENO);
		close(fd);
	}

	// <의 경우 file decriptor open 시 append 옵션 추가 후 표준 출력 변환
	else if(!strcmp(tokens[index], ">>")){
		if( (fd = open(tokens[index+1], O_RDWR | O_APPEND | O_CREAT, 0614)) < 0 ){
			printf("fail to open %s with errno %d\n", tokens[index+1], errno);
			exit(-1);
		}
		dup2(fd, STDOUT_FILENO);
		close(fd);
	}
	
	// <의 경우 file decriptor open 시 truncate 옵션 추가 후 표준 입력 변환
	else if(!strcmp(tokens[index], "<")){
		if( (fd = open(tokens[index+1], O_RDONLY)) < 0){
			printf("fail to open %s with errno %d\n", tokens[index+1], errno);
			exit(-1);
		}
		dup2(fd, STDIN_FILENO);
		close(fd);
	}
}
