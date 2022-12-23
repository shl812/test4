/**
 * myshell.h	: header file for myshell programs
 * @author		: Lee Heon (32207734)
 * @email		: knife967@gmail,com
 * @version		: 1.0
 * @date		: 2022.11.21 ~ 2022.12.09
**/

#ifndef MYSHELL
# define MYSHELL

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <fcntl.h>
# include <errno.h>
# include <string.h>
# include <sys/wait.h>

# define MAX 255

void	*displayPrompt(char charBuffer[]);
int		getDataNum(char charBuffer[]);
void	execWithErrorCheck(char *arg1, char *arg2, char *arg3, char *arg4);
pid_t	forkNexec(char *arg1, char *arg2, char *arg3, char *arg4, int dataNum);
void	redirectControl(int fd, char *argIn, char *argOut, char *argOption, int ioFlag);
pid_t	myRedirection(char *arg1, char *arg2, char *arg3, char *arg4, int operatorIndex);
pid_t	myPipe(char *arg1, char *arg2, char *arg3, char *arg4, int operatorIndex);

#endif
