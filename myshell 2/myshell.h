/*
	이름 : 박민규
	학번 : 32191597
	날짜 : 2022-12-03
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#define MAXARG 7

void doRedirection(int flag, int is_bg, char *argv[], char* input, char * output);
int checkInput(char buf[]);
int tokenized(char buff[], char* arg[] ,char delim[]);
void executesLine(int isbg, char * argv[]);
