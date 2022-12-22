/* myshell.c	: My Shell
 * Update Date  : 22.12.8.
 * Name         : 구희찬
 * Student Num  : 32195083
 * email		: koogmlcks@dankook.ac.kr
 */

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/wait.h>

int tokenize(char* buf, char* delims, char* token[], int maxToken_count) {
	int token_count = 0;
	char* word; 	

    word = strtok(buf,delims); // 빈 칸을 기준으로 문자열 자르기

	while(word != NULL && token_count <= maxToken_count) {
		token[token_count] = word; 	 // 자른 명령어를 저장
		token_count++; // null을 만나 문자열이 끝나면 count++
		word = strtok(NULL, delims); // 탐색한곳 뒤부터 다시 구분자를 찾아 자름
	}

	token[token_count] = '\0';
	return token_count;

}

int run(char* line) {
    int size; // 입력받은 토큰의 수
    int i;
    int background_on = 0;	// 백그라운드 상태 변수
    int redirection_on = 0;	// 리다이렉션 상태 변수
    int redir_input = 0;	// 리다이렉션 방향
    int redir_output = 0;	// 리다이렉션 방향
    int redir_fd;			// 리다이렉션 fd
    pid_t pid;
    char* delims = "\n ";	// 공백 찾기용
    char* tokens[50];		// 자른 문자열 저장하는 2차원 배열

    size = tokenize(line, delims, tokens, sizeof(tokens) / sizeof(char*));

    if(!strcmp(tokens[0], "cd")) {
        if(size == 1) { // 아무 인자 값 없으면 
            chdir(getenv("HOME")); // 홈으로 이동
            return 1;
        } else if(size == 2) { // 입력한 경로로 이동
            chdir(tokens[1]);
            return 1;
        } else { // 인자 값의 경우가 다른 경우
            printf("error!\n");
            return 1;
        }
    }

	if(!strcmp(tokens[0], "exit") || !strcmp(tokens[0], "quit"))
        return 0; // exit 또는 quit 명령어로 myshell 종료
	
    for(i = 0; i < size; i++) { // 백그라운드와 리다이렉션
        if(tokens[i][0] == '&') {
            background_on = 1;
        } if(tokens[i][0] == '>') {
            redirection_on = 1;
            redir_output = i;
        } if(tokens[i][0] == '<') {
            redirection_on = 1;
            redir_input = i;
        }
    }

    pid=fork(); // fork 수행
	
    if(pid < 0) { // fork에 실패한 경우
        printf("fork failed.");
        exit(-1);
    }
	
	else if(pid == 0) { // pid가 자식 프로세스라면
        if(redirection_on == 1) { // 리다이렉션
            if(redir_output != 0) { // >
                redir_fd = open(tokens[redir_output+1], O_RDWR | O_CREAT | O_TRUNC, 0640);
                dup2(redir_fd,STDOUT_FILENO);
                tokens[redir_output]='\0';
            } 
            if(redir_input != 0) {  // <
                redir_fd = open(tokens[redir_input+1], O_RDWR, 0640);
                dup2(redir_fd, STDIN_FILENO);
                tokens[redir_input]='\0';
            }
            close(redir_fd);
        }
		
        execvp(tokens[0],tokens);
        return 1;
    }
	
	else { // pid가 부모 프로세스면
        if(background_on == 1) // 백그라운드 실행이면
            return 1; // 다음 명령어 입력 대기
		
        wait(pid); // 부모는 wait
    }
	
    return 1;
}

int main() {
    char line[1024]; 
	
    while(1) { 
        printf("%s$ ", getcwd(line, 1024)); // 현재 경로 출력
        fgets(line, sizeof(line), stdin); // 한 줄로 명령어 입력받기
        
		if(run(line) == 0) // run 함수 실행하다가 0을 리턴하면 함수 종료
			break;
    }
	
    exit(0);
}