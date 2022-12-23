/*Hw#4 make simple shell, Lee Wonkyu (wonk0424@gmail.com) 
2021/11/04
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#define MAX 128

bool cmd_help(int argc, char* argv[]);
bool cmd_quit(int argc, char* argv[]);
bool cmd_cd(int argc, char* argv[]);
bool run(char *line);

struct shell {
	char* name;
	char* ex;
	bool (*intr_func)(int argc, char* argv[]);
};

struct shell command[6] = {
	{"help", "Show command", cmd_help},
	{"quit", "Quit Shell", cmd_quit},
	{"cd", "Change Directory", cmd_cd},
	{"|", "Pipe"},
	{">", "Redirection"},
	{"&", "Background Run"}
};

bool cmd_help(int argc, char* argv[]) {
	// 실행 후 어떤 기능 실행이 가능한지 안내
	int i;
	
	printf("=========== Shell ===========\n");
	printf(" simple shell made by jisun  \n");
	printf("=============================\n");
	// command 모두 출력
	for (i = 0; i < 6; i++) {
		printf("%s : %s\n", command[i].name, command[i].ex);
	}
	printf("=============================\n");
	return true;
}

bool cmd_quit(int argc, char* argv[]) {
	// 종료
	return 0;
}

bool cmd_cd(int argc, char* argv[]) {
	// change directory
	if (argc != 2) {
		// 인자 수가 2개가 아닐 경우 오류 출력
		printf("Usage : %s <path name>\n", argv[0]);
	}

	if (argc == 2) {
		if (chdir(argv[1])) {
			printf("Usage : wrong directory\n");
		}
	}
	return true;
}

void cmd_pipe(int argc, char* argv[]) {
	int read_size, fd[2], stat;
	pid_t pid;
	char bufc[MAX], bufp[MAX];
	
	if (argc != 4) {
		// 인자 수가 4개가 아닐 경우
		printf("Usage : %s <file name> | <file name> \n", argv[0]);
		return ;
	}
	
	if(pipe(fd) < 0){
		// 파이프 에러
		printf("Usage : Pipe Error \n"); 
		return;
	}
	
	if((pid = fork()) == 0){
		write(fd[1], argv[1], MAX);
		sleep(1);
		read_size = read(fd[0], bufc, MAX);
		bufc[read_size] = '\0';
		exit(0);
	}
	
	else {
		read_size = read(fd[0], bufp, MAX);
		bufp[read_size] = '\0';
		write(fd[1], argv[3], MAX);
		wait(&stat);
		close(fd[0]); 
		close(fd[1]);
	}
	
	return;
}

void cmd_redir(int argc, char* argv[]) {
	int fd;
	
	if (argc != 4) {
		printf("Usage : %s input > output\n", argv[0]);
		return;
	}
	
	if (argc == 4) {
        fd = open(argv[3], O_WRONLY | O_CREAT, 0664);
        if (fd < 0) {
            printf("Usage : file open errro : %d\n", argv[3], errno);
            exit(-1);
        }
		
        dup2(fd, STDOUT_FILENO);
        argv[2] = NULL;
    }
	close(fd);
	return;
}

int cmd_check(char *line[]) {
	int i;
	
	for (i = 0; line[i] != NULL; i++) {
		if (!strcmp(line[i], "|")) {
			// pipe
			line[i] = NULL;
			return 1;
		}
		
		if (!strcmp(line[i], ">")) {
			// redirection
			line[i] = NULL;
			return 2;
		}
		
		if (!strcmp(line[i], "&")) {
			// background process
			line[i] = NULL;
			return 3;
		}
	}
	return 0;
}

int tokenize(char* buf, char* delims, char* tokens[], int max_tokens) {
	// parsing
	int cnt = 0;                               
    char* token = strtok(buf, delims);

    while (token != NULL && cnt < max_tokens) {
        tokens[cnt] = token;
		cnt++;
        token = strtok(NULL, delims);
    }
	
    tokens[cnt] = NULL;
    return cnt;
}

bool cmd_run(char* line) {
	char delims[] = " \n";
    char *tokens[MAX];
    pid_t pid;
	
    int i, stat, check;
	// 토큰화하기
	int cnt = tokenize(line, delims, tokens, sizeof(tokens) / sizeof(char*));
	
	if(cnt == 0) {
		return true; 
	}
	
	// 명령어 확인하기
	check = cmd_check(tokens); 
	for(i = 0; i < 3; i++) { 
        if(strcmp(command[i].name, tokens[0]) == 0)
        	return command[i].intr_func(cnt, tokens);
    }
	
	
	if((pid=fork()) < 0) {
		perror("Fork error\n"); 
		exit(-1);
	}
	
	else if(pid == 0) {
		if(check == 1){  // pipe 실행할 경우
			cmd_pipe(cnt, tokens);
		}
		
		else if(check == 2) {	// redirection 실행할 경우
			cmd_redir(cnt, tokens);
			execvp(tokens[0], tokens);  
		}
		
		else{  // 기본 명령어 실행할 경우
			execvp(tokens[0], tokens);
			printf("execute failed\n"); 
			exit(-1);
		}
	}
	else {
		if(check == 3) {  // background 수행
			waitpid(pid, &stat, WNOHANG); 
		}
		else waitpid(pid, &stat, 0);
	}
	return true;
}

int main() {
	char line[1024];
	
	while(1) {
		printf("%s : JS's shell @@", get_current_dir_name());
		fgets(line, sizeof(line) - 1, stdin);
	
		if (cmd_run(line) == false) {
			break;
		}
	}
	return 0;
}
