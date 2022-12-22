/* 시스템프로그래밍(SW) 1분반 shell 32213775 소프트웨어학과 임수종*/

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>   
#include <stdlib.h>  
#include <unistd.h>
#include <stdbool.h> 
#include <string.h>  
#include <errno.h>   
#include <fcntl.h>  

void help();
bool run(char *line);
int tokenize(char *buf, char *delimit, char *tokens[], int maxTokens);
void changeDir(char *targetDir[]);

int main() {
    char line[1024];
    while (1)
    {
        printf("%s$ ", get_current_dir_name());
        fgets(line, sizeof(line) - 1, stdin);
        if (run(line) == false)
        {
            break;
        }
    }
    return 0;
}

void help()
{
    printf("============Shell============\n");
    printf("How to use dgshell\n");
    printf("?\t: show this help\n");
    printf("help\t: show this help\n");
    printf("exit\t: exit this shell\n");
    printf("quit\t: quit this shell\n");
    printf("&:\t: background processing\n");
    printf(">:\t: redirection\n");
    printf("|:\t: pipe(Just once)\n");
    printf("======================================\n");
}

bool run(char *line) {
	char *tokenArray[128];
	int count;
	int tokenCount = tokenize(line, " \n", tokenArray, (sizeof(tokenArray) / sizeof(char *)));

	//Advanced commnad 들이 요청되었는지를 체크하기 위한 플래그 용도로 변수를 선언
	bool background_flag = false;
	bool redirection_flag = false;
	bool pipe_flag = false;

	//pipe 와 redirection, "|" 와 ">" 토큰 위치를 NULL 로 변경해주기 위해 다음 변수들을 사용
	int redirectionTokenNumber = -1;
	int pipeTokenNumber = -1;

	pid_t fork_return;

	//file descriptor 선언.
	int fd;
	int fdpipe[2];
	//pipe 시에 앞과 뒤 명령어를 구분해줄 것인데 이를 위한 문자열
	char *pipeBuffer1[10];
	char *pipeBuffer2[10];
	int i = 0, col = 0;


	if (tokenCount == 0){ return true; }
	else if ((strcmp(tokenArray[0], "help") == 0) || (strcmp(tokenArray[0], "?") == 0))
	{
		help();
		return true;
	}
	else if ((strcmp(tokenArray[0], "exit") == 0) || (strcmp(tokenArray[0], "quit") == 0))
	{
		return false;
	}
	else if ((strcmp(tokenArray[0], "cd")) == 0 && tokenCount == 2)
	{
		changeDir(tokenArray);
		return true;
	}

	for (count = 0; count < tokenCount; count++)
	{
		if (strcmp(tokenArray[count], "&") == 0)
		{
			background_flag = true;
			tokenArray[count] = NULL;
		}
		else if (strcmp(tokenArray[count], ">") == 0)
		{
			redirection_flag = true;
			tokenArray[count] = NULL;
			redirectionTokenNumber = count + 1;
		}
		else if (strcmp(tokenArray[count], "|") == 0)
		{
			pipe_flag = true;
			pipeTokenNumber = count;
		}
	}

	if ((fork_return = fork()) < 0)
	{
		perror("fork error");
		printf("mysh 을 종료합니다.\n");
		exit(1);
	}
	else if (fork_return == 0)
	{
		if (pipe_flag == true)
		{
			for (i = 0; i != pipeTokenNumber; i++)
			{
				pipeBuffer1[col] = tokenArray[i];
				col++;
			}
		
			tokenArray[pipeTokenNumber] = NULL;
			pipeBuffer1[col] = NULL;
			i++;
			col = 0;

			while (tokenArray[i] != NULL)
			{
				pipeBuffer2[col] = tokenArray[i];
				col++;
				i++;
			}
		
			pipeBuffer2[col] = NULL;
		}	
	
		if (redirection_flag == true)
		{	

			if ((fd = open(tokenArray[redirectionTokenNumber], O_RDWR | O_CREAT, 0664)) < 0)
			{
				printf("Error occurred during 'open' system call, errno=%d\n", errno);
				exit(1);
			}
			// STD_OUT 이 fd 가 가리키는 아이노드를 가리키게 dup2 사용.
			dup2(fd, 1);
			close(fd); // fd 와 해당 파일의 연결 끊음
		}
		if (pipe_flag == false)
		{
			execvp(tokenArray[0], tokenArray);
			exit(1);
		}
		else
		{
			pipe(fdpipe);
			if (fork() == 0)
			{
				close(fdpipe[0]);
				dup2(fdpipe[1], 1);
				close(fdpipe[1]);
				execvp(pipeBuffer1[0], pipeBuffer1);
				exit(1);
			}
			else
			{
				close(fdpipe[1]);
				dup2(fdpipe[0], 0);
				close(fdpipe[0]);
				execvp(pipeBuffer2[0], pipeBuffer2);
				exit(1);
			}
		}
	}
	
	else
	{
		// background_flag 에 따라 부모 프로세스가 wait() 할 지를 결정.
		if (background_flag == false)
		{
			wait();
		}
	}
	return true;
}

int tokenize(char *buf, char *delimit, char *tokens[], int maxTokens)
{
	int tokenCount = 0;
	char *token;
	token = strtok(buf, delimit);
	while (token != NULL && tokenCount < maxTokens)
	{
		tokens[tokenCount] = token;
		token = strtok(NULL, delimit);
		tokenCount++;
	}
	tokens[tokenCount] = NULL;
	return tokenCount;
}

void changeDir(char *targetDir[])
{
	char *path;
	char buf[100];
	
	//현재 디렉토리 이름 복사 후 실패시 NULL 반환.
	path = getcwd(buf, sizeof(buf));
	if (path != NULL)
	{
		//이동할 디렉토리 이름을 뒤에 붙여준다.
		path = strcat(path, "/");
		path = strcat(path, targetDir[1]);
		//chdir 함수를 통해 현재 작업 디렉토리를 변경하여준다.
		chdir(path);
	}
}


