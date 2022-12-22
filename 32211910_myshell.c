/**
 * myshell.c	: shell program
 * @author		: 32211910 Chunho Park
 * @email		: chunho5012@naver.com
 * @date		: 2022.12.04.
**/

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

// buf의 값을 delims 기준으로 분리하여 tokens[]에 저장
int tokenize(char* buf, char* delims, char* tokens[], int maxTokens)	
{
	char* token = strtok(buf, delims);
	int tokenCount = 0;
	
	while(token != NULL && tokenCount < maxTokens)
	{
		// fgets()는 개행 문자까지 읽음
		// 문자열 뒤의 개행 문자 제거
		int len = strlen(token);
		if (len > 0 && token[len - 1] == '\n') 
		{
			token[len - 1] = '\0';
		}
		
		tokens[tokenCount++] = token;
		token = strtok(NULL, delims);
	}
	
	tokens[tokenCount] = '\0';	// 매개변수 마지막에 NULL 추가
	
	return tokenCount;
}

int cd(char* filePath)		// filePath 값으로 작업 경로 변경
{
	if(chdir(filePath) == 0)
		return 1;
	
	return 0;
}

int run(char* line)
{
	char* tokens[10];
	int tokenCount = tokenize(line, " ", tokens, sizeof(tokens) / sizeof(char*));	// 공백 단위로 명령어 분할
	
	// exit 입력 시 종료
	if (strcmp(tokens[0], "exit") == 0)
		return 0;
	
	// cd 명령어
	if (strcmp(tokens[0], "cd") == 0)
	{
		if(tokenCount != 2)
		{
			printf("USAGE : cd file_path\n");
			return 1;
		}
		
		if (cd(tokens[1]) == 0)
			printf("경로를 찾을 수 없습니다.\n");
		return 1;
	}
	
	// 아무 것도 입력하지 않을 시 아무런 처리하지 않음
	if (tokens[0][0] == '\0')
		return 1;
	
	pid_t child;
	int background = 0, redirection = 0;
	
	// 명령어 마지막에 &가 있다면 백그라운드 실행
	if(tokens[tokenCount - 1][0] == '&')
		background = 1;
	
	// > 나 >> 가 입력되었는지 확인
	int i, idx = 0;
	for(i = 0; tokens[i]; ++i)
	{
		if(tokens[i][0] == '>')
			idx = i;
	}
	
	// > 나 >> 는 뒤에서 두번째 토큰에 있어야 함
	// ex) cat hello.txt > hello2.txt
	if(i - idx == 2 && idx > 0)
	{
		if(strcmp(tokens[idx], ">") == 0)
		{
			redirection = 1;
			tokens[idx] = NULL;
		}
		else if(strcmp(tokens[idx], ">>") == 0)
		{
			redirection = 2;
			tokens[idx] = NULL;
		}
	}
	
	int fd, stdout;
	if(redirection != 0)
	{
		if(redirection == 1)	// redirection == 1 : > 입력
			fd = open(tokens[idx + 1], O_WRONLY | O_CREAT, 0641);
		else					// else -> redirection == 2 : >> 입력
			fd = open(tokens[idx + 1], O_WRONLY | O_CREAT | O_APPEND, 0641);
		dup2(STDOUT_FILENO, stdout); 	// stdout에 STDOUT_FILENO을 덮어씌우고
		dup2(fd, STDOUT_FILENO);		// STDOUT_FILENO에 fd를 덮어씌움
	}
	
	if ((child = fork()) == 0)
	{
		// 내장된 명령어 사용
		if (execvp(tokens[0], tokens) < 0)
		{
			printf("명령어가 존재하지 않습니다.\n");
			exit(0);
		}
	}
	
	if(redirection != 0)	// 명령어 수행이 끝나면 STDOUT_FILENO을 저장했던 stdout을 다시 STDOUT_FILENO에 덮어씌움
		dup2(stdout, STDOUT_FILENO);
	
	if(background == 0)		// 백그라운드 실행의 경우 자식 프로세스가 끝나길 기다리지 않음
		wait();
	
	return 1;
}

int main()
{
	char line[1024];
	while(1)
	{
		printf("%s$", get_current_dir_name());		// 현재 디렉토리 출력
		fgets(line, sizeof(line) - 1, stdin);
		if(run(line) == 0)
			break;
	}
}
