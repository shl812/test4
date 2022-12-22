/*
mysh.c : my shell program
author : Ji Woo Park
email : 32211862@dankook.ac.kr
date : 2022.12.09
*/

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdbool.h>
#define MAX 1024

// 키(공백)를 중심으로 나눠주는 함수
int tokenize(char* buf, char* delims, char* tokens[], int max) {
    int token_count = 0;
    char* token = strtok(buf, delims);
    // 명령어별로 자른 문자열을 배열에 저장
    while (token != NULL && token_count < max) {
        tokens[token_count] = token;
        token_count++;
        token = strtok(NULL, delims);
    }
    tokens[token_count] = NULL;
    return token_count;
}

//  cd를 입력했을 때 호출될 함수
void Inner_cmd_cd(char* token) {
    if (chdir(token) == -1) perror("cd"); // chdir로 cd 바꿔줌,  아니면 오류.
}

// mysh을 위한 도움말
void Help_shell() {
    printf("**********************wonjin's shell**********************\n");
    printf("'>'과 '&'를 포함한 쉘에서 사용할 수 있는 명령어\n");
    printf("대부분의 shell 기본 명령어 사용가능\n");
    printf("cd\t: 디렉토리 변경\n");
    printf("exit\t: mysh 나가기\n");
    printf("help\t: wonjin's shell을 위한 도움말\n");
    printf("**********************************************************\n");
    return;
}

// 입출력 재지정을 위한 함수, 명령어를 배열로 받음
void Redirection(char* token[]) {
    pid_t pid; // 프로세스의 id 를 받을 pid 변수
    int fd_rd;
    // 마지막 파일을 열어서 rw- r-- --x, readwite가능, 파일 생성, 있으면 안 만듦
    fd_rd = open(token[3], O_RDWR | O_CREAT | O_TRUNC, 0641);
    dup2(fd_rd, STDOUT_FILENO);
    token[2] = NULL; // '>'는 파일이 아니기 때문에 오류가 나옴.
    close(fd_rd);
    execvp(token[0], token); // 리눅스 명령어 실행
    exit(0);
}

// 명령어를 배열로 받음, Background process실행시켜 줌
void BackgroundExec(char* token[]) {
    pid_t pid;
    int fd_bg;
    // 오류 처리
    if ((pid = fork()) < 0) {
        perror("fork error\n");
        exit(-1);
    }

    // fork()했을 때, 자식프로세스 인 경우
    else if (pid == 0) {
        fd_bg = open("/dev/null", O_RDONLY); // 빈파일을 열어서 실행하고, 읽기 전용으로 파일열기
        dup2(fd_bg, STDOUT_FILENO); // 출력 재지정
        execvp(token[0], token); // 명령어 실행
        exit(0);
    }
}

// 명령어 리스트를 매개변수로 받은 Execute 함수
bool Execute(char* instruction) {
    int token_count, stat;
    int i = 0;
    int number = 0;
    char* tokens[MAX];
    char delims[] = " \n";
    pid_t pid;

    // 명령어 리스트와 빈 리스트를 인수 값으로 tokenize 호출
    token_count = tokenize(instruction, delims, tokens, sizeof(tokens) / sizeof(char*));
    // instruction과 tokens를 포인터로 넘기기 때문에 tokenize에서 변경 된 것이 저장 됨.

    // tokens에 저장된 명령어가 cd면 Inner_cmd_cd 실행
    if (strncmp(tokens[0], "cd", 2) == 0) {
        Inner_cmd_cd(tokens[1]); // InnerCmd 를 호출 1로 넘긴걸 받아옴
        return;
    }

    // tokens에 저장된 명령어가 help면 Help_shell 실행
    if (strncmp(tokens[0], "help", 2) == 0) {
        Help_shell();
    }

    pid = fork(); // 자식 프로세스 생성
    // 오류 처리
    if (pid < 0) {
        perror("fork error\n");
        exit(-1);
    }

    // 자식 프로세스인 경우 명령어 실행
    else if (pid == 0) {
        // redirection 실행 값이 같으면 0이므로 0이 아니면 true
        for (i = 0; i < token_count; i++) {
            if (!strcmp(tokens[i], ">")) { // redirection 실행
                number++;
                Redirection(tokens);
            }
            if (!strcmp(tokens[i], "&")) { // background 실행
                number++;
                BackgroundExec(tokens);
                exit(0);
            }
        }

        // 위의 조건문에서 해당하지 않는 다른 명령어들 실행
        if (number == 0) {
            execvp(tokens[0], tokens); // 명령어 실행
            exit(0);
        }
    }
    // 부모 프로세스인 경우 자식 프로세스가 끝날 때 까지 기다림
    else {
        waitpid(pid, &stat, 0);
    }

    memset(tokens, '\0', MAX); // 명령어 배열 초기화
    return 1;
}

// main 함수
int main() {
    char instruction[MAX]; // 명령어를 받아 줄 배열
    // 무한 루프. exit가 나올 때 까지 반복해서 실행
    while (1) {
        printf("%s# ", get_current_dir_name()); // "현재 디렉토리명 $" 출력
        fgets(instruction, sizeof(instruction) - 1, stdin); // 입력한 문자열 받아와서 instruction에 저장
        // exit를 입력하면 while문 빠져나감
        if (strncmp("exit", instruction, 4) == 0) {
            printf("exit mysh.c\n");
            break;
        }
        Execute(instruction); // 명령어를 Execute 함수로 넘겨 줌
    }
}
