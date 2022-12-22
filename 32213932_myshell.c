/**
* system programming HW4
* myshell.c  : myshell example program
* @author    : 소프트웨어학과 32213932 장혜림
* @email     : hyerim1148@naver.com
* date       : 2022.12.09
**/

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#define MAX 1024

//토큰화(parsing) 함수 구현
int my_tokenize(char* buf, char* tokens[]) {
    char* token;
    int tokenCount = 0;

    //strtok 함수를 통해, buf에 저장되어 있는 문장(명령어, 인자...)을
    //token에 저장함
    token = strtok(buf, " ");

    //만약 token이 NULL이 아니라면,
    //즉, buf에 무언가가 입력되었다면,
    while (token != NULL) {
        //token에 저장되어있는 것들을
        //tokens 배열에 저장하는데 이때 tokenCount를 0부터 하나씩 증가시킴
        //while문을 통해 계속 token이 NULL이 될때까지 반복함
        tokens[tokenCount++] = token;
        //token에 저장된 것들을 tokens 배열에 모두 옮기고 나서
        //token에 NULL을 저장시켜 while문 종료
        token = strtok(NULL, " ");
    }
    //tokenCount의 값을 반환하며
    //my_tokenize 함수 종료
    //이 함수를 통해 buf의 token 개수를 반환함
    tokens[tokenCount] = NULL;
    return tokenCount;
}

//redirection 함수 구현
//토큰화된 것들이 저장돼 있는 배열을 인자로 받음
void my_redir(char* token[]) {
    pid_t pid;
    int fd_redir;
    //만약 fork 된 값의 pid가 0이라면,
    //즉, child 프로세스라면,
    if ((pid = fork()) == 0) {
        //token[3]의 파일을 열어 fd_redir이 가리킴
        fd_redir = open(token[3], O_RDWR | O_CREAT | O_TRUNC, 0641);
        //표준출력을 fd_redir로 설정
        dup2(fd_redir, STDOUT_FILENO);
        close(fd_redir);
        execvp(token[0], token);
        exit(0);
    }
    //parent 프로세스는 wait()
    wait();
}

//background process 구현
//토큰화된 것들이 저장돼 있는 배열을 인자로 받음
void my_bgprocess(char* token[]) {
    pid_t pid;
    int fd_bg;
    //child 프로세스
    if ((pid = fork()) == 0) {
        //'/dev/null'은 비어있는 파일
        //비어있는 파일에서 실행되도록 함
        fd_bg = open("/dev/null", O_RDONLY);
        //fd_bg의 서술자의 값을 표준 출력으로 지정
        dup2(fd_bg, STDOUT_FILENO);
        //token을 token[0]의 경로에 출력
        execvp(token[0], token);
        exit(0);
        //background에서 실행되어야하므로
        //wait() 함수 사용하지 않음
    }
}

//실행 함수 구현
bool my_run(char* cmd) {
    int tokenCount;
    int i = 0;
    char* tokens[MAX];
    pid_t pid;
    int flag = 0;

    //cd 함수 구현
    //토큰의 개수와 토큰된 값을 각각 argc, argv로 받음
    bool my_cd(int argc, char* argv[]) {
        //만약 토큰의 개수가 2개라면,
        //두번째 토큰(경로명에 해당)으로 경로를 옮김
        if (argc == 2) {
            chdir(tokens[1]);
        }
        //만약 토큰의 개수가 1개라면,
        //HOME으로 경로를 옮김
        else if (argc == 1) {
            chdir(getenv("HOME"));
        }
        //토큰의 개수가 1개 또는 2개가 아니라면,
        //사용법이 잘못되었음을 알리는 문구 출력
        else {
            printf("USAGE : %s file_name \n", argv[0]);
            exit(-1);
        }
    }

    //chdir 함수에서 에러가 나면 -1 반환함
    //따라서 chdir에서 에러 시, perror 출력
    void cd_err(char* token) {
        if (chdir(token) == -1) perror("cd");
        //에러가 발생하지 않으면
        //cd 함수 실행
        //토큰의 개수와 입력된 토큰이 배열로 저장돼 있는 것을 전달
        else {
            my_cd(tokenCount, tokens);
        }
    }

    //토큰화하여 tokenCount에 저장
    tokenCount = my_tokenize(cmd, tokens);

    //cd 명령어를 받았을 경우,
    //cd [dir]에서 [dir]에 해당하는 부분을 cd_err에 넘겨주어
    //해당 명령어의 인자값이 제대로 입력되었는지 확인
    if (strcmp(tokens[0], "cd") == 0) {
        cd_err(tokens[1]);
        return 0;
    }

    //토큰 하나씩 증가시키며 비교
    //redirection과 background processing을 실행하기 위함
    for (i = 0; i < tokenCount; i++) {
        //만약 tokens 중에 ">"와 일치하는 부분이 있다면
        //flag를 하나 증가시키고
        //redirection 실행
        if (!strcmp(tokens[i], ">")) {
            flag++;
            my_redir(tokens);
        }
        //만약 tokens 중에 "&"와 일치하는 부분이 있다면
        //flag를 하나 증가시키고
        //background processing 실행
        if (!strcmp(tokens[i], "&")) {
            flag++;
            my_bgprocess(tokens);
        }
    }

    //flag가 1이 아니라면,
    //즉, redirection과 background processing이 실행되지 않는다면,
    if (!flag) {
        //child 프로세스
        if ((pid = fork()) == 0) {
            execvp(tokens[0], tokens);
            exit(0);
        }
        //parent 프로세스는 wait()
        wait();
    }
    //tokens가 null 값을 가리키도록 설정
    memset(tokens, '\0', MAX);
    return 1;
}

int main() {
    //입력받을 명령어를 cmd에 저장
    char cmd[MAX];
    //쉘은 항상 실행되고 있어야하기 때문에 while(1)로 무한 반복문 실행
    while (1) {
        //쉘의 첫 부분에 출력할 내용
        printf("%d mysh$ ", get_current_dir_name());

        //명령어를 받아옴
        fgets(cmd, sizeof(cmd) - 1, stdin);

        //exit 명령어 구현
        //입력된 명령어가 exit일 경우, 쉘 종료 문구 출력
        //break를 통해 무한 반복문을 중단함
        if (strncmp("exit", cmd, 4) == 0) {
            printf("exit myshell\n");
            break;
        }
        my_run(cmd);
    }
}