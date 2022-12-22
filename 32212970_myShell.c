#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <ctype.h>
#include <stdlib.h>
#define MAX 1024

char cwd[128];
int run_status = 1;
int wait_status = 1;

void redirection_in(char *fname){
    int fd_in = open(fname, O_RDONLY);
    dup2(fd_in, 0);
    close(fd_in);
}

void redirection_out(char *fname){
    int fd_out = open(fname, O_WRONLY | O_TRUNC | O_CREAT, 0600);
    dup2(fd_out, 1);
    close(fd_out);
}

void run(char *args[]){
    pid_t pid;
    if (strcmp(args[0],"help") == 0 || strcmp(args[0],"?") == 0 ) {
        printf("***************** Gaeun's Shell *****************\n");
        printf("You can use it just as the conventional shell\n\n");
        printf("Some examples of the built-in commands\n");
        printf("cd          : change directory\n");
        printf("exit        : exit this shell\n");
        printf("quit        : quit this shell\n");
        printf("help        : show this help\n");
        printf("?           : show this help\n");
        printf("************************************************\n"); 
    }   
    else if (strcmp(args[0], "exit") == 0 || strcmp(args[0], "quit") == 0){
        run_status = 0;
        exit(0);
    }
    else if (strcmp(args[0],"cd") == 0 ){
        chdir(args[1]);           
    }
    else {
        pid = fork();
        if (pid < 0) fprintf(stderr, "Fork Error");
        else if ( pid == 0) execvp(args[0], args);
        else {
            if (wait_status) {
                waitpid(pid, NULL, 0);
            } else {
                wait_status = 0;
            }
    }
    redirection_in("/dev/tty");
    redirection_out("/dev/tty");
    }
}
void pipe_func(char *args[]){
    int fd[2];
    pipe(fd);

    dup2(fd[1], 1);
    close(fd[1]);

    run(args);

    dup2(fd[0], 0);
    close(fd[0]);
}

char * tokenize(char *input){
    int i;
    int j = 0;
    char *tokenized = (char *)malloc((MAX * 2) * sizeof(char));
    for (i = 0; i < strlen(input); i++) {
        if (input[i] != '>' && input[i] != '<' && input[i] != '|') {
            tokenized[j++] = input[i];
        } else {
            tokenized[j++] = ' ';
            tokenized[j++] = input[i];
            tokenized[j++] = ' ';
        }
    }
    tokenized[j++] = '\0';

    char *end;
    end = tokenized + strlen(tokenized) - 1;
    end--;
    *(end + 1) = '\0';

    return tokenized;
}
int main(void)
{
    char *args[MAX];
    while (run_status) {
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
           printf("%s $ ", cwd);
        } else {
           perror("getcwd() error");
           return 1;
        };
        fflush(stdout);

        char input[MAX];
        fgets(input, MAX, stdin);

        char *tokens;
        tokens = tokenize(input);

        if (tokens[strlen(tokens) - 1] == '&') {
            wait_status = 0;
            tokens[strlen(tokens) - 1] = '\0';
        }
        char *arg = strtok(tokens, " ");
        int i = 0;
        while (arg) {
            if (*arg == '<') {
                redirection_in(strtok(NULL, " "));
            } else if (*arg == '>') {
                redirection_out(strtok(NULL, " "));
            } else if (*arg == '|') {
                args[i] = NULL;
                pipe_func(args);
                i = 0;
            } else {
                args[i] = arg;
                i++;
            }
            arg = strtok(NULL, " ");
        }
        args[i] = NULL;
        run(args);
    }
    return 0;
}