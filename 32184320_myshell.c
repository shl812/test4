/**
    myShell.c : myshell program
    @author : Sungbeom Cho
    @email : sbcho98@naver.com
    @version : 1.0
    @date : 2022.12.09
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/wait.h>

// 명령어 토큰 수 세어서 반환하는 함수
int tokenizer(char* buf, char* delims, char* tokens[], int maxTokens) {
    char* token;
    int tokenCount = 0;
    
    token = strtok(buf, delims);
    while (token != NULL && tokenCount < maxTokens) {
        tokens[tokenCount++] = token;
        token = strtok(NULL, delims);
    }
    
    tokens[tokenCount] = NULL;
    
    return tokenCount;
}

// change directory 구현 함수, 토큰 수 가 cd 하나면 HOME으로, 2개면 인자값으로 위치 변환을, 3개 이상이면 오류 메세지 출력
bool cd(int tokenCount, char* tokens[]) {
    if (tokenCount == 1) {
        chdir(getenv("HOME"));
    }
    else if (tokenCount == 2) {
        chdir(tokens[1]);
    }
    else {
        printf("ERROR\n");
    }
    return true;
}

// myshell 구동
bool run(char* cmd) {
    char* tokens[128];
    char* temp[128];
    char* delims = " \n";
    
    pid_t child;
    int tokenCount = tokenizer(cmd, delims, tokens, sizeof(tokens)/sizeof(char*));
    int fd;
    int flag = 0;
    int i, j;
    int status;
    bool back = false;
    
    for (i=0; i<tokenCount; i++) {
        if (!strcmp(tokens[i], ">")) {    // redirection
            flag = 1;
            break;
        }
        else if (!strcmp(tokens[i], "&")) {
            back = true;
            break;
        }
        else if (!strcmp(tokens[i], "cd")) {
            return cd(tokenCount, tokens);
        }
    }
    
    if ( (child = fork()) < 0) {
        printf("failed to fork");
        return false;
    }
    else if (child == 0) {
        for (j=0; j<i; j++) {
            temp[j] = tokens[j];
        }
        
        if (flag == 1) {
            fd = open(tokens[i+1],O_WRONLY | O_TRUNC | O_CREAT,0664);
            if(fd < 0) {
                perror("failed to open file");
            }
            close(STDOUT_FILENO);
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        
//        printf("%s %s \n", temp[0], temp[1]);
        
        execvp(tokens[0], temp);                    // 프로그램 실행
        exit(0);
    }
    else {
        if (back == false) {
            waitpid(child, &status, 0);
        }
        else {
            waitpid(child, &status, WNOHANG);
        }
    }
    
    return true;
}

int main()
{
    char cmd[128];
    
    while(1)
    {
        getcwd(cmd, sizeof(cmd));             // 현재 위치값 받아옴
        printf("%s $ ", cmd);
        fgets(cmd, sizeof(cmd) - 1, stdin);
        if (!strcmp(cmd, "exit\n"))           // 명령어가 exit이면 myshell 종료
            break;
        
        run(cmd);                             // myshell 구동
    }
    
    return 0;
}
