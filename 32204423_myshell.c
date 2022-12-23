#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>


//토큰화를 하는 함수
int tokenize(char* buffer, char* delimiter, char* tokens[], int max) {
    int count = 0;
    char* token;
    
    //토큰화를 함
    token = strtok(buffer, delimiter);
    while(token != NULL && count < max) {
        tokens[count] = token;
        count++;
        token = strtok(NULL, delimiter);
    }
    
    // parshing(토큰화를 통해 명령어를 구별)을 함
    tokens[count] = NULL;
    return count;
}

//shell을 작동하는 함수
bool run(char* command) {
    int fd;
    int redirection = 0;
    int background = 0;
    int re_check = 0; //redirection을 해야 하는 지 체크함
    int bg_check = 0; //background processing을 해야 하는 지 체크함
    int count = 0;
    pid_t child;
    char delimiter[] = " \r\t\n";
    char* token[100];
    
    count = tokenize(command, delimiter, token, sizeof(token)/sizeof(char*));

    if(count == 0) //입력이 없다면 true 리턴
        return true;
    if(strcmp(token[0], "exit") == 0) // 터미널에 exit를 입력하면 shell을 종료함
        return false;
   
    for(int i = 0; i < count; i++) { //redirection과 background processing 명령어가 있는지 체크
        if(strcmp(token[i],">") == 0) {
            redirection = i;
            re_check = 1;
            break;
        }
        if(strcmp(token[i],"&") == 0) {
            background = i;
            bg_check = 1;
            break;
        }

    }
    child = fork();
    if(child < 0) {  //자녀가 0보다 작다면 fork는 에러
        printf("fork error\n");
        return false;
    }
    else if(child == 0) { //자녀가 0이라면 fork가 잘 이루어진 것
        if(bg_check == 1) { //background processing이 일어났다면
            token[background] = NULL;
        }
        if(re_check == 1) {  //또한 redirection이 일어났다면 표준 입출력을 변경함
            fd = open(token[redirection + 1], O_WRONLY|O_TRUNC|O_CREAT, 0664);
            close(STDOUT_FILENO);
            dup2(fd, STDOUT_FILENO);
            token[redirection] = NULL;
        }
        execvp(token[0], token);
                
        return 0;
    }
    else if(bg_check == 0) {  //만약 background processing이 없다면 wait을 통해 부모가 자식을 기다림
        wait(NULL);
    }
    return true;
}


int main() {
    char command[1500];
    printf("my shell start\n");
    while(1) {
        printf("%s $ ", getcwd(command,1500));
        fgets(command, sizeof(command) - 1, stdin);
        if(run(command) == 0)
            break;
    }
    
    return 0;
}
