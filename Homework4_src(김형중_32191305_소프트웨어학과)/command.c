#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>

#include "command.h"
#include "inner_cmd.h"
#include "str_process.h"

extern char** environ;

// tokens[0]에는 명령어. get_cmd_path로 명령어 위치 구하고 execve에 넣고, tokens[0], env
int execute(char* command, char** env){
	char** tokens;  // command를 argument단위로 parsing하여 tokens에 배열 형태로 저장
	char** temp;  // redirection시 재설정되는 인자를 담는 배열
	char* cmd_path;  // 명령어의 경로를 나타내는 문자열
	char* redir[3] = {">", "<", ">>"};  // redirection 명령어 배열
	
	
	pid_t pid;
	int args_count;
	int status;
	int fd;
	int back = 0;  // 백그라운드 실행 여부
	int red = 0;  // redirection 실행 여부
	
	int i = 0;
	int slice = 0;  // tokens에서 redirection 명령어가 위치한 인덱스
	
	args_count = parse(command, &tokens);
	
	// 명령어를 입력안하고 enter만 쳤을 경우 무시
	if(!tokens[0]){
		free(tokens);
		return 0;
	}
	
	// 입력이 명령어인지 파일인지 확인
	if((fd = open(tokens[0], O_RDONLY)) > 0){
		close(fd);
		cmd_path = tokens[0];
	}
	// cd(shell built-in) 명령어인지 확인
	else if(!strcmp(tokens[0], "cd")){
		change_dir(tokens, args_count);
		free(tokens);
		return 0;
	}
	// 둘 다 아니면 명령어 절대주소 찾기
	else
		cmd_path = get_cmd_path(env, tokens);
	
	// 명령어를 못찾을 때 공백이 아니면 에러 출력
	if(!cmd_path){
		printf("myshell: command not fount: %s\n", tokens[0]);
		free(tokens);
		return 0;
	}
	
	//---------------------------------------------
	// redirection, 혹은 background 옵션 존재하면 red/back 참(1)으로 설정
	for(i=0; i < args_count; i++){
		if(!strcmp(tokens[i], ">") || !strcmp(tokens[i], ">>") || !strcmp(tokens[i], "<")){
			slice = i;
			red = 1;
			break;
		}
		else if (!strcmp(tokens[i], "&")){
			back = 1;
			break;
		}
	}
	//---------------------------------------------
	// 자식 프로세스 생성
	if((pid=fork()) < 0)
		perror("failed to fork\n");
	
	// 자식 프로세스에서 명령어 프로그램 exec 함수로 실행
	else if(pid == 0){
		// redirection시 표준 입출력 변경
		if(red)
			redirection(tokens, i);
		
		// redirection 명령어 전까지로 명령어 인자 재설정
		// e.g. "echo test > file"의 인자로 구성된 배열을 "echo test"의 인자들로 재설정
		// >와 file을 인자 배열에서 pop하여 재구성
		if(slice != 0){
			// 인자에서 redirection 명령어와 파일을 제외한 만큼의 크기로 temp에 메모리 할당
			temp = (char**)malloc(sizeof(char*) * (args_count-2));
			
			// temp의 모든 요소 null로 초기화
			i=0;
			while(temp[i]){
				temp[i] = '\0';
				i++;
			}
			
			// redirection 명령어와 그 뒤에 나올 file을 제외한 나머지를 temp에 할당
			for(i=0; i<args_count; i++){
				if (i==slice || i==(slice+1))
					continue;
				
				if(i > slice && (i-2)<(args_count-2))
					temp[i-2] = tokens[i];
			
				else{
					if (i<args_count-2)
						temp[i] = tokens[i];
				}
			}
			// tokens 메모리 해제 후 temp로 execve 함수 실행
			free(tokens);
			execve(cmd_path, temp, environ);
		}
		else
			execve(cmd_path, tokens, environ);
	}
	
	// 부모 프로세스에서 자식프로세스의 백그라운드 실행 여부에 따라 행동 결정
	else{
		if(back)
			// 자식 프로세스가 종료되지 않아도 block되지 않고 바로 return
			waitpid(pid, &status, WNOHANG);
		else
			// 자식 프로세스가 종료될 때까지 기다림
			waitpid(pid, &status, 0);
	}
	free(tokens);
	return 0;
}
