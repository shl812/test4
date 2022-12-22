/**
 * 시스템 프로그래밍 SW 1분반 Homework04
 * 32202970_Yoon_Yejin_Shell.c : 리눅스 쉘 구현(리다이렉션, 파이프, 백그라운드 프로세싱) 프로그램
 * @ author		: 32202970 윤예진
 * @ date		:  22-12-10
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#define MAX_BUF 250
#define MAX_TOKEN 5

void process_command();
int break_command(char* command, char** tokens);
void redirection(char* command, char* delim);
void pipe_process(char* command, char* delim);
void print_error(int error_number);
void cd(char** tokens, int token_count);

//커맨드 처리 함수 실행
int main()
{
	process_command();
}


/* < process_command >
* @desc : 명령어를 입력받고 실행, exit를 입력하면 종료
* @param : void
* @return : void
*/
void process_command()
{
	char path[MAX_BUF] = { 0, };			// 현재 디렉토리
	char command[MAX_BUF] = { 0, };			// 명령어
	char* tokens[MAX_TOKEN] = { 0, };		// 명령어 토큰
	char* delim = NULL;						// 리다이렉션 문자
	char flag_bgr = -1;						// 백그라운드 프로세싱 여부 플래그
	int token_count;						// 토큰의 개수
	int pid;								// pid
	
	
	/* 프롬포트 출력 및 기본옵션 커맨드(파이프, 백그라운드 프로세싱, 리다이렉션x) 처리*/
	while (1)
	{
		delim = NULL;
		flag_bgr = -1;
		
		// 커맨드 및 토큰 버퍼 초기화
		for(int i=0; i<MAX_TOKEN; i++)
			tokens[i] = 0;
	
		for(int i=0; i<MAX_BUF; i++)
			command[i] = 0;
		
		getcwd(path, MAX_BUF);				//현재 디렉토리 획득
		printf("%s >>> ", path);			//프롬포트 출력
		
		// 입력이 없다면 continue
		if (fgets(command, MAX_BUF, stdin) == NULL)
			continue;
		
		// 리다이렉션 및 파이프 문자 처리
		if(strstr(command, ">") != 0)
			delim = ">";
		else if(strstr(command, ">>") != 0)
			delim = ">>";
		else if(strstr(command, "<") != 0)
			delim = "<";
		else if(strstr(command, "<<") != 0)
			delim = "<<";
		// 파이프 문자일 경우, 파이프 프로세스 함수 호출
		else if(strstr(command, "|") != 0) {
			delim = "|";
			pipe_process(command, delim);
			continue;
		}

		// 리다이렉션 문자가 포함되었을 시, 리다이렉션 함수 호출
		if(delim != NULL) {
			redirection(command, delim);
			continue;
		}
		
		// 명령어 토큰화 작업, 토큰 개수를 리턴 받음
		token_count = break_command(command, tokens);
		
		// 백그라운드 프로세스 여부
		if(strcmp(tokens[token_count-1], "&") == 0) {
			flag_bgr = 1;
			tokens[token_count-1] = 0;
			token_count--;
		}
		

		// exit
		if (strcmp(tokens[0], "exit") == 0)
			exit(0);
		// cd
		else if (strcmp(tokens[0], "cd") == 0)
			cd(tokens, token_count);
		// 외부 동작 명령어의 경우
		// 자식 프로세스를 만들어 커맨드 실행
		else if (tokens[0] != NULL) {
			//자식 프로세스
			if ((pid = fork()) == 0) {
				// 백그라운드 프로세싱의 경우
				if(flag_bgr>0) {
					execvp(tokens[0], tokens);
					exit(0);
				}
				else {
					execvp(tokens[0], tokens);
				}
			}
			else if (pid > 0 && flag_bgr < 0) wait(&pid);
			//프로세스 생성에 실패 시, 에러 메세지 출력
			else if (pid < 0) print_error(3);
		}

	}
}


/* < break_command >
* @desc : 입력받은 커맨드를 토큰으로 나누고 기타 처리를 하는 함수
* @param : command(커맨드, \n포함하여 읽음), tokens(명령어 토큰을 저장할 배열)
* @return : token_count(토큰 개수)
*/
int break_command(char* command, char** tokens)
{
	//fgets를 통해 커맨드를 입력 받았으므로 \n -> \0 처리
	char len = strlen(command);
	if(command[len-1]=='\n') command[len-1] = 0;
	
	//공백을 기준으로 토큰화, 토큰 개수 카운트
	char* token = NULL;
	tokens[0] = strtok(command, " ");

	int token_count = 1;
	while ((token = strtok(NULL, " ")) != NULL)
	{
		tokens[token_count] = token;
		token_count++;
	}
	
	//토큰 개수 리턴
	return token_count;
}


/* < print_error >
* @desc : 에러 번호에 따라 에러를 출력하는 함수
* @param : error_number(에러 번호)
* @return : void
*/
void print_error(int error_number)
{
	//명령어 실행 불가능할 경우
	if (error_number == 0)
		printf("Invalid command or not executable file !\n");
	//인수가 부족할 경우
	else if(error_number == 1)
		printf("Less parameter!\n");
	//인수가 초과되었을 경우
	else if(error_number == 2)
		printf("Too much command!\n");
	//프로세스 생성에 실패하였을 경우
	else if (error_number == 3)
		printf("Failed create new process!\n");
}


/* < cd >
* @desc : cd 명령어, 프로세스 생성하지 않고 내부 동작으로 처리
* @param : tokens, token_count
* @return : void
*/
void cd(char** tokens, int token_count)
{
	char path[MAX_BUF] = { 0, };
	int path_len;
	
	//명령어 포함 인수가 3개 이상인 경우, 에러메세지 출력
	if (token_count > 2) {
		print_error(2);
		return;
	}
	
	//명령어만 들어왔을 경우, 경로 이동하지 않고 현재 경로만 출력
	if (token_count == 1) {
		getcwd(path, MAX_BUF);
		printf("%s\n", path);
		return;
	}
	//명령어 이동경로 가 들어왔을 경우
	else {
		//strcat을 통해 토큰(경로) 결합
		for (int i = 1; i < token_count; i++)
		{
			strcat(path, tokens[i]);
			path_len = strlen(path);
			path[path_len] = ' ';
			path[path_len + 1] = '\0';
		}
		//이동할 수 없다면(경로가 잘못되었다면)
		if (chdir(path) == -1)
			printf("%s is invalid path!\n", path);
	}
}

/* < redirection >
* @desc : 리다이렉션을 처리하는 함수. 명령어 토큰화, 명령어 실행을 수행한다.
* @param : command, delim(<, <<, >, >>)
* @return : void
*/
void redirection(char* command, char* delim)
{
	// delim 문자 기준 왼쪽 명령어와 해당 명령어를 토큰화한 것
	char* left = NULL;
	char* left_tokens[MAX_TOKEN]= {0, };
	int left_tokenCount;
	
	// delim 문자 기준 오른쪽 명령어와 해당 명령어를 토큰화한 것
	char* right = NULL;
	char* right_tokens[MAX_TOKEN]= {0, };
	int right_tokenCount;
	
	char** src = NULL;				// 입력에 해당하는 명령어
	char** dest = NULL;				// src를 받아 동작하는 명령어
	
	int fd = -1;
	int pid = -1;
	char flag_bgr = -1;				// 백그라운드 프로세싱 여부
	
	left = strtok(command, delim);
	right = strtok(NULL, delim);
	
	left_tokenCount = break_command(left, left_tokens);
	right_tokenCount = break_command(right, right_tokens);
	
	// 백그라운드 프로세싱 여부
	if(strcmp(right_tokens[right_tokenCount-1], "&") == 0) {
		flag_bgr = 1;
		right_tokens[right_tokenCount-1] = 0;
		right_tokenCount--;
	}
	
	/* 프로세스의 결과(src)가 파일(dest)로 리다이렉션 하는 경우
	*  file < process / file << process / process > file / process >> file
	*  (파일에 해당하는 명령어의 경우, 하나만 들어온 경우를 가정)
	*/
	if((left_tokenCount == 1 && (strcmp(delim, "<") == 0 || strcmp(delim, "<<") == 0 )) ||
	   (right_tokenCount == 1 && (strcmp(delim, ">") == 0 || strcmp(delim, ">>") == 0 )))
	{
		if(strcmp(delim, ">") == 0 || strcmp(delim, ">>") == 0 ) {
			src = left_tokens;
			dest = right_tokens;
		}
		else {
			src = right_tokens;
			dest = left_tokens;
		}
		
		// 파일 끝에서부터 쓰기 여부
		if(strcmp(delim, ">>") == 0 || strcmp(delim, "<<") == 0)
			fd = open(dest[0], O_WRONLY, O_APPEND);
		else
			fd = open(dest[0], O_WRONLY);
		
		// 파일 열기에 실패할 경우
		if(fd<0) {
			printf("%s file or directory not found!\n", dest[0]);
			return;
		}
		
		// 자식 프로세스
		if ((pid = fork()) == 0) {
			dup2(fd, STDOUT_FILENO);				// 파일의 fd를 표준 출력에 덮어쓰기

			// 백그라운드 프로세싱이라면
			if(flag_bgr > 0) {
				printf("Back\n");
				execvp(src[0], src);
				exit(0);
				}
			else {
				execvp(src[0], src);
			}
		}
		else if (pid > 0 && flag_bgr < 0) wait(&pid);
		
		//프로세스 생성에 실패 시, 에러 메세지 출력
		else if (pid < 0) print_error(3);
		
	}
	/* 파일(src)이 프로세스(dest)의 입력 리다이렉션 하는 경우
	*  file > process / file >> process / process < file / process << file
	*  (파일에 해당하는 명령어의 경우, 하나만 들어온 경우를 가정)
	*/
	else if((left_tokenCount == 1 && (strcmp(delim, ">") == 0 || strcmp(delim, ">>") == 0 )) ||
	   (right_tokenCount == 1 && (strcmp(delim, "<") == 0 || strcmp(delim, "<<") == 0 )))
	{
		if(strcmp(delim, ">") == 0 || strcmp(delim, ">>") == 0 ) {
			src = left_tokens;
			dest = right_tokens;
		}
		else {
			src = right_tokens;
			dest = left_tokens;
		}
		
		// 파일 끝에서부터 쓰기 여부
		if(strcmp(delim, ">>") == 0 || strcmp(delim, "<<") == 0)
			fd = open(src[0], O_RDONLY, O_APPEND);
		else
			fd = open(src[0], O_RDONLY);
		
		// 파일 열기에 실패할 경우
		if(fd<0) {
			printf("%s file or directory not found!\n", src[0]);
			return;
		}
		
		// 자식 프로세스
		if ((pid = fork()) == 0) {
			dup2(fd, STDIN_FILENO);				// 파일의 fd를 표준 입력에 덮어쓰기

			// 백그라운드 프로세싱이라면
			if(flag_bgr>0) {
				execvp(dest[0], dest);
				exit(0);
			}
			else
				execvp(dest[0], dest);			
		}
		else if (pid > 0 && flag_bgr < 0) wait(&pid);
		//프로세스 생성에 실패 시, 에러 메세지 출력
		else if (pid < 0) print_error(3);
	}
}


/* < pipe_process >
* @desc : 파이프를 처리하는 함수. 명령어 토큰화, 명령어 실행을 수행한다.
* @param : command, delim(|, 파이프 문자)
* @return : void
*/
void pipe_process(char* command, char* delim)
{
	// 파이프 문자 기준 왼쪽 명령어와 해당 명령어를 토큰화한 것
	char* left = NULL;
	char* left_tokens[MAX_TOKEN]= {0, };
	int left_tokenCount;
	
	// 파이프 문자 기준 왼쪽 명령어와 해당 명령어를 토큰화한 것
	char* right = NULL;
	char* right_tokens[MAX_TOKEN]= {0, };
	int right_tokenCount;
	
	int fd[2]= {-1, -1};
	int pid = -1;
	
	left = strtok(command, delim);
	right = strtok(NULL, delim);
	
	left_tokenCount = break_command(left, left_tokens);
	right_tokenCount = break_command(right, right_tokens);
	
	// 파이프 실행에 실패한 경우
	if(pipe(fd) < 0) {
		printf("Pipe failed!\n");
		return;
	}

	//부모 프로세스
	if ((pid = fork()) > 0) {
		//exit
		if (strcmp(left_tokens[0], "exit") == 0)
			exit(0);
		//cd
		else if (strcmp(left_tokens[0], "cd") == 0)
			cd(left_tokens, left_tokenCount);
		// 외부 동작 명령어의 경우
		// 표준 출력을 파이프 출력으로 덮어쓰고, 자식 프로세스를 만들어 커맨드 실행
		else if(left_tokens[0] != NULL) {
			dup2(fd[1], STDOUT_FILENO);
			execvp(left_tokens[0], left_tokens);
			wait(&pid);
		}
	}
	//자식 프로세스
	else if(pid == 0) {
		//ext
		if (strcmp(right_tokens[0], "exit") == 0)
			exit(0);
		//cd
		else if (strcmp(right_tokens[0], "cd") == 0)
			cd(right_tokens, right_tokenCount);
		// 외부 동작 명령어의 경우
		// 표준 입력에 파이프의 입력을 가져오고, 커맨드 실행
		else if(right_tokens[0] != NULL) {
			dup2(fd[0], STDIN_FILENO);
			execvp(right_tokens[0], right_tokens);
		}
	}
	//프로세스 생성에 실패 시, 에러 메세지 출력
	else if (pid < 0) print_error(3);
}
