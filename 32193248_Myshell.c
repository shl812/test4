/*myshell program file, by LeeSeungjun, leo970623@gmail.com*/
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_SIZE 1024
#define ERROR (-1)

void menu() {
  printf("------------------------------------------------\n");
  printf("------SYSTEM PROGRAMMING - MAKING SHELL---------\n");
  printf("-------SOFTWARE - 32193248 SEUNGJUN LEE---------\n");
  printf("-----WRITE DOWN 'quit' TO FINISH THIS SHELL-----\n");
  printf("--WRITE DOWN '&' TO SHOW BACKGROUND PROCESSING--\n");
  printf("-------WRITE DOWN '>' TO SHOW REDIRECTION-------\n");
  printf("------------------------------------------------\n");
}

int tokenize(char *buf, char *delims, char *toks[], int maxT) {
  int t_count = 0;
  char *tok;
  tok = strtok(buf, delims);
  while (tok != NULL && t_count < maxT) {
    toks[t_count] = tok;
    t_count += 1;
    tok = strtok(NULL, delims);
  }
  toks[t_count] = NULL;
  return t_count;
}

bool run(char *line) {
  bool bg = false;
  int re = 0;
  int fd;
  int i;
  int status;
  int t_count;
  int path;
  char delims[] = " \n";
  char *toks[256];
  pid_t pid;

  t_count = tokenize(line, delims, toks, sizeof(toks) / sizeof(char *));
  if (t_count == 0)
    return true;
  if (strcmp(toks[0], "quit") == 0)
    return false;
  if (strcmp(toks[0], "start") == 0) {
    menu();
    return true;
  }
  for (i = 0; i < t_count; i++) {
    if (strcmp(toks[i], "&") == 0) {
      bg = true;
      toks[i] = NULL;
      break;
    }
    if (strcmp(toks[i], ">") == 0) {
      re = i;
      toks[i] = NULL;
      break;
    }
    if (strncmp(toks[0], "cd", 2) == 0) {
      path = chdir(toks[1]);
      if (path == 0)
        return true;
      else {
        printf("path error\n");
        return false;
      }
    }
  }

  pid = fork();

  if (pid < 0) {
    printf("fork error\n");
    _exit(1);
  } else if (pid == 0) {
    if (re) {
      fd = open(toks[re + 1], O_WRONLY | O_APPEND | O_CREAT, 0664);
      close(STDOUT_FILENO);
      dup2(fd, STDOUT_FILENO);
    }
    execvp(toks[0], toks);
    printf("execvp error\n");
    _exit(0);
  } else {
    if (!bg)
      waitpid(pid, &status, 0);
  }
  return true;
}

int main() {
  char line[MAX_SIZE];
  printf("WRITE DOWN : start\n");
  while (true) {
    printf("%s $", get_current_dir_name());
    fgets(line, sizeof(line) - 1, stdin);
    if (run(line) == false)
      break;
  }
  return 0;
}

