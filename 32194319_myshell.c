/**
* mysh.c		: my shell program
* @author 		: SeongHyeon Cho
* @email 		: jcd0070@naver.com
* @date 		: 2022.12.09
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#define MAX_SIZE 1024

void cmd_help() {
        printf("/*************** mysh ***************/\n");
        printf("You can use it just as the convetional shell\n\n");
        printf("Some examples of the built-in commands\n");
        printf(">\t : redirection\n");
        printf("&\t : background processing\n");
        printf("cd\t : change directory\n");
        printf("exit\t : exit this shell\n");
        printf("quit\t : quit this shell\n");
        printf("help\t : show this help\n");
        printf("?\t : show this help\n");
        printf("/************************************/\n");
}

int tokenize(char* buf, char* delims, char* tokens[], int maxTokens) {
        int t_count = 0;
        char* token;
        token = strtok(buf, delims);
        while(token != NULL && t_count < maxTokens) {
                tokens[t_count++] = token;
                token = strtok(NULL, delims);
        }
        return t_count;
}

bool run(char* line) {
        int t_count;
        char* tokens[MAX_SIZE];
        char delims[] = " \n";
        int background = 0;
        int redirection = 0;
        int fd_background, fd_redirection;
        int fd, status, i;
        pid_t child;
        t_count = tokenize(line, delims, tokens, sizeof(tokens)/sizeof(char*));
        if(t_count == 0)
                return false;
// "cd"
        if(strcmp(tokens[0], "cd") == 0)
        {
                if(chdir(tokens[1]) == 0)
                        return true;
                else{
                        printf("cd error\n");
                        return false;
                }
        }

// "exit", "quit"
        if(strcmp(tokens[0], "exit") == 0)
                return false;
        if(strcmp(tokens[0], "quit") == 0)
                return false;

// "help", "?"
        if(strcmp(tokens[0], "help") == 0)
        {
                cmd_help();
                return true;
        }
        if(strcmp(tokens[0], "?") == 0)
        {
                cmd_help();
                return true;
        }

// "&", ">"
        for (i = 0; i < t_count; i++)
        {
                if(strcmp(tokens[i], "&") == 0)
                {
                        background++;
                        tokens[i] = NULL;
                        break;
                }
                if(strcmp(tokens[i], ">") == 0)
                {
                        redirection = i;
                        tokens[i] = NULL;
                        break;
                }
        }

        child =  fork();
        if(child < 0){
                perror("fork error");
                exit(1);
        }
        else if(child == 0){
                if(redirection){
                        close(STDOUT_FILENO);
                    		fd_redirection = open(tokens[redirection+1], O_RDWR | O_CREAT | O_TRUNC, 0641);
                        dup2(fd_redirection, STDOUT_FILENO);
                        close(fd_redirection);
                }
                execvp(tokens[0], tokens);
                exit(0);
        }
        else {
                if(!background)
                        waitpid(child, &status, 0);
        }
        return true;
}

int main() {
        char line[MAX_SIZE];
        while(1){
                printf("%s $", get_current_dir_name());
                fgets(line, sizeof(line) - 1, stdin);
                if(run(line) == false) break;
        }
        return 0;
}
