/*
 * mysh.c	: shell program
 * @author	: 허준
 * @id		: 32197720
 * @email	: heojun990322@dankook.ac.kr
 * @version	: 1.0
 * date		: 2022.12.02
 */

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <pwd.h>
#include <fcntl.h>

#define LINE 1024
#define PATH 256
#define TOKENS 10

int tokenize(char* buf, char* tokens[], char* delim) { 
	int count = 0;
	
	if (buf[strlen(buf) - 1] == '\n') // buf의 마지막 문자가 new line문자면 null로 변경
		buf[strlen(buf) - 1] = '\0';
	
	tokens[0] = strtok(buf, delim); // buf를 delim 기준으로 분리
	count++;
	
	// buf의 문자열 분리가 끝날 때까지 반복
	for (int i = 1; ; i++) {
		if (tokens[i] = strtok(NULL, delim))
			count++;
		else
			break;
	}
	
	return count; // token 개수 return
}

void cd(char* tokens[]) {
	if (!tokens[1] || !strcmp(tokens[1], "~")) { // "cd" 다음 token이 null이거나 "~"인 경우
		struct passwd *pw = getpwuid(getuid()); // 현재 uid인 계정의 상세 정보를 저장
		tokens[1] = pw->pw_dir; // "cd" 다음 token을 hoem directory로 초기화
	}

	if (chdir(tokens[1]) == -1) // 현재 작업 중인 directory를 "cd" 다음 token의 경로로 변경
		perror("chdir error ");
}

void help() {
	printf("version 1.0\n\n");
	printf("built-in commands list\n");
	printf("----------------------------------------------------------\n");
	printf("cd\t\t\t현재 디렉토리 이동\n");
	printf("exit\t\t\t쉘 종료\n");
	printf("help\t\t\t명령어 정보\n");
	printf("----------------------------------------------------------\n");
}

void redirection(char* tokens[], int token_count) {
	int fd;
	int out_redir = 0;
	int in_redir = 0;
	int err_redir = 0;
	int out_err_redir = 0;
	int append = 0;
	
	for (int i = 0; i < token_count; i++) {
		// (i번째 token이 null이 아니다) and (stdin redirection이 아직 되지 않았다) and (i번째 token이 "<"이다) 
		if (tokens[i] && !in_redir && !strcmp(tokens[i], "<")) { // standard input redirection
			// "<" 다음에 주어진 문자열 token을 pathname으로 설정 후 open
			if ((fd = open(tokens[i + 1], O_RDONLY)) == -1) {
				perror("open error ");
				exit(1);
			}
			
			// stdin을 키보드에서 open한 file로 변경
			if (dup2(fd, 0) == -1) { // 
				perror("dup2 error ");
				exit(1);
			}
			
			// i번째와 i+1번째 token을 null로 초기화한 후 stdin redirection 여부를 1로 설정 
			tokens[i] = NULL;
			tokens[i + 1] = NULL;
			in_redir = 1;
		}
		
		// (i번째 token이 null이 아니다) and (stdout redirection이 아직 되지 않았다) and (i번째 token이 ">" 또는 ">>"이다)
		else if (tokens[i] && !out_redir && (!strcmp(tokens[i], ">") || (append = !strcmp(tokens[i], ">>")))) { // standard output redirection
			if (append) { // ">>"인 경우
				// ">>" 다음에 주어진 문자열 token을 pathname으로 설정 후 open
				if ((fd = open(tokens[i + 1], O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR)) == -1) {
					perror("open error ");
					exit(1);
				}
			}
			
			else { // ">"인 경우
				// ">" 다음에 주어진 문자열 token을 pathname으로 설정 후 open
				if ((fd = open(tokens[i + 1], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)) == -1) {
					perror("open error ");
					exit(1);
				}
			}
			
			// stdout을 화면에서 open한 file로 변경
			if (dup2(fd, 1) == -1) {
				perror("dup2 error ");
				exit(1);
			}
			
			// i번째와 i+1번째 token을 null로 초기화한 후 stdout redirection 여부를 1로 설정 
			tokens[i] = NULL;
			tokens[i + 1] = NULL;
			out_redir = 1;
		}
		
		// (i번째 token이 null이 아니다) and (stderr redirection이 아직 되지 않았다) and (i번째 token이 "2>" 또는 "2>>"이다)
		else if (tokens[i] && !err_redir && (!strcmp(tokens[i], "2>") || (append = !strcmp(tokens[i], "2>>")))) { // error
			if (append) { // "2>>"인 경우
				// "2>>" 다음에 주어진 문자열 token을 pathname으로 설정 후 open
				if ((fd = open(tokens[i + 1], O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR)) == -1) {
					perror("open error ");
					exit(1);
				}
			}
			else { // "2>"인 경우
				// "2>" 다음에 주어진 문자열 token을 pathname으로 설정 후 open
				if ((fd = open(tokens[i + 1], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)) == -1) {
					perror("open error ");
					exit(1);
				}
			}
			
			// stderr을 화면에서 open한 file로 변경
			if (dup2(fd, 2) == -1) {
				perror("dup2 error ");
				exit(1);
			}
			
			// i번째와 i+1번째 token을 null로 초기화한 후 stderr redirection 여부를 1로 설정
			tokens[i] = NULL;
			tokens[i + 1] = NULL;
			err_redir = 1;
		}

		// (i번째 token이 null이 아니다) and (stdout과 stderr redirection이 아직 되지 않았다) and (i번째 token이 "2>&1" 또는 "1>&2"이다)
		else if (tokens[i] && !out_err_redir && (!strcmp(tokens[i], "2>&1") || !strcmp(tokens[i], "1>&2"))) {
			// token 문자열 첫 번째 문자와 같은 번호의 fd를 네 번째 문자와 같은 번호의 fd로 변경
			if (dup2((int)(*(tokens[i] + 3) - '0'), (int)(*tokens[i] - '0')) == -1) {
				perror("dup2 error ");
				exit(1);
			}
			
			// i번째와 token을 null로 초기화한 후 stdout과 stderr redirection 여부를 1로 설정
			tokens[i] = NULL;
			out_err_redir = 1;
		}
	}
}

void run(char* line) {
	int token_count, status;
	char* tokens[TOKENS];
	pid_t pid;
	
	if (!memset(tokens, 0, sizeof(char*)*TOKENS)) { // tokens 배열의 모든 원소를 null로 초기화
		perror("memset error ");
		return;
	}
	
	token_count = tokenize(line, tokens, " "); // line을 띄어쓰기를 기준으로 parsing
	
	if (!strcmp(tokens[0], "cd")) // cd를 입력한 경우
		cd(tokens);
	
	else if (!strcmp(tokens[0], "exit")) // exit을 입력한 경우
		exit(0);
	
	else if (!strcmp(tokens[0], "help")) // help를 입력한 경우
		help();
	
	else { // built-in commands가 아닌 경우
		pid = fork(); // process 생성
		int background = strcmp(tokens[token_count - 1], "&"); // background process로 실행했는지 확인
		
		if (pid == -1) { // fork 실패
			perror("fork error ");
			exit(1);
		}
		
		else if (pid == 0) { // child process
			redirection(tokens, token_count); // redirection 처리
			
			if (background) { // foreground
				if (execvp(tokens[0], tokens) == -1) {
					perror("execvp error ");
					exit(1);
				}
			}
			else { // background
				tokens[token_count - 1] = NULL; // tokens의 마지막 문자열 "&"을 null로 초기화
				
				if (execvp(tokens[0], tokens) == -1) {
					perror("execvp error ");
					exit(1);
				}
			}
		}
		
		else if (pid > 0) { // parent process
			if (background) { // foreground
				if (waitpid(pid, &status, 0) == -1) { // child process가 종료될 때 까지 대기
					perror("waitpid error ");
					return;
				}
			}
			else { // background
				printf("pid : %d\n", pid);
				usleep(100*1000); // 0.1초 대기	
			}
		}
	}
}

int main(int argc, char* argv[]) {
	char line[LINE], cur_dir[PATH];
	uid_t uid, euid;
	
	while(1) { // 라인을 입력받고 실행하는 것을 무한반복
		if (!getcwd(cur_dir, PATH)) { // cur_dir을 현재 경로로 초기화
			perror("getcwd error ");
			continue;
		}
		
		uid = getuid();
		euid = geteuid();
		
		if (uid || euid) // root인지 검사
			printf("%s$ ", cur_dir);
		else
			printf("%s# ", cur_dir);
		
		fgets(line, LINE, stdin);
		
		if (!strcmp(line, "\n")) // 라인 입력을 안하고 엔터를 누른 경우
			continue;
		
		run(line); // 라인 실행
	}
	
	return 0;
}