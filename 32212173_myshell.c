/*
 * myshell.c	: my linux shell written in C
 * studentName	: Sung Yoo Hyun
 * studentNumber: 32212173
 * email		: jminfamous@dankook.ac.kr
 * date			: 2022.12.09
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX 255

// shell prompt for user input
// command	: command line input
// username	: names for describing current directory
// hostname :
// dirname	:
// ret		: return address
void *prompt(char command[])
{
	// variables in need
	char username[MAX], hostname[MAX], dirname[MAX];
	void *ret;
	
	// get names in buf
	getlogin_r(username, MAX);
	gethostname(hostname, MAX);
	getcwd(dirname, MAX);
	
	// describe current directory
	printf("%s@%s(%s)# ", username, hostname, dirname);
	
	// get return address at command
	ret=fgets(command, MAX, stdin);

	// handle Enter key as the prompt is end
	if (command[strlen(command)-1]=='\n')
	{
		command[strlen(command)-1]='\0';
	}
	return ret;
}

int main(int arc, char *argv[], char *envp[])
{
	char command[MAX];	// command line
	char *arg;			// for return NULL as the command is inseparable
	char *path="/bin";	// for use builtin command
	pid_t pid;			// forked process id 
	int status;			// child process status 
	
	while (prompt(command))
	{
		// use fork() for spawning child process
		// pid must more than zero or print error message
		if ((pid=fork())<0)
		{
			perror("Failed Forking\n");
		}
		
		// compare command with builtin command and execute
		if (strcmp(command, "exit")==0) // exit
		{
			exit(0);
		}
		else if (pid==0)	// child process
		{
			strtok(command, " ");	// split the command
			arg=strtok(NULL, " ");	// return NULL
			
			// use exec command for execute command
			if (arg==NULL)	// the command read is over
			{
				execlp(command, command, (char*) 0);
			}
			else
			{
				if (strcmp(command, "cd")==0)	// cd
				{
					chdir(arg);	// change directory
					exit(0);	// end
				}
				else
				{
					execlp(command, command, arg, (char*) 0);
				}
			}
			perror("Failed Execute");
		}
		
		// wait until child process finishes terminated
		waitpid(pid, &status, 0);
		
	}
	exit(0);
}