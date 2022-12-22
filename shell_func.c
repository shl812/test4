#include "shell.h"

char **tokenize(char *inputstring){ //문자열 token화 함수
        char **tokens; //토큰들을 저장할 배열
        char *token;
        int count = 0;
        inputstring[strlen(inputstring) - 1] = '\0'; //'\n'대신 '\0'으로 대체
        tokens = malloc(sizeof(char *) * MAX); //토큰들을 넣을 배열 동적 할당
        token = strtok(inputstring, " "); //입력받은 문자열을 " "기준으로 나누기

        while(token != NULL){
                tokens[count] = malloc(strlen(token) + 1); 
//'\0' 포함하기 위해서 +1
                strncpy(tokens[count], token, strlen(token) + 1); 
//tokens에 문자열 복사
                token = strtok(NULL, " "); 
                count++;
        }
        //tokens의 마지막은 NULL로 만들기
        tokens[count] = NULL;
        return tokens;
}

void execute(char **input){  //명령어를 실행하는 함수
        int fork_process;
        int stat;
        int i = 0;
        char *dir;

        if(!strcmp(input[0], "cd")){ //명령어가 'cd'일 경우
                if(input[1] == NULL){
                        printf("USAGE : cd directory\n"); 
//폴더의 이름이 없을 때 예외처리
                }
                chdir(input[1]); //'cd' 명령어 구현
        }
        else if(!strcmp(input[0], "pwd")){ //명령어가 'pwd'일 경우
//<unistd.h>에 있는 getcwd 함수를 사용하여 출력
                printf("%s\n", getcwd(dir, MAX)); 
        }
        else{
//'cd' , 'pwd' 함수가 아닐 때
                if((fork_process = fork()) == 0){ 
//input이 존재할 때 까지
                        while(input[i]){ 
//redirection일 때
                                if(!strcmp(input[i], ">")){ 
//redirection 함수로 실행
                                        redirection(input, i); 
                                }
//백그라운드 프로세싱일 때
                                else if(!strcmp(input[i], "&")){ 
//background 함수로 실행
                                        background(input, i); 
                                }
                                i++;
                        }
                        //명령어가 제대로 들어오지 않을 때 예외처리
                        if(execvp(input[0], input) == -1){ 
                            printf("%s : command not found\n", input[0]);
                                exit(1);
                        }
                }
                //fork 에러처리
                else if(fork_process < 0){
                        printf("fork error\n");
                        exit(1);
                }
                else 
                        wait(&stat);
        }
}

void redirection(char **input, int i){ //redirection 구현 함수
        char *to = input[i+1]; //출력이 될 파일
        char *from = input[i-1]; //출력할 파일
        int to_fd;

        to_fd = open(to, O_WRONLY | O_CREAT, 0641); //출력이 될 파일 open
        if(to_fd < 0 ){ //fork 에러
                printf("open error\n"); 
                exit(1);
        }
        //dup2 함수를 사용하여 기본 출력을 to_fd 쪽으로 가리키게함
        dup2(to_fd, STDOUT_FILENO); 
        // '>' 를 NULL로 처리
        input[i] = NULL; 
        //명령어 실행
        if(execvp(input[0], input) == -1){
                printf("No such command : %s\n", input[0]);
                exit(1);
        }
}

void background(char **input, int i){ //background 프로세싱 구현 함수
        input[i] = NULL;
        int fork_process;
        if((fork_process = fork()) == 0){ //자식프로세스 생성
                if(execvp(input[0], input) == -1){ 
                        printf("No such command : %s\n", input[0]);
                        exit(1);
                }
        }
        else if(fork_process<0){
                printf("fork error\n");
                exit(1);
        }
}

