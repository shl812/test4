// 32191879 소프트웨어학과 박진오//
//---------2022.12.07---------//
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdbool.h>
#include <fcntl.h>
int b=0,L=0;   //background와 redirection을 위한 조건

int tokenize(char* buf,char* delims,char* tokens[],int maxTokens)
{    
	char *token;
	int token_count = 0;
	
	token = strtok(buf,delims);

	while(token != NULL && token_count <maxTokens)
	{
		tokens[token_count++] = token;
		token = strtok(NULL,delims);
	}
	tokens[token_count] = NULL;
	return token_count;
	
}

bool run(char* line)
{
	int token_count; char* tokens[1024]; 
	char* delims = " \n\t";
	int fd_redir,fd_bg, i,j;
	char* tokencp[1024];
	
	pid_t child = 0;
	b=0; L=0;
	token_count = tokenize(line,delims,tokens,sizeof(tokens)/sizeof(char*));
	
	for(i = 0; i< token_count; i++)
	{
		if(!strcmp(tokens[i],">"))
		{
			L++;
		}
		
		if(!strcmp(tokens[i],"&"))
		{
			b++;
		}
		
		printf("입력된 명령어는 : %s \n",tokens[i]);
		
	}
	
	if(strcmp(tokens[0],"cd") == 0)
	{       //CD명령어 (내부명령어) 구현
		if(chdir(tokens[1])!=0)
			perror("cd");
		return 1;
	}
	if(!b) 
	{
		if((child = fork())==0) 
		{
			execvp( tokens[0],tokens);
			exit(0);
		}
		wait();
	} 
	else if(b) 		// background 일때
	{    
		if((child = fork())==0)
		{       
			fd_bg = open("/dev/null",O_RDONLY);
			dup2(fd_bg,STDOUT_FILENO);
			execvp( tokens[0],tokens);
			exit(0);
		}
	}
	else if(L)        //redirection 일때
	{
		if((child=fork())==0)
		{
			fd_redir = open(tokens[3],O_RDWR | O_CREAT | O_TRUNC,0641);
			dup2(fd_redir,STDOUT_FILENO);
			close(fd_redir);
			execvp(tokens[0],tokens);
			exit(0);
		}
		wait();
	}
	memset(tokens, '\0', 1024);
	return 1;
}

int main()
{       
	char line[1024];
    while(1)
    {
		printf("[%s] $", getcwd(line,1024));
		fgets(line, sizeof(line)-1, stdin);
		if(run(line) == false) break;
    }
    return 0;
}