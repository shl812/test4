/*
* mysh.c: implement function of shell 
* @author: Jinmyung Jeon
* @Student's ID: 32214003
* @email: jmj32214003@dankook.ac.kr
* date: 2022.12.09
*/

#include <stdio.h>						
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/wait.h>

//함수 선언
bool cmd_cd(int argc, char* argv[]);				
bool cmd_quit(int argc, char* argv[]);
bool cmd_help(int argc, char* argv[]);

//command structure
struct myShell {						
	char* short_name;
	char* long_name;
	bool (*func) (int argc, char* argv[]);			// 함수 포인터
};


struct myShell command[] = {
		{"cd", "change directory", cmd_cd},
		{"quit", "quit this shell", cmd_quit},
		{"help", "show this help", cmd_help},
	{">", "redirection"},
	{"&", "background processing"}
};

// cd 명령어를 수행하는 함수
bool cmd_cd(int argc, char* argv[]) {				
	if (argc != 2) {		//인자 2개 아니면 오류메세지 출력
        printf("Usage: %s <path name>\n", argv[0]);
    }
    if (chdir(argv[1]) == -1) {
        printf("failed... \n");
    }
	return true;
}

// quit 명령어를 수행하는 함수
bool cmd_quit(int argc, char* argv[]) {				
	return false;
}

// help 명령어를 수행하는 함수
bool cmd_help(int argc, char* argv[]) {				
	int i;
	printf("/**********Simple Shell(created by. Jinmyung Jeon)***********/\n");
	printf("You can use it just as the conventional shell\n");
	printf("Some examples of the commands\n");
	for (i = 0; i < sizeof(command) / sizeof(command[0]); ++i) {
		if (argc == 1 || strcmp(command[i].short_name, argv[1]) == 0)
			printf("%-7s: %s\n", command[i].short_name, command[i].long_name);
	}
	printf("/************************************************************/\n");
	return true;
}

// redirection ('>')
void cmd_redirection(int x, int argc, char* argv[]) {		
	int fd;
	if (argc == 3) {
		fd = open(argv[x + 2], O_WRONLY | O_CREAT, 0664);
		if (fd < 0) {
			printf("Can't open %s file with errno %d\n", argv[x + 2], errno);
			exit(-1);
		}
		dup2(fd, STDOUT_FILENO);
		argv[x + 1] = NULL;

	}
	else {
		fd = open(argv[x + 3], O_WRONLY | O_CREAT, 0664);
		if (fd < 0) {
			printf("Can't open %s file with errno %d\n", argv[x + 3], errno);
			exit(-1);
		}
		dup2(fd, STDOUT_FILENO);	//fd에 표준출력, 복사본 만들기
		argv[x + 2] = NULL;
	}
	close(fd);
	return;
}

// redirection과 background 여부 체크하는 함수
void check(char* line, bool* back, bool* redirection) {	
	//문자열 속에 '>','&' 있는지 확인
	char* check_redirection = strchr(line, '>');			
	char* check_back = strchr(line, '&');
	if (check_redirection != NULL)
		*redirection = true;
	if (check_back != NULL)
		*back = true;
	return;
}

int tokenize(char* buf, char* delims, char* tokens[], int maxTokens) {
	int token_count = 0;
	char* token;
	token = strtok(buf, delims);

	while (token != NULL && token_count < maxTokens) {
		tokens[token_count++] = token;
		token = strtok(NULL, delims);			// 다음 token 가져오기
	}
	tokens[token_count] = NULL;				// 마지막 token NULL

	return token_count;
}

bool run(char* line)
{
	char delims[] = " \n";
	char* tokens[128];
	int token_count;
	int i;
	int status;
	pid_t pid;

	bool back = false;
	bool redirection = false;
	int x;
	check(line, &back, &redirection);			// redirection과 backround 체크

	token_count = tokenize(line, delims, tokens, sizeof(tokens) / sizeof(char*));
	if (token_count == 0)
		return true;

	for (i = 0; i < sizeof(command) / sizeof(command[0]); ++i) {
		if (strcmp(command[i].short_name, tokens[0]) == 0)	//명령어와 첫번째 토큰이 일치하는 경우
			return command[i].func(token_count, tokens); 
	}

	if ((pid = fork()) < 0) {	//fork()의 리턴 값이 0보다 작으면 에러
		perror("fork error");
		exit(-1);
	}
	else if (pid == 0) {
		if (redirection == true) {				// redirection인 경우
			x = 0;
			cmd_redirection(x, token_count, tokens);
		}
		execvp(tokens[0], tokens);
		printf("No such file\n");
		exit(-1);
	}
	else {
		if (back == false)
			waitpid(pid, &status, 0);
		else						// background('&')인 경우
			waitpid(pid, &status, WNOHANG);		// 부모 프로세스가 기다리지 않는다
	}

	return true;

}

int main(void)
{
	char line[1024];
	while (1) {
		printf("[ %s ]/Jinmyung $ ", get_current_dir_name());
		fgets(line, sizeof(line) - 1, stdin);
		if (run(line) == false)
			break;
	}

	return 0;
}
