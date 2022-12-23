/**
 * hw04.c        : system programming homework 04
 * @author       : Yujin Ju
 * @student's ID : 32214411
 * @email        : ugin0709@naver.com
 * @version      : 1.0
 * date          : 2022.11.27
**/

#include <stdio.h>
#include <string.h> // for parsing
#include <unistd.h> // for execl(), fork()
#include <stdlib.h> // for system
#include <wait.h> // for wiat()
#include <fcntl.h>

char **parse(int position, char *argv[]); // parsing function
void change_standard_input(void)
{
	int fd = open("test_txt.txt", O_RDONLY);
	dup2(fd, STDIN_FILENO);
	close(fd);
}
int main(int argc, char *argv[], char *envp[])
{
	char *path;
	char **argument;

	int dup2(int oldfd, int newfd);
       	//use dup2() function to implement redirection
	argument = (char **)malloc(sizeof(char *) * 1);
	argument[0] = strdup("cat");
	path = strdup("/usr/bin/cat");
	change_standard_input();
	execve(path, argument, envp);

	int pid; // pid is returnd by fork
	int counter = 1;
	int f = argc;
	pid = fork(); // fork occurrence
	if(pid == -1) // if -1, fork creation error
	{
		fprintf(stderr, "Fork Failed");
		exit(0); // termination due to error
	}
	else if (pid == 0) //if 0, child process
	{
		printf("Chinld... I'm here\n");
		printf("execl start\n");
	       	// execl is called to create new process and the chile process is terminated
		execl("/bin/ls", "ls", "-1", NULL);
		// execl function
		printf("if you read this meassage, fail creaing execl func..\n");
		sleep(1);
	}
	else // parent process
	{
		printf("Parent... I'm here\n");
		sleep(1);
		printf("process Complete\n");
	}
	return 0;
}