/**
/myshell.c : implement my shell
/@author  : Kim Boseung
/@email   : kbskbs1102@gmail.com
/@version : 8.0.1453
/@date    : 2022.12.03
**/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <stdlib.h>
#define MAX 255

//��¥�� ���� ����, �۾���ġ�� ����ϰ� ��ɾ�� ���ڸ� �Է¹޴� ������Ʈ
void *prompt(char cBuf[]){ 
	time_t rawtime;
	struct tm *timeinfo;
	char hBuf[MAX], uBuf[MAX], dBuf[MAX];
	char *now;
	void *ret;           //���� ������ ����

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	now = asctime(timeinfo);  //����ð��� ���ڿ� �������� ��ȯ

	now[strlen(now)-1] = 0; //������ \n�� �� ���ڷ� �ٲ�

	gethostname(hBuf,MAX); //ȣ��Ʈ�̸��� �����ͼ� ����
	getlogin_r(uBuf, MAX); //���� ������ �����ͼ� ����
	getcwd(dBuf, MAX);     //���� �۾� ��ġ�� �����ͼ� ����

	printf("[%s]%s@%s(%s)$ ", now, hBuf, uBuf, dBuf); //���� ������ ���

	ret = fgets(cBuf, MAX, stdin);  //��ɾ �Է¹޾Ƽ� ���� ������ ����

	if(cBuf[strlen(cBuf)-1]=='\n')  //���ڿ��� ������ \n�� �ι��ڷ� ��ȯ
		cBuf[strlen(cBuf)-1]=0;

	if (strcmp(cBuf, "exit")==0){   //exit��ɾ �Է¹����� ���α׷� ����
		return 0;
	}

	return ret;
}

int main(){
	char cBuf[MAX];
	char *arg;
	pid_t pid;
	int status;        //���� ������ ����

	while(prompt(cBuf)){  //��ɾ ���������� �Է¹޴� ���� �ݺ�
		if((pid = fork())<0){    //pid�� 0�̻��̾���Ѵ�
			perror("fork error");
		}
		else if(pid==0){        //�ڽ� ���μ����� ���
			strtok(cBuf, " ");       //" "�� �������� cBuf�� ������ ��ūȭ�Ѵ�
			arg = strtok(NULL," ");  //strtok �Լ��� �̿��� ���� ���� ��ūȭ
		
			if(arg ==NULL){ //���ڰ� ���ٸ� 
				execlp(cBuf, cBuf, (char*)0); 
			}
			else{ //���ڰ� �ִٸ� 
				if(strcmp(cBuf, "cd") == 0){ //��ɾ cd��� ���丮 �̸��� arg�� ������ �̵�
					chdir(arg);
					_exit(0);
				}
				else{   //�� �ǿ��� ����� 
					execlp(cBuf, cBuf, arg, (char*) 0);
				}
			}
			perror("couldn't execute"); 
		}
		waitpid(pid, &status, 0);//���μ����� ���������� ��ٸ���
	}

	exit(0);
}
