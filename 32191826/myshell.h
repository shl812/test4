#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#define MAXARG 7

void redirect(int flag, int is_bg, char *argv[], char* input, char * output);
int tokenized(char buff[], char* arg[] ,char delim[]);
int checkInput(char buf[]);
void execute(int isbg, char * argv[]);
