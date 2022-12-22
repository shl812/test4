/**
 * ms2.c	: shell program prototype2
 * @author	: Seungbeom Kim
 * @student'sID	: 32190777
 * @email	: sungbum0503@naver.com
 * @version	: 2.0
 * @date	: 22.12.08
**/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>


#define MAX_SIZE 1024

bool cd(int argc, char* argv[]);
bool quit(int argc, char* argv[]);

struct cmdSet {
 char *name;
 char *info;
 bool (*cmdFunc) (int argc, char *argv[]);
};

struct cmdSet cmds[4] = {
 {"cd", "Change directory", cd},
 {"quit", "Quit my shell", quit},
 {">", "Redirection"},
 {"&", "Background processing"}
};

bool cd(int argc, char* argv[]) {
	if (argc == 2) {
		if (chdir(argv[1]))
			printf("%s is no directory\n", argv[1]);
	}
	else {
		printf("Enter correct argument: cd [directory name]\n");
	}
	return true;
}

bool quit(int argc, char* argv[]) {
 return false;
}

void redirection(int argc, char* argv[]) {
	int fd;
	if (argc == 4) {
		fd = open(argv[3], O_WRONLY | O_CREAT, 0664);
		if (fd < 0) {
			printf("%s file dosen't open. errno. %d\n", argv[3], errno);
			exit(-1);
		}
		dup2(fd, STDOUT_FILENO);
		argv[2] = NULL;
	}
	close(fd);
	return;
}

int tokenize(char* buf,char* delims,char* ts[],int maxT) {
	char *t;
	int t_count = 0;
	t = strtok(buf,delims);
	while(t != NULL && t_count < maxT){
		ts[t_count++] = t;
		t = strtok(NULL,delims);
	}
	ts[t_count] = NULL;
	return t_count;
}

bool execute(char* line) {
	char* delims = " \n\t";
	char* ts[MAX_SIZE]; 
	int i;
	int t_count; 
	int status;
	pid_t pid;
	bool BG = false;
 	bool RD = false;
	char* isRD = strchr(line, '>');
 	char* isBG = strchr(line, '&');
	
	if (isRD != NULL) 
		RD = true;
	if (isBG != NULL) 
		BG = true;
	
	t_count = tokenize(line, delims, ts, sizeof(ts)/sizeof(char*));
	
	if (t_count == 0)
		return true;
	
	if(BG == true)		// & 오류 잡기
		ts[t_count-1] = NULL;
	
	for (i = 0; i < 3; i++) {
		if (strcmp(cmds[i].name, ts[0]) == 0)
			return cmds[i].cmdFunc(t_count, ts);
	}
	
	if ((pid = fork()) < 0) {
		perror("fork error");
		exit(-1);
	}
	else if (pid == 0) {
		if (RD == true) {
			redirection(t_count, ts);
		}
		execvp(ts[0], ts);
	}
	if (BG == false)
		wait(&status);
	
	return true;
}


int main() {
	char line[MAX_SIZE];
	while(1) {
		printf("[%s] $", getcwd(NULL, 0));
		fgets(line, sizeof(line)-1, stdin);
		if(execute(line) == false) break;
	}
	return 0;
}