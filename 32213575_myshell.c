/*HW4 Make a shell 
name : 32213575 이진
date : 2022-12-09-*/

#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/stat.h>

//구조체 
struct COMMAND {
   char* cmd;
   char* desc;
   bool(*func)(int argc,char* argv[]);
};

bool cmd_cd(int argc, char* argv[]);
bool cmd_exit(int argc, char* argv[]);
bool cmd_help(int argc, char* argv[]);

//cmd 목록
struct COMMAND bulletin_cmds[]={
   {"cd","change directory",cmd_cd},
   {"exit","exit the shell",cmd_exit},
   {"help","show help menu",cmd_help}
};

//cd구현
bool cmd_cd( int argc, char* argv[] ){ //cd : change directory
    if( argc == 1 )
        chdir( getenv( "HOME" ) );
    else if( argc == 2 ){
        if( chdir( argv[1] ) )
            printf( "No directory\n" );
    }
    else
        printf( "USAGE: cd [dir]\n" );
    
    return true;
}

bool cmd_exit( int argc, char* argv[] ){//shell 종료
        return false;
}

bool cmd_help( int argc, char* argv[] ){ // 명령어 출력
	printf("===========myshell 명령어 종류===========\n");
        int i;
        for( i = 0; i < sizeof( bulletin_cmds ) / sizeof( struct COMMAND ); i++ )
        {
                if( argc == 1 || strcmp( bulletin_cmds[i].cmd, argv[1] ) == 0 )
                        printf( "%-10s: %s\n", bulletin_cmds[i].cmd, bulletin_cmds[i].desc );
        }
	return true;
}

//입력받은 문자열 나누기
int tokenize( char* buf, char* delims, char* tokens[], int maxTokens ){
        int token_count = 0;
        char* token;
        token = strtok( buf, delims );
        while( token != NULL && token_count < maxTokens ){
                tokens[token_count] = token;
                token_count++;
                token = strtok( NULL, delims );
        }
        tokens[token_count] = NULL;
        return token_count;
}

//redirection 실행-output만 
void Redirection(int argc,char *argv[]){
    int fd;
	if(argc!=4){		//인자수 에러처리
		printf("잘못된 입력입니다");
	}
    if(argc == 4) {      // redirection을 적용할 파일=argv[3]
      fd = open(argv[3], O_WRONLY | O_CREAT|O_TRUNC, 0664); //fd로 가리킴
      if(fd < 0) { // 에러 처리
		  fatal("file open error");
         	exit(-1);
      }
      dup2(fd, STDOUT_FILENO); // standard ouput을 위에서 open한 파일로 설정(fd가 가리킴)
      argv[2] = NULL; // 입력된 > 없애기
    }
    close(fd);
}
//input, ouput redirection 구현하다가 실패
// void Redirection( int argc, char *argv[]){
//     int input_fd, output_fd;
//     int status, pid;
    
//     if ((pid=fork()) == -1)
//         perror("fork failed");
//     else if (pid != 0) {
//         if(is_bg==0)
//             pid = wait(&status);
//         else {
//             printf("[1] %d\n", getpid());
//             waitpid(pid, &status, WNOHANG);
//         }
//     } else {
//         if (flag == 2) {
//             if((input_fd = open(input, O_RDONLY))==-1){
//                 perror(argv[0]);
//                 exit(2);
//             }
//             dup2(input_fd, 0);
//             close(input_fd);
//             execvp(argv[0], argv);
//         } else if (flag == 3) {
//             output_fd = open(output, O_CREAT|O_TRUNC|O_WRONLY, 0600);
//             dup2(output_fd, 1);
//             close(output_fd);
//             execvp(argv[0], argv);
//         } else {
//             if (input != NULL && output != NULL) {
//                 input_fd = open(input, O_RDONLY);
//                 dup2(input_fd, 0);
//                 close(input_fd);
                
//                 output_fd = open(output, O_CREAT|O_TRUNC|O_WRONLY, 0600);
//                 dup2(output_fd, 1);
//                 close(output_fd);
//                 execvp(argv[0], argv);
//             }
//         }
//     }
// }

//pipe 명령어 구현시도
// bool pipe(char* cmd){
//    int fd[2];
//    char bufc[16], bufp[16];
//    int read_size=0;
   
//    if(pipe(fd)==-1){
//       perror("Pipe error");
//       exit(1);
//    }
//    if(fork()==0){
//       close(STDOUT_FILENO);
//       dup2(fd[1],1);
//       close(fd[0]); close(fd[1]);
      
//       execvp(str[0],str);
//       perror("exevp failed");
//       exit(1);
//    }
//    if(fork()==0){
//       close(STDOUT_FILENO);
//       dup2(fd[0],0);
//       close(fd[1]);
         // close(fd[0]);
      
//       execvp(str[0],str);
//       perror("exevp failed");
//       exit(1);
//    }
   
//    close(fd[0]);
//    close(fd[1]);
//    wait(0);
   
// }

//실행
bool run( char* line ){
    char delims[] = " \n\t";  //token을 나누는 기준
    char* tokens[128]; //token받기
    int token_count;
    int i;
	pid_t pid; //pid
    int status; //process의 status
    bool y_redirection;  //redireciotn여부 
    bool y_background;   //backgorund processing 여부 
    
   
    for(i=0;i<strlen(line);i++){ //background 실행은 wait하지 않는다.
                if(line[i] == '&'){
                        y_background=true;
                        line[i]='\0'; 
                        break;
                }
    }
   
    //입력명령어 분석
    token_count = tokenize( line, delims, tokens, sizeof( tokens ) / sizeof( char* ) );
    if( token_count == 0 )  //없다면 빈줄
                return true;
   
    //입력된게 어떤 cmd인지 확인
    for( i = 0; i < sizeof( bulletin_cmds ) / sizeof( struct COMMAND ); i++ ){
                if( strcmp( bulletin_cmds[i].cmd, tokens[0] ) == 0 )
                        return bulletin_cmds[i].func( token_count, tokens );
    }
	 //redirection 여부와 backgorund processing 여부 확인하고 설정하기
	if((strchr(line,'>'))!=NULL) y_redirection=true;
	if((strchr(line,'&'))!=NULL) y_background=true;


        
      //그 외 명령어 실행-fork()
        if( (pid = fork()) <0 ){  //fork 에러처리
               printf( "Failed to fork" );
               exit( -1 );
        }
        else if( pid == 0 ){ //자식 프로세스-backgoround processing여부에따라 나누기
        	 if(y_redirection==true){  //>가 있으면 redirction실행
           	 	Redirection(token_count,tokens);
        	 }
            execvp( tokens[0], tokens );  //그후에 실행
        }
      if(y_background==true){   		  //background processing 실행
            waitpid(pid,&status,WNOHANG); //WNOHANG 자식이 종료되지 않은 상태여도 함수 바로 리턴
      }
      else{
         waitpid(pid,&status,0);
      }
        
        return true;
}

int main(){
   char line[1024];
   
   while(1){    //계속 실행
      printf("%s $",get_current_dir_name());
      fgets(line,sizeof(line)-1,stdin); //키보드입력 가져옴,마지막은 엔터이니까 -1
      if(run(line)==false) break; //입력이 없으면 break
   }
   return 0;
}