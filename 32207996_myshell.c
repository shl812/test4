/**
*mycat.c : my shell program
*@author: Haeyoung Hwang
*@email: 32207996sw@dankook.ac.kr
*@date : 2022.12.08
**/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdbool.h>
#define MAX 1024

int status;
int bg = 0, redir = 0;
pid_t child;

int tokenize(char* buf, char* delims, char* tokens[], int maxTokens) {	//��ū�� �������� ����
    int tokenCount = 0;	//��ū ����
    char* token;	//��ū ������ ���� �����ͺ���
    token = strtok(buf, delims);	//delims �������� buf�� ������ token�� ������
    while (token != NULL && tokenCount < maxTokens) {
        tokens[tokenCount] = token;	//tokens�� ��ū ����
        tokenCount++;	//��ū ���� 1�� ����
        token = strtok(NULL, delims);
    }
    tokens[tokenCount] = NULL;
    return tokenCount;	//���� ��ȯ
}

void redirection(char* token[]) { // �Է�, ��� ������
    pid_t pid;
    int fd_redir;

    fd_redir = open(token[3], O_RDWR | O_CREAT | O_TRUNC, 0641);
    dup2(fd_redir, STDOUT_FILENO);
    token[2] = NULL; //>�� �ΰ����� 
    close(fd_redir);
    execvp(token[0], token);
    exit(0);
}


bool run(char* line) {	//�� ����
    char* tokens[MAX];
    char delims[] = " \r\n\t";		//�̸� �������� ��ū ����
    int i, tokenCount, fd_redir;
    bool backgr;

    tokenCount = tokenize(line, delims, tokens, sizeof(tokens) / sizeof(char*));
    if (tokenCount == 0) //��ū�� ���ٸ�(�Է� ����)
        return true;	//��� �۵�

    //��ɾ� ���� 
    //cd 
    if (strcmp(tokens[0], "cd") == 0) {
        if (chdir(tokens[1]) != 0)	//token[1]�� ���丮 ����      
            perror("cd error");	//�����޼���       
        return;
    }
    //exit 
    if (strcmp(tokens[0], "exit") == 0)
        return false;
    //help
    if (strcmp(tokens[0], "help") == 0) {
        printf("==================================================\n");
        printf("My shell!!!\n");
        printf("Some examples of the built-in commands\n");
        printf("------------------------------------------\n");
        printf("cd    : ���丮 �����ϱ�\n");
        printf("exit  : �� ������\n");
        printf("help  : ����\n");
        printf("==================================================\n");
    }


    for (i = 0; i < tokenCount; i++) {
        //background processing
        if (strcmp(tokens[i], "&") == 0) {	//&�� �ִ� ���
            bg++;				//1�� ����
            backgr = true;			//�θ� ���μ������� ����ϱ� ����
            tokens[i] = '\0';	//&�� null�� ����
            break;
        }
        if (strcmp(tokens[i], ">") == 0) { // redirection ����
            redir++;
            redirection(tokens);
        }
    }

    child = fork();	//fork
    if (child == 0) {	//�ڽ� ���μ���(execve() ����)
        if (bg == 0) {	//redirection ���� ��׶��� =0 �� foreground�� ��
            if (redir == 0) {
                execvp(tokens[0], tokens);
                exit(0);
            }
        }
        else if (bg > 0) {	//background
            backgr = true;
            execvp(tokens[0], tokens);
            exit(0);
        }
        else if (redir > 0) {	//redirection
            fd_redir = open(tokens[i], O_RDWR | O_CREAT | O_TRUNC, 0644);

            if (fd_redir < 0) {
                perror("open fail\n");
                exit(1);
            }
            dup2(fd_redir, 1);
            close(fd_redir);
        }
    }

    else if (child < 0) {	//fork ����
        perror("fork error\n");
        exit(0);
    }

    else {	//�θ� ���μ���
        if (backgr == false) //background�� wait ����
            wait(&status);

    }

    return true;
}

//���� �Լ�
int main() {
    char line[MAX];	//�Է��� ������ �迭

    while (1) {	//���ѷ���
        printf("%s $ ", get_current_dir_name());
        fgets(line, sizeof(line) - 1, stdin);		//�Է¹ޱ�

        if (run(line) == false)
            break;
    }
    return 0;
}

