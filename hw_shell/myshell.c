/*
* This is homework 'my shell' : shell program
* @author : Donghwi Kim
* @student's ID : 32190483
* @date : 2022.12.08
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h> 
#include <sys/wait.h>
#include <pthread.h>
#include <errno.h>
#include "myshell.h"

void cmd_help() //도움말
{
    printf("--------------- MY Shell Program --------------\n");
	printf("--------------------- HELP! -------------------\n");
    printf("\t간단한 shell 프로그램입니다.\n");
    printf("\t아래는 명령어 사용법입니다.\n");
    printf("\texit\t: shell을 나갑니다\n");
    printf("\thelp\t: 도움말을 켜줍니다.\n");
	printf("\tcd\t: 디렉토리의 위치를 변경합니다.\n");
	printf("\tls\t: 파일이나 디렉토리의 정보를 출력합니다.\n");
	printf("\tps\t: 프로세스상태를 출력합니다.\n");
	printf("\tpwd\t: 현재 디렉토리 위치를 알려줍니다.\n");
    printf("\t>\t: 표준 출력 재지정입니다.\n");
	printf("\t>>\t: append추가한 출력 재지정입니다.\n");
    printf("\t<\t: 표준 입력 재지정입니다.\n");
    printf("\t&\t: 백그라운드 프로세싱입니다.\n");
    printf("-----------------------------------------------\n");
}

bool cmd_cd(int token_count, char* tokens[])
{
	if(token_count==1) //token의 개수가 1이면 HOME임
		chdir(getenv("HOME"));
    else if(token_count==2) //token의 개수가 2이면 cd명령어 다음인 디렉토리경로로 chdir해줌
		chdir(tokens[1]);
    else
		printf("에러가 발생했습니다!\n"); //그 외면 에러 발생메시지 출력.
    return true;
} 


int tokenize(char* buf, char* delims, char* tokens[], int maxTokens) // parsing의 개념을 구현
{
    char* token;
    int token_count = 0;

    token = strtok(buf, delims); // 입력한 문자열을 delims을 만나면 만나기까지의 상태를 저장한다.
    //tokens배열에 delims단위'\n'로 끊은 token을 계속 저장해준다.
	while( token != NULL && token_count < maxTokens) //문자열이 끝나거나 입력한 문자열의 사이즈를 넘지않을 때 동안 반복해준다.
    {
		tokens[token_count++] = token;  
		token = strtok(NULL, delims); 
    }
	
    tokens[token_count] = NULL; //마지막 tokens에는 NULL을 추가해준다.

    return token_count; //token의 갯수를 반환
	
}

bool run(char* line)
{
    char* tokens[1024]; // 입력받은 문자열을 저장할 배열
    char* temp[1024]; //tokens를 복사할 배열
    char* delims = " \n"; //입력받은 문자열을 \n 단위로 끊어주기 위한 변수
    bool back = false; //백그라운드에서 프로세스를 실행할지 결정해주는 변수
    pid_t child; //자식 프로세스 번호를 저장할 변수(int형과 같음)
    int status; //상태 저장 변수
    int token_count = tokenize(line, delims, tokens, sizeof(tokens) / sizeof(char*)); // token의 갯수를 반환하면서 tokens 배열을 만들어준다.
    int fd; //파일저장 변수
    int flag = 0; //redirection 구분 변수
    int i,k; 
	
	//입력한 token을 명령어와 비교하면서 명령어가 있으면 flag를 설정하고 loop문 빠져나가게 설정
    for(i=0;i<token_count;i++) 
    { 
		//사용자가 입력한 redirection이 무엇인지 파악해주기 없으면 flag = 0이다.
		if(!strcmp(tokens[i],OUT_REDIR))  // token에 > 이 포함되면
		{
			flag = 1;
			break;
		}
		else if(!strcmp(tokens[i],APPEND)) // token에 >> 이 포함되면
		{
  			flag = 2;
			break;
		}
		else if(!strcmp(tokens[i],IN_REDIR)) // token에 < 이 포함되면
		{
			flag = 3;
			break;
		}
		else if(!strcmp(tokens[i],BACK_PRO)) // token에 & 이 포함되면
		{
			back = true;
			break;
		}
		else if(!strcmp(tokens[i],"cd"))// token에 cd가 있으면, cmd_cd함수 호출
		{
			return cmd_cd(token_count,tokens);
		}
    }

	child = fork();	//fork()로 프로세스를 복사해주고 자식 프로세스 번호를 저장한다.
    if(child  < 0)  //fork 예외처리
    {
		perror("fork error");
		return false;
    }        
    else if(child == 0) //자식프로세스가 맞으면 다른 프로그램을 실행해 준다.
    {
		for(k=0; k<i; k++) //tokens을 temp에 복사해준다.
		{
			temp[k]=tokens[k];
		}
		if(flag==1) // 출력 재지정일때 : >
		{
			//tokens의 i+1은 명령어 다음의 파일이름을 뜻함. 
			fd = open(tokens[i+1], O_WRONLY | O_TRUNC | O_CREAT,0664); //  쓰기전용으로 만약 기존 파일이 있으면, 파일 크기를 0으로 만듦
			if(fd < 0) // open에러 시
				error("Can't open %s file with error num %d\n.", tokens[i+1], errno);
			close(STDOUT_FILENO);
			dup2(fd, STDOUT_FILENO); //fd를 dup2함수로 표준 출력으로 바꿔준다.
			close(fd);
		}
		else if(flag==2) //append한 출력 재지정일때 : >>
		{
			fd = open(tokens[i+1], O_WRONLY | O_APPEND | O_CREAT,0664); //append를 해주고, 쓰기전용으로 생성하면서 열어준다.
			if(fd < 0) // open 에러 시
				error("Can't open %s file with error num %d\n.", tokens[i+1], errno);
			close(STDOUT_FILENO);
			dup2(fd, STDOUT_FILENO);
			close(fd);
		}
		else if(flag==3) // 입력 재지정일때 : <
		{
			fd = open(tokens[i+1], O_RDONLY); //읽기 전용으로 파일을 열어준다.
			if(fd < 0) // open 에러 시
				error("Can't open %s file with error num %d\n.", tokens[i+1], errno);
			close(STDIN_FILENO);
			dup2(fd,STDIN_FILENO); //fd를 dup2함수로 표준 입력으로 바꿔준다.
			close(fd);
		}
		execvp(tokens[0], temp); // tokens[0]로 프로그램 실행 이름을, temp로 입력한 문자열(명령)을 execvp함수의 인자로 넣어서 실행해준다.
		exit(0);
    }
    else //부모프로세스는 waitpid를 사용해서 자식프로세스를 기다려줌
    {
		if(back==false)
			waitpid(child, &status, 0);
		// & 를 입력받아 백그라운드에서 프로세스를 실행할 경우 
		else 
			waitpid(child, &status, WNOHANG);  //인자로 WNOHANG를 사용해서 기다리지 않고 바로 명령을 실행할 수 있게 해줌.
	}
    return true;
}
