/*
 * fileName : myshell.c
 * Name     : choi seoyoung
 * number   : 32214543
 * Date     : 2022.12.09
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>


void printPrompt(){  //프롬프트를 출력하는 함수
       	char dir[200];  //현재 directory를 저장할 변수
	getcwd(dir, 200);   //getcwd()함수 실행
	printf("%s $ ", dir);  //현재 디렉토리 $ 출력
}

void ls(){
	DIR *dp;   //directory 구조체
	struct dirent *entry;  // 디렉토리 내의 파일 entry
	char *path = "."; //현재 디렉토리가 경로
	int count = 0;   //한 줄에 다섯 개씩 출력하기 위한 count 변수
	dp = opendir(path); //현재 디렉토리 open
	if(dp == NULL){   //디렉토리 열기 실패시 error handling
		return;
	}
	while((entry = readdir(dp)) != NULL){  //readdir은 dp가 가리키는 디렉토리 내 파일 목록의 주소를 반환
		if(count > 5){   //5개씩 화면에 출력하기 위해서 count 변수
			printf("\n");
			count = 0;
		}
		if(entry->d_name[0] == '.') continue;  //숨김파일이라면 출력하지 않음
		printf("%15s", entry->d_name);  //파일이나 디렉토리 이름 출력
		count += 1;
	}
	printf("\n");
	return;
}

void cat(char *tokens[]){
	int  c;  //file을 한 글자씩 읽고 저장할 변수
	FILE *fp;  //fopen했을 떄 이 값을 저장할 변수

	if((fp = fopen(tokens[1], "r")) == NULL){   //read모드로 파일연다
		printf("cat 명령어 실패");
		return; //error handling
	}
	while((c = getc(fp)) != EOF){  //파일이 끝날 때까지 반복
		putchar(c);  //파일 내용의 한 글자씩 화면에 출력
	}
	fclose(fp);  //fp를 닫음

}
int tokenize(char *buf, char* delims, char* tokens[], int maxTokens){
	int count = 0;  //명령문을 공백을 기준으로 잘랐을 때 총 나눠진 개수를 세기 위한 변수
	char *temp = strtok(buf, delims); //delims을 기준으로 문자열 자르기

	while(temp != NULL && count < maxTokens) {  /*NULL이 아닐 떄까지(문장의 끝이 날 떄까지) + tokens에 저장할 수 있는 문자열
		(윗 주석 이어서 설명) 개수보다 명령어 개수가 작을 떄까지 반복 */
		tokens[count] = temp;		//주어진 명령어를 delims을 기준으로 나눠서 순차적으로 저장
		temp = strtok(NULL, delims);  	//NULL문자를 기준으로 다시 자르기
		count++; //토큰의 개수 하나 증가
	}
	tokens[count] = '\0'; //명령어가 끝났음을 알려주기 위해서 명령어 token 뒤에 NULL 추가
	return count;
}	

bool run(char *line){    //프롬프트 옆에 사용자가 입력한 명령어를 실행하기 위한 함수
	char *delims = " \n";   //명령을 토큰으로 나눌 때의 기준점
	int token_count;  //토큰의 개수
	char *tokens[32];  //토큰들을 저장할 변수
	int i = 0;  //반복문을 진행할 때 변수
	int fd;   //file descriptor을 저장할 변수
	pid_t child;   //프로세스 pid를 저장할 변수
	int backprocess_index = 0;   // & 가 저장된 토큰의 인덱스
	int backprocess_check = 0;   // background process인지 check하는 변수
	int inredirection_index = 0;   // > 가 저장된 토큰의 인덱스
	int inredirection_check = 0;   // redirection이 실행됐는지 check하는 변수
	int outredirection_index = 0;  // < 가 저장된 토큰의 인덱스
	int outredirection_check = 0; //output redirection이 실행됐는지 check하는 변수
	int status;   //자식 프로세스의 상태를 저장할 변수
	
	token_count = tokenize(line, delims, tokens, sizeof(tokens)/sizeof(char*));  //token의 개수
	
	if(token_count == 0){
		return true;
	}
	if(strcmp(tokens[0], "exit")== 0){   //exit 입력 시 shell 종료
		return false;
	}
	while(tokens[i] != '\0') {     //명령어가 끝날 때까지 반복
		if(strcmp(tokens[i], "&") ==  0){   //명령어에 &가 포함된다면 즉,  background processing 명령어라면
			backprocess_index = i;   // &가 저장된 토큰 인덱스를 저장
			backprocess_check = 1;
			break;
		}
		if(strcmp(tokens[i], ">")== 0){  //명령어에 >가 포함된다면 즉, ouput redirection 명령어라면
			outredirection_index = i;  //>가 저장된 토큰 인덱스를 저장
			outredirection_check = 1;
			break;
		}
		if(strcmp(tokens[i], "<") == 0){  //명령어에 <가 포함된다면 즉, input redirection 명령어라면
			inredirection_index = i;  //<가 저장된 토큰 인덱스 저장
			inredirection_check = 1;  
		}
		i++;
	}
	if(strcmp(tokens[0], "ls") == 0 && backprocess_check == 0 && outredirection_check == 0 && inredirection_check == 0){
		ls();
		return true;
	}
	if(strcmp(tokens[0], "cat") == 0 && backprocess_check == 0 && outredirection_check == 0 && inredirection_check == 0){
		cat(tokens);
		return true;
	}

    	child = fork();	 //자식 프로세스 실행
	
	if(child == -1){  // 자식 프로세스를 실행했는데 제대로 실행되지 않았다면
		printf("fork error");  //error handling
		return false;    //false 반환하여 shell 종료
	}

	else if(child == 0){  //자식 프로세스라면
		if(backprocess_check == 1){   //background process가 실행되었다면 
			tokens[backprocess_index] = '\0';  //&대신에 null 저장
			printf("\n--------------background process--------------\n");
			execvp(tokens[0], tokens);   //명령 실행(token에서 null만나기 전까지)
			
		}
		if(outredirection_check == 1){
			fd = open(tokens[outredirection_index + 1], O_RDWR | O_CREAT, 0641);  // > 뒤에적힌 파일을 연다
			close(STDOUT_FILENO);  //STDOUT_FILENO가 열려있다면 닫음
			dup2(fd, STDOUT_FILENO);   //STDOUT_FILENO의 모든 출력이 fd로 됨
			tokens[outredirection_index] = '\0';   // >대신 null 저장
			execvp(tokens[0], tokens); // 명령 실행(token에서 null 만나기 전까지)
		}
		if(inredirection_check == 1){   //input redirection이라면
			fd = open(tokens[inredirection_index + 1], O_RDONLY);  // < 명령어 뒤의 파일을 연다
			close(STDIN_FILENO);   //STDIN_FILENO가 열려있다면 닫음
			dup2(fd, STDIN_FILENO);  //STDIN_FILENO의 모든 입력이 fd로 됨
			tokens[inredirection_index] = '\0';  // < 대신 null로 저장
			execvp(tokens[0], tokens);  //명령 실행(token에서 null 만나기 전까지)
		}
		return false;  //execvp 실패했다면 false 반환
	}

	else if(backprocess_check == 0){  //부모 프로세스 실행 중인데, 명령어가 background process가 아니라면
		wait(&status);  //자식 프로세스가 끝날 떄까지 wait
	}
	return true;  //제대로 실행되었다면 true 반환
	
}

int main(){
	char line[128];   //입력받은 명렁어를 저정할 배열    
	
	while(1){  
		printPrompt();    //프롬프트 출력해주는 함수
		fgets(line, sizeof(line) - 1 , stdin);//stdin에서 문자열을 읽어와서 line 에 저장
		bool X = run(line);  //명령어를 실행시켰을 떄(run)
		if(X == false)    // false값이 나오면 shell 종료
			break;
	}
	return 0;
}
