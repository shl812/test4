/**
  * myShell.c		: shell program
  * @author		: Minho Lee
  * @email		: 32172995@dankook.ac.kr
  * @version		: 1.0
  * date			: 2022.12.09
  **/


#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>

#define MAX 1024


int tokenize(char *buf, char *tokens[]){
	char *delims;
	char *token;
	int token_count =0;
	token = strtok(buf,delims);
	while(token != NULL){
		tokens[token_count++]=token;
		token = strtok(NULL,delims);
	}
	tokens[token_count] = NULL;
	return token_count;
}

void InnerCmd(char *token){
	if(chdir(token) == -1) perror("cd");
}

void BackgroundProce(char *token[]){
	pid_t pid;
	int fd_bgp;
	printf("%s\n",token[0]);
	if((pid = fork())==0){
		fd_bgp = open("/dev/null", O_RDONLY);
		dup2(fd_bgp, STDOUT_FILENO);
		execvp(token[0],token);
		exit(0);
	}
}
	
void Redirection(char *token[]){
	pid_t pid;
	int fd_rdi;
	printf("redirection\n");
	if((pid = fork())==0){
		fd_rdi = open(token[3], O_RDWR | O_CREAT | O_TRUNC,0641);
		dup2(fd_rdi,STDOUT_FILENO);
		close(fd_rdi);
		execvp(token[0],token);
		exit(0);
	}
	wait();
}


bool Execute(char *cmd){
	int token_count;
	int i =0;
	char *tokens[MAX];
	pid_t pid;
	int addflag = 0;

	token_count = tokenize(cmd, tokens);
	if(strcmp(tokens[0],"cd") ==0){
		InnerCmd(tokens[1]);
		return;
	}
	for(i=0;i<token_count;i++){
		if(!strcmp(tokens[i],">")){
			addflag++;
			Redirection(tokens);
		}
		if(!strcmp(tokens[i],"&")){
			addflag++;
			BackgroundProce(tokens);
		}
	}
	if(!addflag){
		if((pid = fork()) ==0){
			execvp(tokens[0],tokens);
			exit(0);
		}
		wait();
	}
	memset(tokens,'\0',MAX);
	return 1;
}
int main(){
	char cmd[MAX];
	while(1){
		printf("%s$ ",get_current_dir_name());
		fgets(cmd, sizeof(cmd)-1,stdin);
		if(strncmp("exit",cmd,4) == 0){
			break;
		}
		
		Execute(cmd);
	}
}


