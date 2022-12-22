/* �ý������α׷��� 4�� ����
name : �̰��� 
Student ID : 32192987 
date : 2022-12-08  */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>   
#include <fcntl.h>  
#include <unistd.h> // dup2 �Լ� ����� ���� ���̺귯��
#include <stdbool.h> // bool ����� ���� ���̺귯��
#include <string.h> // str���� �Լ� ����� ���� ���̺귯��

#define MAX_SIZE 1024 // 1024ũ�⸦ �����ϴ� MAX_SIZE

void cmd_help(); // �� ��ɾ �������ִ� �Լ�
int tokenize(char *buf, char *delims, char *tokens[], int maxTokens); // �Է¹��� ������ �������ִ� �Լ�
void change_Dir(char *Dirname[]); // ���� �۾� ���丮�� �������ִ� �Լ�
bool run(char *line); // ��ɾ �����ϴ� �Լ�

int main() {
    char line[MAX_SIZE];
    while (1) {
        printf("%s$ ", getcwd(line, MAX_SIZE));  //getcwd�Լ��� ����ؼ� ���� ���丮�� �˷��ְ� �����ϸ� -1 ��ȯ
        fgets(line, sizeof(line) - 1, stdin); // �����ϸ� string ���۸� ����Ű�� �����͸� �����Ѵ�.
        if (run(line) == false) break; //run �Լ����� ���� ���� ���� false�� ����
    }
    return 0;
}

void cmd_help() { // �� ��ɾ �������ִ� �Լ�
    printf("help : help�� �ٽ� �����ش�.\n");
    printf("exit : ���� ����.\n");
    printf("> : �����̷���\n");
    printf("& : background processing\n");
}

int tokenize(char *buf, char *delims, char *tokens[], int maxTokens) {
	int token_Count = 0;
	char *token;
	token = strtok(buf, delims); // strtok �Լ��� ����ؼ� delims�� �������� buf���� ���ڿ� �ڸ��⸦ �Ѵ�.
	while (token!=NULL && token_Count<maxTokens) { // ��ū�� ������ �б� ������
		tokens[token_Count] = token; //token�� �� �ε����� ����
		token = strtok(NULL, delims); // ��ɾ �и���Ŵ
		token_Count=token_Count+1; // token_Count���� �ϳ� �����Ѵ�.
	}
	tokens[token_Count] = '\0'; // ������ �ε��� �� ���� �߰�
	return token_Count;
}

void change_Dir(char *Dirname[]) {
	char *path; // ��� ����
	char buf[MAX_SIZE]; // �ӽ� ���� ����
	
	path = getcwd(buf, MAX_SIZE); //getcwd�Լ��� ����ؼ� ���� ���丮�� �˷��ְ� �����ϸ� -1 ��ȯ
	if (path != NULL) { // ��ΰ� ������� �ʴ´ٸ�
		path = strcat(path, "/"); // path �ڿ� /�� ���δ�.
		path = strcat(path, Dirname[1]); // path �ڿ� ���� ��θ� ���δ�. 
		chdir(path); //chdir �Լ��� ����ؼ� ���� �۾� ���丮�� ����.
	}
}

bool run(char *line) {
	char *cmd[MAX_SIZE]; // ��ɾ ���� ���� �迭
	int i, index = 0;
	int fork_result; // fork���� ������ ����
	// �ش� �۾��� �� �� �����ϴ� �÷��� ���� ���� 
	bool bd_flag = false; // ��׶���
	bool redir_flag = false; // �����̷���
	// �����̷��� ��ġ�� �����ϱ� ���� ����
	int redirIndex = -1;

	int t_Cnt = tokenize(line, " \n", cmd, (sizeof(cmd) / sizeof(char *))); // tokenize�Լ��� ȣ���� �� ���� t_Cnt 

	if (t_Cnt == 0) { return true; }
	else if (strcmp(cmd[0], "help") == 0) { // help�� �Է��ϸ� cmd_help�Լ� ����
		cmd_help();
		return true;
	}
	else if (strcmp(cmd[0], "cd") == 0 && t_Cnt == 2) { // cd�� �Է��ϸ� change_Dir�Լ� ����
		change_Dir(cmd);
		return true;
	}
	else if (strcmp(cmd[0], "exit") == 0) { return false; } // exit�� �����ϸ� false ��ȯ
	
	for (i = 0; i<t_Cnt; i++) {
		if (strcmp(cmd[i], "&") == 0) {
			bd_flag = true; // ��׶��� �� true
			cmd[i] = NULL; // &�� ��ġ�� NULL�� �ٲ�
		}
		else if (strcmp(cmd[i], ">") == 0) {
			redir_flag = true; // �����̷��� �� true
			redirIndex = i + 1; // > ������ argument�� ������ ����
			cmd[i] = NULL; // >�� ��ġ NULL�� �ٲ�
		}
	}

	if ((fork_result = fork()) < 0) {
		perror("fork ����"); // perror�Լ��� ���� ���� �޽����� ����Ѵ�.
		printf("���� �����մϴ�.\n");
		exit(1);
	}
	
	else if (fork_result == 0) {
		if (redir_flag == true) {	
			int fd = open(cmd[redirIndex], O_RDWR | O_CREAT, 0664); // ���� ��ũ���Ϳ� ������ ���� �̸��� �޾� ������ ����
			dup2(fd, 1); // �� �ִ� ���� �ڿ� �ִ� ���� ����(��� ���)
			close(fd); // fd ���� ����
		}
		execvp(cmd[0], cmd); // PATH�� ��ϵ� ���丮�� �ִ� ���α׷��� ����
		exit(1);
	}
	
	else { // ��׶��� ���� false�� �θ� ���μ��� wait() ����
		if (bd_flag == false) { wait(); }
	}
	return true;
}