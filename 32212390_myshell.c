/**
 * mysh.c : My simple shell
 * @author : Hwi Shin
 * @email : sh01224@naver.com
 * @version : 1.0
 * @date : 2022.11.25
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>


struct COMMAND {    
    char* name;
    char* desc;
    bool (*func)(int argc, char* argv[]);   
};

pid_t child = -1;   
int status;  

bool com_exit(int argc, char* argv[]);   
bool com_help(int argc, char* argv[]);   
void redirect(char* tokens[], int i);

struct COMMAND builtin_cmds[] =
{
    {"exit", "쉘을 종료합니다.", com_exit},
    {"quit", "쉘을 종료합니다.", com_exit},
    {"help", "명령어 목록을 출력합니다.", com_help},
    {"?", "명령어 목록을 출력합니다.", com_help},
};

bool com_exit(int argc, char* argv[]) {
    return false;
}

bool com_help(int argc, char* argv[]) {   
    int i;
    for(i = 0; i < sizeof(builtin_cmds) / sizeof(struct COMMAND); i++)
    {
        if(argc == 1 || strcmp(builtin_cmds[i].name, argv[1]) == 0)
            printf("%-10s: %s\n", builtin_cmds[i].name, builtin_cmds[i].desc);
    }
    return true;
}

int tokenize(char* buf, char* delims, char* tokens[], int maxTokens) {
    int token_count = 0;
    char* token;
    token = strtok(buf, delims);
    while(token != NULL && token_count < maxTokens) {
        tokens[token_count] = token;
        token_count++;
        token = strtok(NULL, delims);
    }
    tokens[token_count] = NULL;
    return token_count;
}

void redirect(char* tokens[], int i) {
    int fd;
    
    fd = open(tokens[i+1], O_RDWR | O_CREAT);
    
    if (fd < 0) {
        perror("error");
        exit(-1);
    }
    
    dup2(fd, STDOUT_FILENO);
    tokens[i] = '\0';
    tokens[i+1] = '\0';
    close(fd);
}

bool run(char* line) {
    char delims[] = " \r\t\n";
    char* tokens[128];
    int token_count;
    int i;
    bool back;
    
    for(i = 0; i < strlen(line); i++) {    
        if(line[i] == '&') {
            back = true;
            line[i] = '\0';
            break;
        }
    }
    
    token_count = tokenize(line, delims, tokens, sizeof(tokens) / sizeof(char*));
    if(token_count == 0)
        return true;
        for(i = 0; i < sizeof(builtin_cmds) / sizeof(struct COMMAND); i++) {
            if(strcmp(builtin_cmds[i].name, tokens[0]) == 0)
                return builtin_cmds[i].func(token_count, tokens);
        }

        if(child = fork() ==  0) {
            for(i = 0; i < token_count; i++) {
                if(*tokens[i] == '>') {
                    redirect(tokens, i);
                    break;
                }
            }

            execvp(tokens[0], tokens);
            printf("No such file\n");
            exit( 0 );
        }
        else if(child < 0)
        {
            printf("Failed to fork()!");
            exit(0);
        }
        else if(back == false) {
            wait(&status);
        }
        return true;
}

int main() {
    char line[1024];
    while(1)
    {
        printf(">> ");
        fgets(line, sizeof(line) - 1, stdin);
        if(run(line) == false)
            break;
    }
    
    return 0;
}