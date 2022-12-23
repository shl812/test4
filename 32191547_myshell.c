/**
* mycat.c	: make shell
* @author	: Kunwoo Park
* @email	: pkunwoo6593@gmail.com
* @version	: 1.0
* @date		: 2022. 10 .31
* @StudentID: 32191547
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/wait.h>


int main(int arc, char *argv[], char *envp[])
{
	int pid;
	int child_pid;
	int status;
	pid = fork();
	
	if(pid<0){
		perror("fork failed\n");
		exit(1);
	}
	
	else if (pid ==0){
		char	*path;
		char	**argument;

		argument = (char **)malloc(sizeof(char *) * 2);
		argument[0] = strdup("ls");
		argument[1] = strdup("-l");
		path = strdup("/bin/ls");
		
		int fd = open("test.txt", O_RDONLY);
		dup2(fd, STDIN_FILENO);
		close(fd);
		
		execve(path, argument, envp);
		
	}
	
	else{
		child_pid = wait(NULL);
		printf("ls is complete\n");
		exit(0);
	}
}