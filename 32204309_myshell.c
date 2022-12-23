#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int is_redirection;                                                                              // 어떤 리다이렉션을 수행해야 하는지에 대한 정보를 저장

int cmd_help(int argc, char *argv[]) {                                                           // help 명령어 사용 시 호출
    printf("myshell 프로그램 사용법: Command [Option] [Arguments]\n");
    return 0;
}

int cmd_cd(int argc, char *argv[]) {                                                             // cd 명령어 사용 시 호출
    if(argc == 1) {                                                                              // "cd"만 입력했을 경우 "/workspace"로 이동
        chdir("/workspace");
    }
    else if(argc == 2) {
        if(strcmp(argv[1], "--help") == 0) {                                                     // "--help"를 옵션으로 주었을 때
            printf("cd: cd[-L|[-P [-e]] [-@]] [dir]\n\tChange the shell working directory.\n");  // 설명 출력
        }
        else if(chdir(argv[1]) == -1) {                                                          // 주어진 위치로 이동하고 만약 해당하는 디렉터리가 없을 시
            printf("bash: cd: %s: 그런 파일이나 디렉터리가 없습니다\n", argv[1]);                   // 오류 발생을 알림
        }
    }
    else {                                                                                       // 명령어가 올바르게 사용되지 않으면
        printf("bash: cd: 인수가 너무 많음");                                                     // 오류 발생을 알림
    }
    return 0;
}

int background_run(char *line) {                                                                 // 백그라운드 실행 요청 파악을 위한 함수
    int i;
    for(i = 0; i < strlen(line); i++) {
        if(line[i] == '&') {                                                                     // 명령어에 '&' 문자가 포함되어있다면
            line[i] = ' ';                                                                       // 공백으로 바꾼 후
            return 1;                                                                            // 백그라운드에서 실행하라는 의미에서 1 반환
        }
    }
    return 0;                                                                                    // 백그라운드 실행 요청이 없을 시 0 반환
}

char *check_redirection(char *line) {                                                            // 리다이렉션 요청 여부를 파악하고 요청이 있을 시 리다이렉션 대상 파일을 반환하는 함수
    char *destination;
    int fd, i;

    for(i = strlen(line); i >= 0; i--) {                                                         // 명령어 끝에서 부터 검사
        if(line[i] == '>') {
            if(line[i - 1] == '>') {                                                             // ">>" 일 경우 1
                is_redirection = 1;
                destination = strtok(&line[i + 1], " \n");                                       // 리다이렉션 대상 파일을 찾아 저장
                line[i - 1] = '\0';                                                              // '>' 문자를 '\0'으로 대체해 일반적인 명령어로 변환
            }
            else {                                                                               // ">" 일 경우 2
                is_redirection = 2;
                destination = strtok(&line[i + 1], " \n");                                       // 리다이렉션 대상 파일을 찾아 저장
                line[i] = '\0';                                                                  // '>' 문자를 '\0'으로 대체해 일반적인 명령어로 변환
            }
            return destination;                                                                  // 리다이렉션 대상 파일 반환
        }
        else if(line[i] == '<') {                                                                // "<" 일 경우 3
            destination = strtok(&line[i + 1], " \n");                                           // 리다이렉션 대상 파일을 찾아 저장
            is_redirection = 3;
            line[i] = '\0';                                                                      // '<' 문자를 '\0'으로 대체해 일반적인 명령어로 변환
            return destination;                                                                  // 리다이렉션 대상 파일 반환
        }
    }
    is_redirection = 0;                                                                          // 리다이렉션을 요청하지 않았다면 0
    destination = NULL;                                                                          // 리다이렉션 대상 파일은 NULL
    return destination;                                                                          // NULL 반환
}


int tokenize(char *buf, char *delims, char *tokens[], int maxTokens) {                           // 구문 분석을 위한 함수
    int token_count = 0;                                                                         // 토큰 개수

    char *token = strtok(buf, delims);                                                           // 주어진 문자열을 특정 문자를 기준으로 잘라서 저장
    
    while(token != NULL && token_count < maxTokens) {                                            // 반복문을 통해 모든 단어를 tokens 배열에 저장
        tokens[token_count] = token;
        token_count++;
        token = strtok(NULL, delims);
    }

    tokens[token_count] = NULL;                                                                  // 단어들이 저장된 배열에 끝엔 NULL을 저장

    return token_count;                                                                          // 토큰 개수 반환
}

int run(char *line) {                                                                            // 명령어 실행을 위한 함수
    char *tokens[1024];
    pid_t child;                                                                                 // 주어진 명령어를 위한 자식 프로세스
    int fd;                                                                                      // 리다이렉션 수행 시 필요한 파일 디스크립터

    int is_background_run = background_run(line);                                                // 백그라운드 실행 요청 여부 파악

    char *destination = check_redirection(line);                                                 // 리다이렉션 요청 여부 파악 후 리다이렉션 대상 파일 저장

    int token_count = tokenize(line, " \n", tokens, sizeof(tokens) / sizeof(char *));            // 주어진 명령어를 실행하기 위해 문자열 파싱

    if(tokens[0] == NULL) {                                                                      // 아무 명령도 주어지지 않았으면
        return 1;                                                                                // 1 반환
    }
    else if(strcmp(tokens[0], "cd") == 0) {                                                      // cd 명령이 주어졌으면
        cmd_cd(token_count, tokens);                                                             // cd 명령을 위한 함수 cmd_cd 함수 호출 후 1 반환
        return 1;
    }
    else if(strcmp(tokens[0], "exit") == 0) {                                                    // exit 명령이 주어졌으면
        return 0;                                                                                // 0 반환
    }
    else if(strcmp(tokens[0], "help") == 0) {                                                    // help 명령이 주어졌으면
        cmd_help(token_count, tokens);                                                           // help 명령을 위한 함수 cmd_help 함수 호출 후 1 반환
        return 1;
    }
    

    if((child = fork()) == 0) {
        if(is_redirection == 1) {                                                                // 리다이렉션 ">>" 일 경우 
            fd = open(destination, O_WRONLY | O_CREAT | O_APPEND, 0644);                         // 리다이렉션 대상 파일을 열음 (쓰기 권한으로 열며 파일의 마지막에서부터 이어쓰고 파일이 없을 시 생성)
            dup2(fd, STDOUT_FILENO);                                                             // STDOUT_FILENO에 fd 복제
            close(fd);                                                                           // fd를 닫음
        }
        else if(is_redirection == 2) {                                                           // 리다이렉션 ">" 일 경우
            fd = open(destination, O_WRONLY | O_CREAT | O_TRUNC, 0644);                          // 리다이렉션 대상 파일을 열음 (쓰기 권한으로 열며 파일이 없을 시 생성하고 원래 있던 내용을 지우고 처음부터 작성)
            dup2(fd, STDOUT_FILENO);                                                             // STDOUT_FILENO에 fd 복제
            close(fd);                                                                           // fd를 닫음
        }
        else if(is_redirection == 3) {                                                           // 리다이렉션 "<" 일 경우
            fd = open(destination, O_RDONLY);                                                    // 리다이렉션 대상 파일을 열음 (읽기 권한)
            dup2(fd, STDIN_FILENO);                                                              // STDIN_FILENO에 fd 복제
            close(fd);                                                                           // fd를 닫음
        }
        int check = execvp(tokens[0], tokens);                                                   // 주어진 명령 실행
        if(check == -1)                                                                          // 실행 실패 시 오류 발생을 알림
            printf("bash: %s: 명령어를 찾을 수 없음\n", tokens[0]);
    }
    else if(!is_background_run) {                                                                // 백그라운드 실행이 아니라면 자식 프로세스가 끝날 때까지 기다림
        wait();
    }

    return 1;
}

int main() {
    char line[1024];                                                                             // 명령어를 담을 공간
    while(1) {
        printf("%s $ ", get_current_dir_name(line, 1024));                                       // 현재 디렉터리 위치와 함께 프롬프트 출력
        fgets(line, sizeof(line) - 1, stdin);                                                    // 명령어를 입력 받음
        if(run(line) == 0) break;                                                                // 주어진 명령어 실행
    }
}
