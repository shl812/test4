//32194579 최민석
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#define MAX_BUF 64
//int argc, char **argv

int Tokenizer(char *buf, char *token[]){
    //buf 파싱
    int count = 0;
    char *ptr = strtok(buf, " ");
    token[count++] = ptr;
    while(1){
        ptr = strtok(NULL, " ");
        if(ptr == NULL){
            token[count] = NULL;
            break;
        }
        else{
            token[count++] = ptr;
        }
    }//buf 파싱 종료
    return count; //토큰 개수 리턴
}

void ChangeDirectory(char *cd){
    if(chdir(cd) == -1){
       printf("%s with errno %d", strerror(errno), errno);
    }
}

void Redirection(char path[], char *tokens[], int cursor){//출력리다이렉션>
    //path : /bin/ls
    //tokens : ls > list.txt
    pid_t pid;
    int fd;
    int i;
    int n = 0;
    char *before_tokens[100];
    char *after_tokens[100];
    memset(before_tokens, 0x00, 100); //buf초기화
    memset(after_tokens, 0x00, 100); //buf초기화
    
    for(i = 0 ; i < cursor ; i++){
        before_tokens[i] = tokens[i];
    }
    for(i = cursor + 1 ; tokens[i] != NULL ; i++){
        after_tokens[n++] = tokens[i];
    }
    pid = fork();
    if(pid < 0){
        printf("fork error\n");
    }
    if(pid == 0) {
        fd = open(after_tokens[0], O_RDWR | O_CREAT | O_TRUNC, 0641);
        dup2(fd, STDOUT_FILENO); //표준입출력 대신 fd에 쓰기
        close(fd);
        if(execve(path, before_tokens, NULL) == -1){
            printf("Command execution is failed\n");
            exit(0);
        }
    }
    wait(NULL);
}

void DoubleRedirection(char path[], char *tokens[], int cursor){//출력리다이렉션>>APPEND
    //path : /bin/ls
    //tokens : ls > list.txt
    pid_t pid;
    int fd;
    int i;
    int n = 0;
    char *before_tokens[100];
    char *after_tokens[100];
    memset(before_tokens, 0x00, 100); //buf초기화
    memset(after_tokens, 0x00, 100); //buf초기화
    
    for(i = 0 ; i < cursor ; i++){
        before_tokens[i] = tokens[i];
    }
    for(i = cursor + 1 ; tokens[i] != NULL ; i++){
        after_tokens[n++] = tokens[i];
    }
    pid = fork();
    if(pid < 0){
        printf("fork error\n");
    }
    if(pid == 0) {
        fd = open(after_tokens[0], O_RDWR | O_CREAT | O_APPEND, 0641);
        dup2(fd, STDOUT_FILENO); //표준입출력 대신 fd에 쓰기
        close(fd);
        if(execve(path, before_tokens, NULL) == -1){
            printf("Command execution is failed\n");
            exit(0);
        }
    }
    wait(NULL);
}


void RedirectionIn(char path[], char *tokens[], int cursor){//입력 리다이렉션
    //path : /bin/ls
    //tokens : ls < list.txt
    pid_t pid;
    int fd;
    int i;
    int n = 0;
    char *before_tokens[100];
    char *after_tokens[100];
    memset(before_tokens, 0x00, 100); //buf초기화
    memset(after_tokens, 0x00, 100); //buf초기화
    
    for(i = 0 ; i < cursor ; i++){
        before_tokens[i] = tokens[i];
    }
    for(i = cursor + 1 ; tokens[i] != NULL ; i++){
        after_tokens[n++] = tokens[i];
    }
    pid = fork();
    if(pid < 0){
        printf("fork error\n");
    }
    if(pid == 0) {
        fd = open(after_tokens[0], O_RDONLY);
        dup2(fd, STDIN_FILENO); //표준입출력 대신 fd로 읽기
        close(fd);
        if(execve(path, before_tokens, NULL) == -1){
            printf("Command execution is failed\n");
            exit(0);
        }
    }
    wait(NULL);
}

void BackgroundExec(char path[], char *tokens[], int cursor){//백그라운드 실행 &
    printf("Background Exec:::\n");
    pid_t pid;
    tokens[cursor] = NULL;
    pid = fork();
    if(pid < 0){
        printf("fork error\n");
    }
    if(pid == 0) {
        if(execve(path, tokens, NULL) == -1){
            printf("Command execution is failed\n");
            exit(0);
        }
    }
    //no wait()
}
    
void Execute(char path[], char *tokens[], int tokencount){
    pid_t pid;
    int flag = 0;
    int i = 0;
    strcat(path, tokens[0]); // "/bin/" 뒤에 명령어 붙이기   ex) "/bin/ls"
    //cd 명령어
    if(strcmp(tokens[0], "cd") == 0){
        ChangeDirectory(tokens[1]);
        return;
    }
    if((pid = fork()) < 0){
        printf("fork error\n");
        exit(0);
    }
    else if(pid == 0){ //자식프로세스
        //execlp(buf,buf,NULL) to
        //execve(path, argv, NULL)
        for(i = 0 ; i < tokencount ; i++){
            if(strcmp(tokens[i],">") == 0 && flag == 0){
                Redirection(path, tokens, i);
                flag++;
            }
            else if(strcmp(tokens[i],">>") == 0 && flag == 0){
                DoubleRedirection(path, tokens, i);
                flag++;
            }
            else if(strcmp(tokens[i],"<") == 0 && flag == 0){
                RedirectionIn(path, tokens, i);
                flag++;
            }
            else if(strcmp(tokens[i],"&") == 0 && flag == 0){
                BackgroundExec(path, tokens, i);
                flag++;
            }
            if(flag != 0){
                break;
            }
        }
        if(flag == 0){
            if(execve(path, tokens, NULL) == -1){
                printf("Command execution is failed\n");
                exit(0);
            }
        }
        
        
    }
    if(pid > 0){
        wait(NULL);
    }
}

int main(){
    char buf[MAX_BUF];
    char path[100] = "/bin/";
    char *argv[100];
    int tokencount;
    printf("32194579_MINSEOK CHOI\n");
    printf("+::welcome to myShell 1.0::+ \n");
    while(1)
    {
        printf("@ ");
        memset(buf, 0x00, MAX_BUF); //buf초기화
        memset(argv, 0x00, 100);
        memset(path, 0x00, 100);
        strcpy(path, "/bin/");
        fgets(buf, MAX_BUF-1, stdin); //표준입출력에 쓰기
        //사용법 : char* fgets(char* str, int num, FILE* pFile);
        if(strncmp(buf, "exit\n", 5) == 0){
            break;
        }
        buf[strlen(buf)-1] = 0x00; //마지막 enter코드 버퍼에서 제외
        tokencount = Tokenizer(buf, argv); //buf내용을 토큰화해서 argv에 삽입
        Execute(path, argv, tokencount);
        
    }
    return 0;
}
