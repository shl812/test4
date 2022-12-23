/*
Homework 4 title: Make a shell in goorm IDE
ID: 32181141
NAME: KIM CHANG OH
Major: Software engineering
Class: System_Programming_3rd_class
Summit DATE: 2022.12.09*/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>


#define MAXARGC 10

void help(){//help명령어 처리내용
	printf("==============================================\n");
	printf("%-5s: USAGE: ls [option]\n", "ls");
	printf("%-5s: USAGE: cd <name of directory>\n ", "cd");
	printf("%-5s: USAGE: gcc [option] [name of c-file]\n", "gcc");
	printf("%-5s: USAGE: vi [name of file]\n", "vi");
	printf("%-5s: cat [name of file\n]", "cat");
	printf("===============================================\n");
}

void child_handler(int sig){//dead process 방지목적
	int status;
 	pid_t pid;

	if((pid=waitpid(-1, &status,WNOHANG))>0)
		fprintf(stderr, "kill ed process: %d\n", pid);
	
}

int Get_Token(char* command, char* token[]){//토큰생성
	char* p;
	int i=0;//
	int token_num=0;//토큰 저장할 인덱스

	while(command[i]){
		p=command+i;
		token[token_num]=p;
		while(command[i]!=' '&&command[i]&&command[i]!='\t'&&command[i]!='\n'){
			i++;
			p++;
		}//띄어쓰기, NULL을 만날때 까지 반복
		*p='\0';
		i++;
		token_num++;
	}
	token[token_num]=(char*)0;//execvp를 위한 구문
	return token_num;
}


int Run(char* line){

	int arg_count=1;
	pid_t child;//프로세스 id
	char *command[MAXARGC];//명령어 토큰
	char *command_redirection[MAXARGC];
	int redirection_chk=0;//'redirection, Not redirection
	int redirection_rvs_chk=0;
	int redirection_inx;
	int redirection_rvs_inx;
	int token;//토큰의 갯수
	int stat;
	int i,fd;//반복문과 입출력 재지정시 사용

	for(i=0;line[i];i++){
		if(line[i]==' ')
			arg_count++;
	}//명령어 개수 카운트

	if(arg_count>=MAXARGC){
		printf("Over Command!\n");
		return 0;
	}//명령어가 너무 많은 경우
	token=Get_Token(line, command);//명령어 토큰화

	if(!token) return 0;

	for(i=0;i<token;i++){
		if((!strcmp(command[i], ">"))||(!strcmp(command[i], ">>")))
		{
			redirection_inx=i;
			redirection_chk=1;
		}
		if(!strcmp(command[i], "<")){
			redirection_rvs_chk=1;
			redirection_rvs_inx=i;
		}
	}//토큰 인덱스저장(redirection)

	if(!(strcmp(command[0], "cd"))){//폴더변경
		if(arg_count==1){
			chdir(getenv("HOME"));
		}//cd만 입력시 홈으로 이동
		else if(arg_count==2){
			if(chdir(command[1])){//디렉토리 입력시 해당 위치로 이동
				printf("No Directory founed\n");//디렉토리가 존재하지 않을 시 출력
			}
		}
		return 0;
	}

	else if(!strcmp(command[0], "help")){//help명령어 수행
		help();
		return 0;
	}

	else if((!strcmp(command[0],"exit"))||(!strcmp(command[0],"quit"))){
		exit(1);
	}

	signal(SIGCHLD,(void *)child_handler);

	child=fork();//프로세스 포크
	 if(child==0){
		if(child<0){//fork실패시
			printf("error in fork\n");
			return 0;
		}

		else if(redirection_chk){//'>'수행
			for(i=0;i<redirection_inx;i++){
					command_redirection[i]=command[i];
			}
			fd=open(command[arg_count-1],O_RDWR|O_CREAT,0764);
			command_redirection[i]='\0';
			close(STDOUT_FILENO);
			dup2(fd,STDOUT_FILENO);//출력재지정
			execvp(command_redirection[0], command_redirection);
			printf("Can not Execute\n");
			exit(0);
		}

		else{
			execvp(command[0],command);//프로세스변환
			printf("%s: Can not Execute\n", command[0]);//변환 불가시
			exit(0);
		
		}
	}

	else if(strcmp(command[arg_count-1], "&")){
						   
			waitpid(child,&stat,0);
	}
	return 0;//run 종료
}

void show_cur_dir(){//현재 디렉토리 출력
	char dir_path[256];
	getcwd(dir_path,256);
	printf("%s$ ", dir_path);
}


int main(int argc, char* argv[])
{	
	char line[512]="\0";//입력문자열 초기화
	while(1){
		show_cur_dir();
		fgets(line, sizeof(line),stdin);
		fflush(stdin);
		line[strlen(line)-1]='\0';
		if(Run(line)){//exit나 quit인 경우 myShell 종료
			break;
		}
	}
	return 0;
}	
