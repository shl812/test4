/*
    HW#4 Make a shell
    32191030 소프트웨어학과 김정우
*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

bool cmd_cd(int argc, char*argv[]);
bool cmd_quit(int argc, char*argv[]);
bool cmd_help(int argc, char*argv[]);

struct cmd_str {
    char* name;
    char* explain;
    bool (*func) (int argc, char* argv[]);
};

struct cmd_str command[5] = {
    {"help", "Show help menu", cmd_help},
    {"cd", "Change directory", cmd_cd},
    {"quit", "Quit program", cmd_quit},
    {"&", "Background processing"},
    {">", "Redirection"}
};

bool cmd_help (int argc, char* argv[])
{
    printf("--------------------My shell--------------------\n");
    printf("You can use it just as the conventional shell\n");
    printf("Some examples of the built-in commands\n");
    for (int i = 0; i < 5; i++)
        printf("%s: %s\n", command[i].name, command[i].explain);
    printf("------------------------------------------------\n");
    return true;
}

bool cmd_cd (int argc, char* argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <path name>\n", argv[0]);
    }
    if (chdir(argv[1]) == -1)
    {
        printf("failed, change directory\n");
    }
    return true;
}

bool cmd_quit (int argc, char* argv[])
{
    return false;
}

int tokenize (char* buf, char* delims, char* tokens[], int maxTokens)
{
    int t_count = 0;
    char* token;
    token = strtok(buf, delims);

    while (token != NULL && t_count < maxTokens) //token이 NULL일때까지 (= strtok 함수가 NULL을 반환할때까지)
    {
        tokens[t_count++] = token;
        token = strtok(NULL, delims);
    }
    tokens[t_count] = NULL;

    return t_count;
}

void cmd_redir (int argc, char* argv[])
{
    int fd;
    if (argc == 4)
    {
        fd = open(argv[3], O_WRONLY | O_CREAT, 0664);
        if (fd < 0)
        {
            printf("file open errno : %s\n", argv[3], errno);
            exit(-1);
        }
        dup2(fd, STDOUT_FILENO);
        argv[2] = NULL;
    }
    close(fd);
    return;
}

bool run (char *line)
{
    char delims[] = " \n";
    int token_count;
    char *tokens[128];
    pid_t pid;
    int stat;

    bool bp = false;
    bool redir = false;
    char* check_bp = strchr(line, '&');
    char* check_redir = strchr(line, '>');
    if (check_bp != NULL) bp = true;
    if (check_redir != NULL) redir = true;

    token_count = tokenize(line, delims, tokens, sizeof(tokens) / sizeof(char*));
    if (token_count == 0)   return true;

    for (int i = 0; i < 3; i++)
    {
        if (strcmp(command[i].name, tokens[0]) == 0)
            return command[i].func(token_count, tokens);
    }

    if ((pid = fork()) < 0)
    {
        perror("fork error");
        exit(-1);
    }
    else if (pid == 0)
    {
        if (redir == true)
            cmd_redir(token_count, tokens);
        execvp(tokens[0], tokens);
    }

    if (bp == false)
        waitpid(pid, &stat, 0);
    else
        waitpid(pid, &stat, WNOHANG);
    
    return true;
}

int main()
{
    char line[1024];

    while (1)
    {
        printf(" %s : KJW Mysh $ ", get_current_dir_name());
        fgets(line, sizeof(line) - 1, stdin);
        if (run(line) == false)
            break;
    }

    return 0;
}