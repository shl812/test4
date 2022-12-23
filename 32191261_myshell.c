/* Implement Shell with C
* author	: HanYi Kim
* email		: h_ani99@naver.com
* version	: 1.0
**/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

bool cmd_cd(int argc, char* argv[]);    // 나중에 쓸 함수들을 미리 전방선언
bool cmd_help(int argc, char* argv[]);
bool cmd_quit(int argc, char* argv[]);

struct cmdForm {                        // 구조체로 명령어 형식을 만듬
	char* name;
	char* exp;
	bool (*func)(int argc, char* argv[]);
};

struct cmdForm cmds[5] = {            // 구조체로 만든 명령어
	{"cd","Change Directory",cmd_cd},
	{"help","Show Command Help",cmd_help},
	{"quit","Quit",cmd_quit},
	{">","Redirection"},
	{"&","Background Processing"}
};

bool cmd_cd(int argc, char* argv[]) {
	if (argc == 2) {
		if (chdir(argv[1])) printf("%s is no directory\n", argv[1]);
	}
	else printf("USAGE : cd directory_name\n");
	return true;
}

bool cmd_help(int argc, char* argv) {
	int i;
	printf("---------------------------------------\n");
	printf("Show command\n");
	for (i = 0; i < sizeof(cmds) / sizeof(cmds[0]); i++) {
		printf("%s : %s\n", cmds[i].name, cmds[i].exp);
	}
	printf("---------------------------------------\n");
	return true;
}

bool cmd_quit(int argc, char* argv[]) {
	return false;
}

bool cmd_redir(int argc, char* argv[]) {
	int fd;
	if (argc == 4) {
		fd = open(argv[3], O_RDWR | O_CREAT, 0664);
		if (fd < 0) {
			printf("Can't open %s file with errno %d\n", argv[3], errno);
			exit(-1);
		}
		dup2(fd, STDOUT_FILENO);
		argv[2] = NULL;
	}
	close(fd);
	return;
}

int tokenize(char* buf, char* delims, char* tokens[], int maxTokens) {
	int cnt = 0;
	char* token;
	token = strtok(buf, delims);            // 첫번째 토큰을 얻어옴
	while (token != NULL && cnt < maxTokens) {
		tokens[cnt++] = token;
		token = strtok(NULL, delims);        // 차례대로 토큰 얻어옴,찾은 토큰 바로뒤에 NULL 붙기 때문에 NULL 입력
	}
	tokens[cnt] = NULL;
	return cnt;
}

bool run(char* line) {
	char delims[] = " \n";
	char* tokens[100];
	pid_t child;
	int stat, token_cnt;
	int i;

	bool backPro = false;
	bool reDir = false;

	char* back_check = strchr(line, '&');
	char* redir_check = strchr(line, '>');

	if (back_check != NULL) backPro = true;
	if (redir_check != NULL) reDir = true;

	token_cnt = tokenize(line, delims, tokens, sizeof(tokens) / sizeof(char*));

	if (token_cnt == 0) return true;
	if (backPro == true) tokens[token_cnt - 1] = NULL;
	for (i = 0; i < 3; i++) {           
		if (strcmp(cmds[i].name, tokens[0]) == 0)     // strcmp를 통해 두 문자열 비교 / 완전히 같다면 0을 리턴한다.
			return cmds[i].func(token_cnt, tokens);   // tokens를 벡터로 전달

	}
	if ((child = fork()) < 0) {
		perror("fork error\n");
		exit(-1);
	}
	else if (child == 0) {
		if (reDir == true) {
			cmd_redir(token_cnt, tokens);
		}
		execvp(tokens[0], tokens);
	}
	if (backPro == false)
		wait(&stat);
	return true;
}

int main() {
	char line[1024];
	while (1) {
		printf("%s $", get_current_dir_name());
		fgets(line, sizeof(line) - 1, stdin);
		if (run(line) == false) break;
	}
	return 0;
}
