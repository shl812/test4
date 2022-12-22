#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>   
#include <stdlib.h>  
#include <unistd.h>
#include <stdbool.h> 
#include <string.h>  
#include <errno.h>   
#include <fcntl.h>  

void cmd_help();
int tokenize(char *buf, char *delimis, char *tokens[], int maxTokens);
void cmd_cd(char *targetDir[]);
bool run(char *line);

void cmd_help() //shell의 사용방법을 설명해주는 함수이다.
{   printf("======================================\n");
    printf("help\t: show this help\n");
    printf("quit\t: quit this shell\n");
    printf("&:\t: background processing\n");
    printf(">:\t: redirection\n");
    printf("======================================\n");
}
int tokenize(char *buf, char *delims, char *tokens[], int maxTokens)
// 입력받은 문자열을 분리하여 토큰에 저장해주는 함수이다.
{
	int token_count = 0;
	char *token;
	token = strtok(buf, delims); //strtok 함수를 사용하여 입력받은 문자열을 분리한다.
	while (token != NULL && token_count < maxTokens)
	{
		tokens[token_count] = token;
		token = strtok(NULL, delims);
		token_count++;
	}
	tokens[token_count] = NULL;
	return token_count;
}
void cmd_cd(char *targetDir[])//cd를 입력받았을떄 실행되야할 함수이다.
{
	char *path;
	char buf[1024];
	path = getcwd(buf, sizeof(buf));//buf의 경로를 받아온다.
	if (path != NULL)
	{
		path = strcat(path, "/");
		path = strcat(path, targetDir[1]);//목적경로를 붙인다.
		chdir(path);//해당 디렉토리로 이동한다.
	}
}

bool run(char *line) {
	char *tokens[128];
	int count; 
	int token_count = tokenize(line, " \n", tokens, (sizeof(tokens) / sizeof(char *)));
    //명령어로부터 분리된 토큰의 갯수이다.
	pid_t fork_return;
	bool background_flag = false;//백그라운드 작업의 여부를 정해준다.
	bool redirection_flag = false;//리다렉션 작업의 여부를 정해준다.
	int redirTokenNumber = -1;
	int fd;

	if (token_count == 0){ return true; }
	else if ((strcmp(tokens[0], "help") == 0))//입력받은 명령어가 help이면 cmd_help를 실행한다.
	{
		cmd_help();
		return true;
	}
	else if ((strcmp(tokens[0], "exit") == 0))
	{
		return false;
	}
	else if ((strcmp(tokens[0], "cd")) == 0 && token_count == 2)
    //입력받은 명령어가 cd고 명령어로부터 분리된 토큰의 갯수가 2개면 cmd_cd함수를 실행한다.
	{
		cmd_cd(tokens);
		return true;
	}

	for (count = 0; count < token_count; count++)
	{
		if (strcmp(tokens[count], "&") == 0)// &을 사용하였을때
		{
			background_flag = true;//플래그를 설정하고
			tokens[count] = NULL;// & 문자열을 NULL로 바꾼다.
		}
        else if (strcmp(tokens[count], ">") == 0) // >를 사용하였을때 플래그를 설정하고
		{
			redirection_flag = true;
			tokens[count] = NULL;// > 문자열을 NULL로 바꾼다.
			redirTokenNumber = count + 1;//다음에 오는 문자열로 offset을 설정한다.
		}
	}
    if ((fork_return = fork()) < 0) //fork 에러인 경우를 정의해준다.
	{
		perror("fork error");
		exit(1);
	}
	else if (fork_return == 0)
	{
		if (redirection_flag == true)
		{	

			if ((fd = open(tokens[redirTokenNumber], O_RDWR | O_CREAT, 0664)) < 0)
			{
				printf("Error");
				exit(1);
			}
			// 1이 STD_OUT을 의미하며 fd 가 1을 향하게 dup2 사용.
			dup2(fd, 1);
			close(fd); 
		}
        execvp(tokens[0],tokens); // execvp 함수를 사용하여 명령어를 실행하고 프로세스를 종료시킨다.
        exit(1);
	}
	else
	{
		if (background_flag == false)
		{
			wait(NULL);
	 }
	return true;
}
}
int main() {
    char line[1024];
    while (1)
    {
        printf("%s$ ", getcwd(line,1024)); //현재 디렉토리를 반환한다.
        fgets(line, sizeof(line) - 1, stdin);//명령어를 입력받는다.
        if (run(line) == false) break;
    }
    return 0;
}


