/*
 * dwsh.c : 직접 만든 shell 코드
 * author : 소프트웨어학과 32193049 이동원
 * email : ldw6589@gmail.com
 * @version : 1.0.0
 * @date : 2022-12-09 17:02:30
 */


#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<wait.h>
#include<stdlib.h>
#include<stdbool.h>
#include<fcntl.h>

bool run(char*);
bool isBack(char**, int);
bool isReDir(char**, int);
void dw_ReDir(char**, int);

char* command_wordV[10];
char command[100];
char pathbuf[64];

//무한반복문을 통해 쉘의 초기 화면 나타내기 
int main() {

	while (1) {
		getcwd(pathbuf, 64);
		printf("%s$ ", pathbuf); //현재 경로와 '$' 나타내기
		fgets(command, 100, stdin); //입력받은 값을 command에 저장
		command[strlen(command) - 1] = 0x00; //엔터키 빼주기 
		if (run(command) == false) break;

	}
	return 0;
}


bool run(char* command)
{
	pid_t fork_rt, d_pid;
	int exit_status;
	int cnt = 0; int state;

	char* command_word = strtok(command, " "); // 명령어에서 분리 
	while (command_word != NULL)
	{
		command_wordV[cnt] = command_word;
		command_word = strtok(NULL, " "); //strtok를 통해 공백 기준으로 명령어 분해하기
		cnt++;
	}
	//입력한 명령어 보여주기
	printf("명령어: ");
	for (int k = 0; k < cnt; k++)
	{
		printf("%s ", command_wordV[k]);
	}
	printf("\n");
	if (cnt == 0) return false;
	if (strncmp(command_wordV[0], "exit", 4) == 0) return false; //exit명령어 구현 

	if ((fork_rt = fork()) < 0) // fork 및 exec
	{
		printf("fork error");
		exit(1);
	}
	else if (fork_rt == 0) // 자식프로세스
	{
		if (cnt == 4) //명령어 인자가 4개일 때 isReDir을 통해 확인
		{
			if (isReDir(command_wordV, cnt))
			{
				dw_ReDir(command_wordV, cnt); //리다이렉션 실행
			}
		}
		execvp(command_wordV[0], command_wordV); //리스트보단 벡터 형태가 편할 것 같아 execvp 사용 
	}

	else //부모프로세스
	{
		if (!isBack(command_wordV, cnt)) wait(&state); //백그라운드 실행 아닐 시
		else //백그라운드 실행 시
		{
			printf("[1] %d\n", getpid());
			waitpid(d_pid, &state, WNOHANG);
		}
	}

	return true;
}
//백그라운드 실행 명령어인지 분석
bool isBack(char** command_wordV, int cnt)
{
	bool back = false;
	if (strcmp(command_wordV[cnt - 1], "&") == 0)
	{
		printf("백그라운드 실행입니다. \n");
		back = true;
	}
	return back;
}
//리다이렉션 실행 명령어인지 분석
bool isReDir(char** command_wordV, int cnt)
{
	bool reDir = false;
	if (strcmp(command_wordV[cnt - 2], ">") == 0)
	{
		printf("리다이렉션 실행입니다. \n");
		reDir = true;
	}
	return reDir;
}
//리다이렉션 실행 함수
void dw_ReDir(char** command_wordV, int cnt)
{
	int fd, fd1, r_s, w_s;
	char buf[64];

	fd = open(command_wordV[1], O_RDONLY);
	fd1 = open(command_wordV[3], O_RDWR | O_CREAT, 0641);
	dup2(fd1, STDOUT_FILENO);

	while (1) {
		r_s = read(fd, buf, 64);
		if (r_s == 0) break;
		w_s = write(STDOUT_FILENO, buf, r_s);
	}

	close(fd);
}

