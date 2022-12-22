#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#define MAX 1024

void redirection(char **input, int i); //redirection 구현 함수
void background(char **input, int i); //background processing 구현 함수
char **tokenize(char *inputstring); //입력받은 문장 token화하는 함수
void execute(char **input); //명령어 실행하는 함수
