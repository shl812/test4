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
		//입력받을 값에 '\0'로 채운다
		for (i = 0; i < 256; i++) {
			buf[i] = '\0';
		}
		//명령어를 입력받고 끝을 '\0'으로 바꾼다
		fgets(buf, sizeof(buf), stdin);
		buf[255] = '\0';
		i = 0;
		//입력받은 값을 \t\r\n으로 분리시켜 cmd에 입력한다
		str = strtok_r(buf, "\t\r\n", &status);
		while (str != NULL && i < 256) {
			cmd[i++] = str;
			str = strtok_r(NULL, "\t\r\n", &status);
		}
		cmd[i] = NULL;
		//cmd[0]이 q이면 종료한다
		if (strcmp(cmd[0], "q") == 0)
			exit(1);
		//자식 프로세스 생성
		pid = fork();
		//프로세스 생성 실패
		if ((pid) < 0) {
			printf("forkfailed");
			exit(1);
		}
		//자식 프로세스에서 리다이렉션 구현
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
		//부모 프로세스에서 자식프로세스를 기다린다
		else
			waitpid(pid, NULL, 0);

	}
	return 0;
}