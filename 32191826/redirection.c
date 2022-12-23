//이름 : 박주희
//학번 : 32191826
//날짜 : 2022.12.09

#include "myshell.h"

void redirect(int flag, int is_bg, char *argv[], char* input, char * output){
    int input_fd, output_fd;
    int status, pid;
    
    if ((pid=fork()) == -1)
        perror("fork failed");
    else if (pid != 0) {
        if(is_bg==0)
            pid = wait(&status);
        else {
            printf("[1] %d\n", getpid());
            waitpid(pid, &status, WNOHANG);
        }
    } else {
        if (flag == 2) {
            if((input_fd = open(input, O_RDONLY))==-1){
                perror(argv[0]);
                exit(2);
            }
            dup2(input_fd, 0); // standard input
            close(input_fd);
            execvp(argv[0], argv);
        } else if (flag == 3) {
            output_fd = open(output, O_CREAT|O_TRUNC|O_WRONLY, 0600);
            dup2(output_fd, 1); // standard output
            close(output_fd);
            execvp(argv[0], argv);
        }
    }
}
