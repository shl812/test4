/*
 * mysh.c : making shell myself
 * author : Kyuwon Jeong
 * student id : 32204027
 * email : 32204027@dankook.ac.kr
 * version : 1.0
 * date : 2022. 12. 07
 */

//cd, exit, quit, redirection, pipe, bg process는 execvp로 실행이 되지 않는다


#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>

typedef enum { true = 1, false = 0 } bool;


int tokenize(char* buf, char* delims, char* tokens[], int maxTokens){	//입력받은 명령어를 토큰화하여 배열 형태로 저장 후 토큰의 개수를 반환하는 함수
	
	char** ptr_tokens=tokens;	//tokens 배열을 포인터로 call by reference
	char* token = strtok(buf, delims);	//delims를 기준으로 buf에 들어있는 정보를 token화하여 token에 저장
	int token_count=0;	//token의 개수를 세기 위한 변수, 1로 초기화, token_count-1을 하여 배열의 index를 나타내기도 한다
	
	while(token != NULL && token_count < maxTokens){
		ptr_tokens[token_count]=token;	//tokens 배열 token_count-1 번째 index에 token을 저장
		token=strtok(NULL, delims);	//다음 토큰을 token에 저장
		token_count++;	//token_count를 +1 증가
	}//endwhile

	return token_count;	//tokens 배열에 들어있는 원소의 개수가 저장되어 있는 token_count 변수를 반환
}//endfunc (tokenize)


bool run(char* line){	//입력받은 명령어를 실행시키는 함수

	char* tokens[50];	//line을 token으로 나누어 저장하기 위한 tokens
	char* delims=" \t";	//구분자는 공백과 탭키('\t')를 사용한다
	int token_count;	//token의 개수를 담기 위한 변수 token_count
	
	int fd;	//for redirection
	
	int fd_pipe[2][2];	//for pipe
	int pipe_count = 0;	//for pipe
  	int start_point[3];	//for pipe
	char* pipe_flag=strchr(line, '|');	//for pipe
	
	int child;	//for fork
	int i=0;	//for loop
	
	int bg_flag=false;	//for background process
	
	char* redirection_flag_0 = strchr(line, '>');	//for redirection
	char* redirection_flag_1 = strstr(line, ">>");	//for redirection
	char* redirection_flag_2 = strchr(line, '<');	//for redirection
	
	token_count = tokenize(line, delims, tokens, sizeof(tokens)/sizeof(char*));
	tokens[token_count]=NULL;	//execvp 함수 실행을 위해 NULL 값 저장
	
	if(strcmp(tokens[0],"\n")==0){	//입력이 개행문자 하나라면
		return true;	 //true를 반환하면서 run 함수 종료, main 함수로 돌아간다
	}
	
	
	
	/* start handling internal command */
	
	//start (exit, quit, q) command
	
	if (strcmp(tokens[0], "exit" ) == 0 || strcmp(tokens[0], "quit" ) == 0 || strcmp(tokens[0], "q" ) == 0){
		//exit or quit or q가 입력되면
		//run 함수에서 false를 리턴하면서 main 함수 종료
			return false;
		}
	
	//end (exit, quit, q) command
	
	//start cd command
	
	if(strcmp(tokens[0], "cd") == 0){	//tokens[0]이 cd 일 때		
		if( tokens[1] == NULL ){	//tokens[1]이 NULL일 때 (tokens[0]밖에 없을 때)			
			//change directory to home directory
			if(chdir("/")!=0){	//root(/) 경로로 change directory 해준다
				printf("chdir error with code : %d\n", errno);	//error handling
				return false;	//비정상 종료
			}//endif
		}//endif
		
        else if(tokens[1] != NULL){	//tokens[1]이 NULL이 아닐 때 (tokens[0]뿐만 아니라 tokens[1]도 있을 때)			
			if( chdir( tokens[1] ) != 0){	//change directory to target directory
				printf("%s 라는 파일이나 디렉토리가 없습니다\n", tokens[1]); //error handling
			}//endif
		}//endelseif
		
		else{	//print usage
				printf( "USAGE: cd target_directory\n" );
			}//endelse		
	}//endif
	
	//end cd command

	/* end handling internal command */
	
	
	
	
	/* start handling redirection, pipe, bg processing */
	
	if(strcmp(tokens[token_count-1], "&")==0){	//명령어 마지막에 &가 있으면 bg_flag를 true로 변경
		bg_flag=true;
		tokens[token_count-1]=NULL;	//& 기호 지우기
	}
	//bg_flag 설정 완료
	
	//명령어 처리 시작
	if(pipe_flag!=NULL){	//pipe인 경우
    	
		for (i = 0; tokens[i] != NULL; i++) {  //tokens 배열 원소 개수만큼 반복

			if (strcmp(tokens[i], "|")==0) {	//tokens 배열의 i번째 인덱스가 | 문자라면 NULL로 변경
				
				pipe_count += 1;	//pipe의 개수를 +1 증가시킨다
				tokens[i] = NULL;	//| 문자를 NULL로 변경

				start_point[pipe_count] = i+1;	//| 문자 다음을 명령어의 시작점으로 저장
			}

		}

		pipe(fd_pipe[0]);	//첫번째 파이프 생성

		//첫 명령어
		if ((child = fork()) < 0) {
			printf("fork error with code : %d\n", errno);
			return false;
		}

		else if (child == 0) {	//자식 프로세스일 때
			dup2(fd_pipe[0][1], STDOUT_FILENO);	//첫번째 파이프 입구에 표준 출력 fd를 복제
			execvp(tokens[0], tokens);	//첫번째 명령어 실행
		}

		else{	//부모 프로세스일 때
			close(fd_pipe[0][1]);	//첫번째 파이프 입구 닫기
			wait(NULL);	//자식 프로세스 기다리기
		}
		
		//두번째 명령어
		if(pipe_count==2){	//파이프 개수가 2개일 때에만 실행

			pipe(fd_pipe[1]);	//두번째 파이프 생성

			if ((child = fork()) < 0) {
			  printf("fork error with code : %d\n", errno);
			  return false;
			}

			else if (child == 0) {	//자식 프로세스일 때
			  dup2(fd_pipe[1][1], STDOUT_FILENO);	//두번째 파이프 입구에 표준 출력 fd를 복제
			  dup2(fd_pipe[0][0], STDIN_FILENO);	//첫번째 파이프 출구에 표준 입력 fd를 복제
			  execvp(tokens[start_point[1]], &tokens[start_point[1]]);	//두번째 명령어 실행
			}
			
			else{	//부모 프로세스일 때
				close(fd_pipe[1][1]);	//두번째 파이프 입구 닫기
				close(fd_pipe[0][0]);	//첫번째 파이프 출구 닫기
				wait(NULL);	//자식 프로세스 기다리기
			}

		}//endif(pipe_count==2)

		//세번째 명령어
		if ((child = fork()) < 0) {
			printf("fork error with code : %d\n", errno);
			return false;
		}

		else if (child == 0) {	//자식 프로세스일 때
			dup2(fd_pipe[pipe_count-1][0], STDIN_FILENO);	//첫번째 파이프 출구에 표준 입력 fd를 복제
			execvp(tokens[start_point[pipe_count]], &tokens[start_point[pipe_count]]);	//두번째 or 세번째 명령어 실행
			//파이프가 하나 있을 때에는 두번째 명령어
			//파이프가 두개 있을 때에는 세번째 명령어
		}
		
		else{	//부모 프로세스일 때
			close(fd_pipe[pipe_count-1][1]);	//첫번째 or 두번째 파이프 입구 닫기
			wait(NULL);	//자식 프로세스 기다리기
		}
		
		return true;	//run 함수에서 true를 반환함으로 main 함수에서 무한반복문이 종료되지 않도록 한다
		
	}//endif == pipe
	
	else{	//pipe가 아닌 경우
		
		//fork start
		if((child=fork())==-1){	//fork error 
			printf("fork error with code : %d\n", errno);
			return false;
		}
	
	//parent, child control
	
		if(child==0){	//start child process (자식 프로세스 일 때)
		
			if(bg_flag==false){	//foreground process's child (백그라운드로 실행하지 않을 때)
			
				//start redirection command
				if(redirection_flag_0 != NULL && redirection_flag_1 == NULL){	//redirection ('>')처리
					for(i=0;strcmp(tokens[i],">")!=0;i++);	//> 문자 인덱스를 찾기 위한 for문	
					if ((fd = open(tokens[token_count - 1], O_CREAT | O_RDWR | O_TRUNC, 0644)) < 0){
						return false;
					}	
				dup2(fd, STDOUT_FILENO);
				tokens[i] = NULL;
				}
			
				else if(redirection_flag_1 != NULL){	//redirection ('>>')처리
					for(i=0;strcmp(tokens[i],">>")!=0;i++);	//>> 문자 인덱스를 찾기 위한 for문	
					if ((fd = open(tokens[token_count - 1], O_RDWR | O_APPEND, 0644)) < 0){
						return false;
					}
					dup2(fd, STDOUT_FILENO);
					tokens[i] = NULL;
				}
			
				else if(redirection_flag_2 != NULL){	//redirection ('<')처리
					for(i=0;strcmp(tokens[i],"<")!=0;i++);	//< 문자 인덱스를 찾기 위한 for문	
					if ((fd = open(tokens[token_count - 1], O_RDWR, 0644)) < 0){
						return false;
					}
					dup2(fd, STDIN_FILENO);
					tokens[i] = NULL;
				}
			//end redirection command
			
				if(execvp(tokens[0], tokens)==-1){	//명령문 실행, 오류라면 에러메시지 출력
					if(!(strcmp(tokens[0],"cd")==0||strcmp(tokens[0],"quit")==0||strcmp(tokens[0],"exit")==0||strcmp(tokens[0],"q")==0)){
						//명령어가 cd, quit, exit, q라서 오류라면 에러메시지 출력하지 않음
						//그 외 명령어인 경우에만 에러메시지 출력
						printf("%s 라는 명령어가 없습니다\n", tokens[0]);
					}
				}
			return false;
		}
		
			else if(bg_flag==true){	//background process's child (백그라운드로 실행 할 때)
			
			
				//start redirection command
				if(redirection_flag_0 != NULL && redirection_flag_1 == NULL){	//redirection ('>')처리
					for(i=0;strcmp(tokens[i],">")!=0;i++);	//> 문자 인덱스를 찾기 위한 for문	
					if ((fd = open(tokens[token_count - 1], O_CREAT | O_RDWR | O_TRUNC, 0644)) < 0){
						return false;
					}	
				dup2(fd, STDOUT_FILENO);
				tokens[i] = NULL;
				}
			
				else if(redirection_flag_1 != NULL){	//redirection ('>>')처리
					for(i=0;strcmp(tokens[i],">>")!=0;i++);	//>> 문자 인덱스를 찾기 위한 for문	
					if ((fd = open(tokens[token_count - 1], O_RDWR | O_APPEND, 0644)) < 0){
						return false;
					}
					dup2(fd, STDOUT_FILENO);
					tokens[i] = NULL;
				}
			
				else if(redirection_flag_2 != NULL){	//redirection ('<')처리
					for(i=0;strcmp(tokens[i],"<")!=0;i++);	//< 문자 인덱스를 찾기 위한 for문	
					if ((fd = open(tokens[token_count - 1], O_RDWR, 0644)) < 0){
						return false;
					}
					dup2(fd, STDIN_FILENO);
					tokens[i] = NULL;
				}
			//end redirection command
			
				if(execvp(tokens[0], tokens)==-1){	//명령문 실행, 오류라면 에러메시지 출력
					if(!(strcmp(tokens[0],"cd")==0||strcmp(tokens[0],"quit")==0||strcmp(tokens[0],"exit")==0||strcmp(tokens[0],"q")==0)){
						//명령어가 cd, quit, exit, q라서 오류라면 에러메시지 출력하지 않음
						//그 외 명령어인 경우에만 에러메시지 출력
						printf("%s 라는 명령어가 없습니다\n", tokens[0]);
					}

				}
			return false;
		}
		
	}//end child process
	
	
		else{	//start parent process

			if(bg_flag==false){	//foreground process's parent
				waitpid(child, NULL, WCONTINUED);	//자식 프로세스가 끝날 때까지 기다린다
				//스레드가 하나처럼 동작
				return true;
			}

			else if(bg_flag==true){	//background process's parent
				//background process의 부모 프로세스는 자식 프로세스를 기다리지 않고 진행
				waitpid(child, NULL, WNOHANG);	//자식 프로세스가 끝날 때까지 기다린다
				//스레드가 두개처럼 동작
				return true;
			}

		}	//end parent process
	}//endelse == not pipe
	
	/* end handling redirection, pipe, bg processing */
	
	
}//endfunc (run)


int main(){

	char line[1024]={'\0'};	//명령어를 입력받을 char형 배열 line, size는 1024로 선언, '\0'로 초기화
	int i=0;	//반복문을 위한 변수 i

	while(1){
		
		printf("%s$ ", get_current_dir_name());	//현재 디렉토리의 경로를 출력하고
		//현재 경로 출력
		
		fgets(line, sizeof(line)-1, stdin);	//표준입력인 키보드로 통해 입력 받기를 대기한다
		//입력 대기 및 데이터 입력
		
		for(i=0;line[i]!='\n';i++);	//개행 문자 인덱스를 찾기 위한 for문	
		
		if(line[0]!='\n'){	//입력이 개행문자가 아니라면
			line[i]='\0';	//개행문자가 들어있는 인덱스에 널값을 저장한다
		}
        //명령어를 담은 line 배열 처리 완료
		
		//명령어 실행 시작
		if(run(line)==false){	//명령어를 입력받은 line을 run 함수에 인자로 넣어 실행시키고, false가 뜬다면
			break;	//무한반복문을 탈출한다
		}//endif
		
	}//endwhile
	
	return 0;	//0을 return 하면서 main 함수를 종료시킨다
	
}//endfunc (main)
