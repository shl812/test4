/**
* system programming HW4
* myshell.c  : myshell example program
* @author    : ����Ʈ�����а� 32213932 ������
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

//��ūȭ(parsing) �Լ� ����
int my_tokenize(char* buf, char* tokens[]) {
    char* token;
    int tokenCount = 0;

    //strtok �Լ��� ����, buf�� ����Ǿ� �ִ� ����(��ɾ�, ����...)��
    //token�� ������
    token = strtok(buf, " ");

    //���� token�� NULL�� �ƴ϶��,
    //��, buf�� ���𰡰� �ԷµǾ��ٸ�,
    while (token != NULL) {
        //token�� ����Ǿ��ִ� �͵���
        //tokens �迭�� �����ϴµ� �̶� tokenCount�� 0���� �ϳ��� ������Ŵ
        //while���� ���� ��� token�� NULL�� �ɶ����� �ݺ���
        tokens[tokenCount++] = token;
        //token�� ����� �͵��� tokens �迭�� ��� �ű�� ����
        //token�� NULL�� ������� while�� ����
        token = strtok(NULL, " ");
    }
    //tokenCount�� ���� ��ȯ�ϸ�
    //my_tokenize �Լ� ����
    //�� �Լ��� ���� buf�� token ������ ��ȯ��
    tokens[tokenCount] = NULL;
    return tokenCount;
}

//redirection �Լ� ����
//��ūȭ�� �͵��� ����� �ִ� �迭�� ���ڷ� ����
void my_redir(char* token[]) {
    pid_t pid;
    int fd_redir;
    //���� fork �� ���� pid�� 0�̶��,
    //��, child ���μ������,
    if ((pid = fork()) == 0) {
        //token[3]�� ������ ���� fd_redir�� ����Ŵ
        fd_redir = open(token[3], O_RDWR | O_CREAT | O_TRUNC, 0641);
        //ǥ������� fd_redir�� ����
        dup2(fd_redir, STDOUT_FILENO);
        close(fd_redir);
        execvp(token[0], token);
        exit(0);
    }
    //parent ���μ����� wait()
    wait();
}

//background process ����
//��ūȭ�� �͵��� ����� �ִ� �迭�� ���ڷ� ����
void my_bgprocess(char* token[]) {
    pid_t pid;
    int fd_bg;
    //child ���μ���
    if ((pid = fork()) == 0) {
        //'/dev/null'�� ����ִ� ����
        //����ִ� ���Ͽ��� ����ǵ��� ��
        fd_bg = open("/dev/null", O_RDONLY);
        //fd_bg�� �������� ���� ǥ�� ������� ����
        dup2(fd_bg, STDOUT_FILENO);
        //token�� token[0]�� ��ο� ���
        execvp(token[0], token);
        exit(0);
        //background���� ����Ǿ���ϹǷ�
        //wait() �Լ� ������� ����
    }
}

//���� �Լ� ����
bool my_run(char* cmd) {
    int tokenCount;
    int i = 0;
    char* tokens[MAX];
    pid_t pid;
    int flag = 0;

    //cd �Լ� ����
    //��ū�� ������ ��ū�� ���� ���� argc, argv�� ����
    bool my_cd(int argc, char* argv[]) {
        //���� ��ū�� ������ 2�����,
        //�ι�° ��ū(��θ� �ش�)���� ��θ� �ű�
        if (argc == 2) {
            chdir(tokens[1]);
        }
        //���� ��ū�� ������ 1�����,
        //HOME���� ��θ� �ű�
        else if (argc == 1) {
            chdir(getenv("HOME"));
        }
        //��ū�� ������ 1�� �Ǵ� 2���� �ƴ϶��,
        //������ �߸��Ǿ����� �˸��� ���� ���
        else {
            printf("USAGE : %s file_name \n", argv[0]);
            exit(-1);
        }
    }

    //chdir �Լ����� ������ ���� -1 ��ȯ��
    //���� chdir���� ���� ��, perror ���
    void cd_err(char* token) {
        if (chdir(token) == -1) perror("cd");
        //������ �߻����� ������
        //cd �Լ� ����
        //��ū�� ������ �Էµ� ��ū�� �迭�� ����� �ִ� ���� ����
        else {
            my_cd(tokenCount, tokens);
        }
    }

    //��ūȭ�Ͽ� tokenCount�� ����
    tokenCount = my_tokenize(cmd, tokens);

    //cd ��ɾ �޾��� ���,
    //cd [dir]���� [dir]�� �ش��ϴ� �κ��� cd_err�� �Ѱ��־�
    //�ش� ��ɾ��� ���ڰ��� ����� �ԷµǾ����� Ȯ��
    if (strcmp(tokens[0], "cd") == 0) {
        cd_err(tokens[1]);
        return 0;
    }

    //��ū �ϳ��� ������Ű�� ��
    //redirection�� background processing�� �����ϱ� ����
    for (i = 0; i < tokenCount; i++) {
        //���� tokens �߿� ">"�� ��ġ�ϴ� �κ��� �ִٸ�
        //flag�� �ϳ� ������Ű��
        //redirection ����
        if (!strcmp(tokens[i], ">")) {
            flag++;
            my_redir(tokens);
        }
        //���� tokens �߿� "&"�� ��ġ�ϴ� �κ��� �ִٸ�
        //flag�� �ϳ� ������Ű��
        //background processing ����
        if (!strcmp(tokens[i], "&")) {
            flag++;
            my_bgprocess(tokens);
        }
    }

    //flag�� 1�� �ƴ϶��,
    //��, redirection�� background processing�� ������� �ʴ´ٸ�,
    if (!flag) {
        //child ���μ���
        if ((pid = fork()) == 0) {
            execvp(tokens[0], tokens);
            exit(0);
        }
        //parent ���μ����� wait()
        wait();
    }
    //tokens�� null ���� ����Ű���� ����
    memset(tokens, '\0', MAX);
    return 1;
}

int main() {
    //�Է¹��� ��ɾ cmd�� ����
    char cmd[MAX];
    //���� �׻� ����ǰ� �־���ϱ� ������ while(1)�� ���� �ݺ��� ����
    while (1) {
        //���� ù �κп� ����� ����
        printf("%d mysh$ ", get_current_dir_name());

        //��ɾ �޾ƿ�
        fgets(cmd, sizeof(cmd) - 1, stdin);

        //exit ��ɾ� ����
        //�Էµ� ��ɾ exit�� ���, �� ���� ���� ���
        //break�� ���� ���� �ݺ����� �ߴ���
        if (strncmp("exit", cmd, 4) == 0) {
            printf("exit myshell\n");
            break;
        }
        my_run(cmd);
    }
}