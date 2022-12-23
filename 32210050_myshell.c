#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#define MAX_BUF 64
#define MAXARG 7

/*
myshell.c
author : 강민지
student number : 32210050
*/


void redirectionIn(char path[], char* tokens[], int cursor) {
	//<
	
    pid_t pid;

    int fd, i;
    int num = 0;

    char* Before[100]; //redirectionIn 이전 buf
    char* After[100]; //redirectionIn 이후 buf
    
    memset(Before, 0x00, 100); 
    memset(After, 0x00, 100); 
	//before,after buf 초기화

    for (i = 0; i < cursor; i++) {Before[i] = tokens[i];}
	
    for (i = cursor + 1; tokens[i] != NULL; i++) {After[num++] = tokens[i];}

    pid = fork();

    if (pid < 0) {printf("fork error\n");}

    else if (pid == 0) {
        fd = open(After[0], O_RDONLY);
        dup2(fd, STDIN_FILENO);
        close(fd);
        if (execve(path, Before, NULL) == -1) {
            printf("Command execution is failed\n");
            exit(0);
        }
    }

    wait(NULL);
}

void redirection1(char path[], char* tokens[], int cursor) { 
	//>
	
    pid_t pid;
	
    int fd, i;
    int num = 0;
    
    char* Before[100]; //redirection1 이전 buf
    char* After[100]; //redirection1 이후 buf
    
    memset(Before, 0x00, 100); 
    memset(After, 0x00, 100); 
	//before,after buf 초기화

    for (i = 0; i < cursor; i++) {Before[i] = tokens[i];}
	
    for (i = cursor + 1; tokens[i] != NULL; i++) {After[num++] = tokens[i];}
	
    pid = fork();

    if (pid < 0) {printf("fork error\n");}
	
    else if (pid == 0) {
        fd = open(After[0], O_RDWR | O_CREAT | O_TRUNC, 0641);
        dup2(fd, STDOUT_FILENO); 
        close(fd);
        if (execve(path, Before, NULL) == -1) {
            printf("Command execution is failed\n");
            exit(0);
        }
    }

    wait(NULL);
}

void redirection2(char path[], char* tokens[], int cursor) {
	//>>
	
    pid_t pid;

    int fd, i;
    int num = 0;

    char* Before[100]; //redirection2 이전 buf
    char* After[100]; //redirection2이후 buf
    
    memset(Before, 0x00, 100); 
    memset(After, 0x00, 100); 
	//before,after buf 초기화

    for (i = 0; i < cursor; i++) {Before[i] = tokens[i];}
	
    for (i = cursor + 1; tokens[i] != NULL; i++) {After[num++] = tokens[i];}
    
    pid = fork();
    
    if (pid < 0) {printf("fork error\n");}

    else if (pid == 0) {
        fd = open(After[0], O_RDWR | O_CREAT | O_APPEND, 0641);
        dup2(fd, STDOUT_FILENO); 
        close(fd);
        if (execve(path, Before, NULL) == -1) {
            printf("Command execution is failed\n");
            exit(0);
        }
    }

    wait(NULL);
}




void backgroundProcess(char path[], char* tokens[], int cursor) {
    //백그라운드

    pid_t pid;
    tokens[cursor] = NULL;

	printf("################################\n\n");
    printf("      Background Process\n\n");
	printf("################################\n\n");

    pid = fork();

    if (pid < 0) {printf("fork error\n");}
    
    else if (pid == 0) {
        if (execve(path, tokens, NULL) == -1) {
            printf("Command execution is failed\n");
            exit(0);
        }
    }
}

void change(char* cd) {
    if (chdir(cd) == -1) {printf("%s with errno %d", strerror(errno), errno);}
}

void exe(char path[], char* tokens[], int tokenNum) {
    pid_t pid;
    int flag = 0;

    strcat(path, tokens[0]); 

    //cd
    if (strcmp(tokens[0], "cd") == 0) {
        change(tokens[1]);
        return;
    }

    if ((pid = fork()) < 0) {
        printf("fork error\n");
        exit(0);
    }

    else if (pid == 0) { //자식
        for (int i = 0; i < tokenNum; i++) {
            if (strcmp(tokens[i], "<") == 0 && flag == 0) {
                redirectionIn(path, tokens, i);
                flag++;
            }
			else if (strcmp(tokens[i], ">") == 0 && flag == 0) {
                redirection1(path, tokens, i);
                flag++;
            }
            else if (strcmp(tokens[i], ">>") == 0 && flag == 0) {
                redirection2(path, tokens, i);
                flag++;
            }
            else if (strcmp(tokens[i], "&") == 0 && flag == 0) {
                exe(path, tokens, i);
                flag++;
            }
            if (flag != 0) {break;}
        }
        if (flag == 0) {
            if (execve(path, tokens, NULL) == -1) {
                printf("Command execution is failed\n");
                exit(0);
            }
        }


    }
    if (pid > 0) {
        wait(NULL);
    }
}

int Parsing(char* buf, char* token[]) {   //파싱
    int num = 0;
    char* ptr = strtok(buf, " ");//공백기준으로 자름
    token[num++] = ptr;
    while (1) {
        ptr = strtok(NULL, " ");
        if (ptr == NULL) {
            token[num] = NULL;
            break;
        }
        else {token[num++] = ptr;}
    }//파싱 끝
    return num; //토큰 개수 리턴
}


int main() {
	
    int tokenNum; 
    char path[100] = "/bin/"; 
    char* argv[100];
    char buf[MAX_BUF];


	printf("################################\n\n");
    printf("         Make myShell\n\n");
	printf("################################\n\n");

    while (1)
    {
        printf("goormide>");
        memset(buf, 0x00, MAX_BUF);
        memset(argv, 0x00, 100);
        memset(path, 0x00, 100);
        strcpy(path, "/bin/"); 
        fgets(buf, MAX_BUF - 1, stdin); 
        if (strncmp(buf, "exit\n", 5) == 0) {
            break;
        }
        buf[strlen(buf) - 1] = 0x00; 
        tokenNum = Parsing(buf, argv);
        exe(path, argv, tokenNum);

    }
    return 0;
}
