// 32201525 문주안 컴퓨터공학과
// 시스템프로그래밍 2분반
// Homework4
// 2022.12.09
// homework4.
// timidity@kakao.com
// 



#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#define MAX 1024

int tokenize(char* buf, char* delims, char* tokens[], int maxTokens){
    int i = 0;
    char* token = strtok(buf, delims);

    while(token != NULL && i < maxTokens){
        tokens[i] = token;
        token = strtok(NULL, delims);
        i++;
    }
    return i;
}

bool run(char* line){
    char* tokens[MAX];
    pid_t child, redirection_child;
    int exit_status = -1;
    int fd;

    char* delims = " ";

    int j = 0;
    int i = 0;
    char* isbackground;

    // 자르기
    int token_count = tokenize(line, delims, tokens, sizeof(tokens) / sizeof(char*));

    // tokens 출력
    //printf("%s\n", tokens[0]);

    // handling internal commands such as cd, and exit
    // handling redirection and background processing

    // exit이면 종료
    if(!strcmp(tokens[0], "exit")){
            return false;
    }

    // redirection 판별
    while(tokens[j] != NULL){
            if(strcmp(tokens[j], ">") == 0){
                    if(redirection_child == 0){ // child
                            if(fd = open(tokens[j + 1], O_RDWR | O_CREAT | O_TRUNC, 0644) < 0){
                                    printf("redirection open error");
                                    exit(-1);
                            }
                            else {
                                   // printf("I'm child");
                                    dup2(fd, STDOUT_FILENO);
                                    tokens[j] = '\0';
                                    execvp(tokens[0], tokens);
                                    close(fd);
                            }
                    }
                    else{
                            wait(&exit_status);
                    }
                    return true;
            }
            j++;
    }

    // background 판별
    if((isbackground = strchr(tokens[0], '&')) != NULL){
        while(tokens[0][i])
                i++;
        tokens[0][i - 1] = '\0';
    }
    if((child = fork()) == -1){ // error
        printf("error");
        exit(-1);
    }
    else if(child== 0){ // child
        execvp(tokens[0], tokens);
    }
    if(isbackground == NULL)
        wait(&exit_status);

    return true;
}

int main(void) {
    char line[MAX];
    while(1){
        //getcwd(), 현재 디렉토리를 불러온다.
        printf("%s $ ", getcwd(line, MAX));
        fgets(line, sizeof(line), stdin);
        int len = strlen(line);
        if(line[len - 1] == '\n')
                line[len - 1] = '\0';
        if(run(line) == false) break;
    }
    return 0;
 }

