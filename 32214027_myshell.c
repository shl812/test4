/**
* MakeShell.c	:Make a Shell
* @author	:32214027 정규민
* @email	:32214027@dankook.ac.kr
* @version	:1.0
* @date		:2022. 12. 09
**/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

bool cd(int argc, char*argv[]);		//cd함수
bool quit(int argc, char*argv[]);	//quit함수
bool help(int argc, char*argv[]);	//help함수
struct Str {	//이름, 설명, 함수
	char*name;
	char*desc;
	bool (*func) (int argc, char*argv[]);
};
struct Str command[5] = {	//명령어 5개 명령어 호출, 설명, 함수
	{"cd", "Change the shell working directory.", cd},
	{"quit", "Quit Shell program", quit},
	{"help", "Display information about builtin commands.", help},
	{">", "Redirection"},
	{"&", "Background processing"}
};
bool cd(int argc, char*argv[]) {	//cd함수
	if(argc != 2) {
		printf("%s\n",argv[0]);
	}
	if(chdir(argv[1]) == -1) {
		printf("error\n");
	}
}
bool quit(int argc, char*argv[]) {		//quit함수
	return false;
}
bool help(int argc, char*argv[]) {		//help함수
	int i;
	printf("GNU bash, version 4.4.20(1)-release (x86_64-pc-linux-gnu)\n");
	printf("쉘 명령어는 내부적으로 정의되어 있습니다.  'help'를 입력하면 목록이 보입니다.\n");
	printf("`name' 함수에 대해 더 많은 것을 알아보려면 `help name' 을 입력하십시오.\n");
	for(i = 0; i < 5; i++) {
		printf("%s: %s", command[i].name, command[i].desc);
	}
	return true;
}
int tokenize(char*buf, char*delims, char*tokens[], int maxTokens) {
	int t_count =0;
	char * token;
	token = strtok(buf, delims);
	while(token != NULL && t_count < maxTokens) {
		tokens[t_count++] = token;
		token = strtok(NULL, delims);
	}
	tokens[t_count] = NULL;
	return t_count;
}
void cmd_redir(int argc, char* argv[]) {
    int fd;
    if (argc == 4) {
        fd = open(argv[3], O_WRONLY | O_CREAT, 0664);
        if (fd < 0) {
            printf("file open errro :%d\n", argv[3], errno);
            exit(-1);
        }
        dup2(fd, STDOUT_FILENO);
        argv[2] = NULL;
    }
    close(fd);
    return;
}
bool run(char *line) {
    char delims[]=" \n";
    int t_count;
    char *tokens[128];
    pid_t pid;
    int stat;
    int i;
    bool backp = false;
    bool redir = false;
    char* check_Backp = strchr(line, '&');
    char* check_Redir = strchr(line, '>');
    if (check_Backp != NULL) backp = true;
    if (check_Redir != NULL) redir = true;
    t_count = tokenize(line, delims, tokens, sizeof(tokens) / sizeof(char*));
    if (t_count == 0) return true;

    for (i = 0; i < 3; i++) {
        if (strcmp(command[i].name, tokens[0]) == 0)
            return command[i].func(t_count, tokens);
    }
    if ((pid = fork()) < 0) {
        perror("fork error");
        exit(-1);
    }
    else if (pid == 0) {
        if (redir == true) {
            cmd_redir(t_count, tokens);
        }
        execvp(tokens[0], tokens);
    }

    if (backp == false)
        waitpid(pid, &stat, 0);
    else
        waitpid(pid, &stat, WNOHANG);		

    return true;
}
int main() {
	char line[1024];

    while (1) {
        printf("MakeShell:%s#", get_current_dir_name());
        fgets(line, sizeof(line) - 1, stdin);
        if (run(line) == false)
            break;
    }

	return 0;
}