#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
int main(void) {
	char buf[256];
	int i, fd;
	pid_t pid;
	char* path = "/bin/";
	char* cmd[256];
	char* str;
	char* status;
	while (1) {
		printf(">>> ");
		//�Է¹��� ���� '\0'�� ä���
		for (i = 0; i < 256; i++) {
			buf[i] = '\0';
		}
		//��ɾ �Է¹ް� ���� '\0'���� �ٲ۴�
		fgets(buf, sizeof(buf), stdin);
		buf[255] = '\0';
		i = 0;
		//�Է¹��� ���� \t\r\n���� �и����� cmd�� �Է��Ѵ�
		str = strtok_r(buf, "\t\r\n", &status);
		while (str != NULL && i < 256) {
			cmd[i++] = str;
			str = strtok_r(NULL, "\t\r\n", &status);
		}
		cmd[i] = NULL;
		//cmd[0]�� q�̸� �����Ѵ�
		if (strcmp(cmd[0], "q") == 0)
			exit(1);
		//�ڽ� ���μ��� ����
		pid = fork();
		//���μ��� ���� ����
		if ((pid) < 0) {
			printf("forkfailed");
			exit(1);
		}
		//�ڽ� ���μ������� �����̷��� ����
		else if (pid == 0) {
			for (int j = 0; j < i; j++) {
				if (!strcmp(cmd[j], ">")) {
					fd = open(cmd[j + 1], O_RDWR | O_CREAT | O_TRUNC, 0644);
					dup2(fd, STDOUT_FILENO);
					close(fd);
					cmd[j] = NULL;
					break;
				}

				else if (!strcmp(cmd[j], ">>"))
				{
					fd = open(cmd[j + 1], O_RDWR | O_CREAT | O_APPEND, 0644);
					dup2(fd, STDOUT_FILENO);
					close(fd);
					cmd[j] = NULL;
					break;
				}
			}
			strcat(path, cmd[0]);
			execve(path, cmd, NULL);
			exit(1);
		}
		//�θ� ���μ������� �ڽ����μ����� ��ٸ���
		else
			waitpid(pid, NULL, 0);

	}
	return 0;
}