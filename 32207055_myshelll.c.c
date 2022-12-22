/**
 * oniShell.c  : make shell
 * @Author : Yewon Gu
 * @Email : jastu111@naver.com
 * @Version : 1.0
 * @Date : 2022.12.02-
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>


#define TRUE	1
#define FALSE	0

#define MAX_COUNT 10

const char *prompt = "oniShell ";

char* cmdgrp[MAX_COUNT];
char* cmdlist[MAX_COUNT];
char* cmdargs[MAX_COUNT];
char cmdline[BUFSIZ];

int tokenize(char *s, const char *delimiters, char** argvp, int max_l);
void parse_redirect(char* cmd);
void ex_cmd(char *cmdlist);
void ex_cmd_line(char* cmdline);
void ex_cmd_grp(char* cmdgrp);

struct sigaction act;
static int IsBack=0; //background인지

// COMMAND 구조체 
typedef struct { 
    char* name;
    char* desc;
    int ( *func )( int argc, char* argv[] ); // 함수포인터
} COMMAND;


//cd : Change directory
int cmd_cd( int argc, char* argv[] ){ 
    if( argc == 1 )	
        chdir( getenv( "HOME" ) );
    else if( argc == 2 ){	// 두번째 인자가 디렉토리 이름으로 존재하지 x
        if( chdir( argv[1] ) )
            printf( "No directory\n" );
    }
    else
        printf( "USAGE: cd directory_name \n" );  
	
    return TRUE;
}

// exit : oni shell 종료
int cmd_exit( int argc, char* argv[] ){
    printf("---Exit---\n");
    exit(0);
    
    return TRUE;
}


static COMMAND builtin_cmds[] =
{
    { "cd", "change directory", cmd_cd },
    { "exit", "exit onishell", cmd_exit }, 
};


// main 함수
int main(int argc, char**argv)
{
	int i;
	sigset_t set;
	
	sigfillset(&set);
	sigdelset(&set,SIGCHLD);
	sigprocmask(SIG_SETMASK,&set,NULL);
    
	act.sa_flags = SA_RESTART;
	sigemptyset(&act.sa_mask);
	sigaction(SIGCHLD, &act, 0);
	
	printf("--------------------------------\n");
	printf("-----------ONI  SHELL-----------\n");
	printf("----------'종료 : exit'----------\n");
	printf("--------------------------------\n");

    while (1) {
        fputs(prompt, stdout);
        printf( "[%s] $ ", get_current_dir_name() );
        fgets(cmdline, BUFSIZ, stdin);
        cmdline[ strlen(cmdline) -1] ='\0';
		ex_cmd_line(cmdline);
	}
	return 0;
}



// tokenizer
int tokenize(char *s, const char *delimiters, char** argvp, int max_l )
{
	int i = 0;
	int numtokens = 0;
	char *snew = NULL;
    
	if( (s==NULL) || (delimiters==NULL) )
	{
		return -1;
	}
    
	snew = s+strspn(s, delimiters);
	
	argvp[numtokens]=strtok(snew, delimiters);
	
	if( argvp[numtokens] !=NULL)
		for(numtokens=1; (argvp[numtokens]=strtok(NULL, delimiters)) != NULL; numtokens++)
		{
			if(numtokens == (max_l-1)) return -1;
		}
    
	if( numtokens > max_l) return -1;
    
	// 토큰 개수 반환
	return numtokens;    
}


// redirection 구현
void parse_redirect(char* cmd)
{
	char *arg;
	int cmdlen = strlen(cmd);
	int fd, i;
	
	for(i = cmdlen-1;i >= 0;i--)
	{
		switch(cmd[i])
		{
			case '<':
				arg = strtok(&cmd[i+1], " \t");
				fd = open(arg, O_RDONLY | O_CREAT, 0644);
				if(fd < 0){
					perror("file open error");
					exit(1);
				}
				dup2(fd, STDIN_FILENO);
				close(fd);
				cmd[i] = '\0';
				break;
			case '>':
				arg = strtok(&cmd[i+1], " \t");
				fd = open(arg, O_RDWR | O_CREAT | O_TRUNC, 0644);
                if(fd < 0){
					perror("file open error");
					exit(1);
				}
                dup2(fd, STDOUT_FILENO);
                close(fd);
                cmd[i] = '\0';
				break;
				
			default:break;
		}
	}
    
}


// 백그라운드 실행 
int background(char *cmd)
{
	int len = strlen(cmd);
	int i;
    for(i=0; i < len; i++)
        if(cmd[i] == '&')
        {
            cmd[i] = ' ';
            return 1;
        }
	return 0;
}


// 커맨드 실행 
void ex_cmd(char *cmdlist)
{
    parse_redirect(cmdlist);
    
    if(tokenize(cmdlist, " \t", cmdargs, MAX_COUNT) <= 0){
		perror("tokenize error");
		exit(1);
	}
		
    execvp(cmdargs[0], cmdargs);
    perror("exec error");
	exit(1);
}



void ex_cmd_grp(char *cmdgrp)
{
	int i=0;
	int pid;
	int count = 0;
	int pfd[2];
    sigset_t set;
    
	setpgid(0,0);
 	if(!IsBack){
        tcsetpgrp(STDIN_FILENO, getpid());
	}
    
    sigfillset(&set);
    sigprocmask(SIG_UNBLOCK,&set,NULL);
    
    if((count = tokenize(cmdgrp, "|", cmdlist, MAX_COUNT)) <= 0){
		perror("tokenize error");
		exit(1);
	}
		
    
	for(i=0; i<count-1; i++)
    {
		pipe(pfd);  //pipe
		
		pid = fork();
		if(pid==-1){                        //fork error
			perror("fork error");
			exit(1);
		}else if(pid==0){                   //child
			close(pfd[0]); 
            dup2(pfd[1], STDOUT_FILENO);
            ex_cmd(cmdlist[i]);
		}else{                              //parent
			close(pfd[1]);
            dup2(pfd[0], STDIN_FILENO);
		}
	}

	ex_cmd(cmdlist[i]);
    
}



void ex_cmd_line(char* cmdline)
{
    int count = 0;
    int i=0, j=0, pid;
    char* cmdvector[MAX_COUNT];
    char cmdgrptemp[BUFSIZ];
    int numtokens = 0;
    
    count = tokenize(cmdline, ";", cmdgrp, MAX_COUNT);
    
    for(i=0; i<count; ++i)
    {
        memcpy(cmdgrptemp, cmdgrp[i], strlen(cmdgrp[i]) + 1);
        numtokens = tokenize(cmdgrp[i], " \t", cmdvector, MAX_COUNT);
        
		//cd, exit 실행
        for( j = 0; j < sizeof( builtin_cmds ) / sizeof( COMMAND ); j++ ){
            if( strcmp( builtin_cmds[j].name, cmdvector[0] ) == 0 ){
                builtin_cmds[j].func( numtokens , cmdvector );
                return;
            }
        }
        
		//그밖의 명령어 실행
		IsBack = background(cmdgrptemp); //백그라운드 검사
        
		if((pid=fork())==-1){
			perror("fork error");
			exit(1);
		}else if(pid==0){
			ex_cmd_grp(cmdgrptemp);
		}else{
			if(IsBack) break;
            waitpid(pid, NULL, 0);
            tcsetpgrp(STDIN_FILENO, getpgid(0));
            fflush(stdout);
		}
		
		
    }
    
}


