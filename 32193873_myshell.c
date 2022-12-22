/*
	Name : siwoo Jang
	Student Number : 32193873
	Major : Software
	Date : 2022-12-09
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>



int tokenize( char* buf, char* delims, char* tokens[], int maxTokens) {
	char* token;
	int token_count=0;
	int fd_rd, fd_backg;
	token = strtok(buf, delims);
	while( token != NULL && token_count < maxTokens){
		tokens[token_count] = token;
		token_count++;
		token = strtok(NULL, delims);
	}
	
	tokens[token_count] = NULL;
	return token_count;
}

bool run ( char* line ) {
	int token_count; char* tokens[1024]; char* delims = " \n\t";
	int fd_rd, i , j;
	int BFlag = 0;
    int LFlag = 0;
	int LLFLAG = 0;
	char* tokencp[1024];
	pid_t child;
	token_count = tokenize(line, delims , tokens, sizeof(tokens)/ sizeof(char*));

	//exit 입력되면 0 반환해서 끝내기	
	if ( strcmp(tokens[0],"exit") == 0){
		return 0;
	}

	//cd 입력될때 디렉토리 이동 가능하게 하기
	if (strcmp(tokens[0],"cd")==0){
		
		chdir(tokens[1]);
		return 1;
	}
	
	int Findex; // redirection 대상 파일
	//redirection 몇번 도는지 세고 + 1 
	for ( i=0; i<token_count; i++){
		if ( strcmp(tokens[i], "&")==0){
			BFlag = 1;
			tokens[i]=NULL;
		}else if ( strcmp(tokens[i],">>") == 0) { //redirection 파일 뒤에 쓰기 구현
			Findex = i+1;
			
			LLFLAG=1;//LFlag = 1;
			tokens[i] = NULL;
		}
		else if ( strcmp(tokens[i],">") == 0) { //redirection 표준 출력(덮어쓰기) 구현 
			Findex = i+1;
			LFlag = 1;
			tokens[i] = NULL;
		}
		
	}
	if( (child = fork()) == 0 ) { // fork()와 execvp 구현 
		if (LLFLAG){
			fd_rd = open(tokens[Findex],O_WRONLY|O_CREAT| O_APPEND);
			dup2(fd_rd, STDOUT_FILENO);
		}
		else if(LFlag){
			fd_rd = open(tokens[Findex],O_WRONLY|O_CREAT| O_TRUNC);
			dup2(fd_rd, STDOUT_FILENO);
		}
		execvp( tokens[0], tokens);
		exit(0);
	}
	//only run wait if not background
	if(!BFlag){
		wait();
	}
	
	return 1;
	
}

int main(int argc, char* argv[]) {
	
	char line[1024];
	while(1) {
		printf(" %s$", get_current_dir_name() );
		fgets(line,sizeof(line)-1,stdin);
		if ( run(line) == 0 ) break;
	}
	
	return 0;
}