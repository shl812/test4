/**
 * myshell.c : myshell program
 * @author : SeJun Lim
 * @student ID : 32193767
 * @email : lsj1137@naver.com
 * @version : 1.0
 * @date : 2022.11.18
**/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

// 리다이렉션 함수
void redirect(int mode, int bgEx, char *argv[], char* input, char * output){
    int fd_in, fd_out;	// 입출력용 file descriptor 변수 생성
    pid_t child;			// 자식 프로세스 pid 저자용 변수 생성
    
    if ((child=fork()) == -1)   // fork 후 자식 pid child에 저장, 에러 났는지 확인
        perror("fork failed");	// 예외처리
    else if (child != 0) {      // 부모 프로세스인 경우
        if(bgEx==0)			    // 백그라운드 실행 안하도록 입력 받으면
            child = wait();		// 자식이 끝날때 까지 wait
        else {					// 백그라운드 실행 하도록 입력 받으면
            printf("[1] %d\n", getpid());   // [1] pid 출력
            waitpid(child, NULL, WNOHANG);  // 백그라운드 실행
        }
    } else {					// 자식 프로세스인 경우
        if (mode == 2) {		// < 일 때
            if((fd_in = open(input, O_RDONLY))==-1){	// 입력용 open 하고 에러났는지 확인
                perror(argv[0]);						// 예외처리
                exit(2);
            }
            dup2(fd_in, 0); // standard input 으로 dup
            close(fd_in);	// fd_in 종료
            execvp(argv[0], argv);  // 새 프로그램 실행
        } else if (mode == 3) { // > 일 때
            fd_out = open(output, O_CREAT|O_TRUNC|O_WRONLY, 0600);  // 출력용 open 하고 에러났는지 확인
            dup2(fd_out, 1); 	// standard output 으로 dup
            close(fd_out);		// fd_out 종료
            execvp(argv[0], argv);	// 새 프로그램 실행
        }
    }
}
// 입력 확인 함수
int chkInput(char* buf){
	// 조건에 맞는 제대로 된 입력인 경우
	if (strcmp(buf,"")!=0 && strcmp(buf,"\t")!=0 && strcmp(buf," ")!=0)
		return 0;	// 0 리턴
	return 1;		// 이상한 경우 1 리턴
}
// 토큰화 함수
int tokenize(char* buf, char* tokens[], char delim[], int maxTokens) {
	int token_count = 0;
	char* token = strtok(buf, delim);	// delim 기준으로 토큰화
	while (token != NULL && token_count < maxTokens){	// maxTokens 넘어가지 않는 선에서
		tokens[token_count++] = token;	// tokens 에 쪼개진 token 저장, token_count + 1
		token = strtok(NULL, delim);	// delim 기준으로 토큰화
	}
	return token_count;
}
// 실행 함수
int run(char* line) {
	// 각종 변수 초기화
	char* tokens;
	char* args[7];
	char* tokenedArg[7];
	char in[50],out[50];
	int token_count = 0, args_count = 0, mode = 1, bgEx = 0, n = 0, i, status, child_depth = 1;
	pid_t child;
	
	if (line==NULL || chkInput(line)==1) {	// 입력 체크 결과 1이면
		printf("잘못된 입력입니다..\n");       // 체크 결과 출력
		return 1;							// 1 리턴(다시 받아오기)
	}
	if (strchr(line, '|')!=NULL){		// 파이프 기호가 포함 되어 있는 경우
		args_count = tokenize(line, args, "|", sizeof(tokens)/sizeof(char*));   // "|" 기호 기준으로 토큰화
	} else {						// 파이프라인 기호가 없는 경우
		args_count = 1;				// args_count 를 1로 설정
		args[0]=line;				// args[0]에 입력 넣고
		args[1]=(char*)0;			// args[1]에 null 넣기
	}
	if (strcmp(args[0],"exit")==0)	// args[0], 즉 입력이 exit라면
		return 0;					// 0 리턴(종료)
	for (i=0; i<args_count; i++) {	// 토큰 개수 만큼 반복
		tokens = strtok(args[i], " ");			// args[i]에 있는 string을 띄어쓰기 기준으로 토큰화
		while (tokens) {						// tokens가 null이 아닐 때까지
			if (strcmp(tokens,"<")==0) {		// tokens가 <인 경우 (리다이렉션)
				mode = 2;						// mode 2로 변경
			} else if (strcmp(tokens,">")==0) {	// tokens가 >인 경우 (리다이렉션)
				mode = 3;						// mode 3으로 변경
			} else if (strcmp(tokens,"&")==0) { // tokens가 &인 경우 (백그라운드)
				bgEx = 1;						// bgEx 1로 변경
			} else {							// 아무것도 아닌 문자인 경우
				if (mode == 1)					// mode가 1일 때 (일반 실행)
					tokenedArg[n++] = tokens;	// tokenedArg배열에 tokens 저장
				else if(mode == 2) 				// mode가 2일 때 (<)
					strcpy(in,tokens);			// in 에 tokens 저장
				else 							// mode가 3일 때 (>)
					strcpy(out,tokens);			// out 에 tokens 저장
			}
			tokens = strtok(NULL," ");			// 띄어쓰기 기준으로 토큰화
		}
		tokenedArg[n] = (char*)0;				// tokenedArg 마지막에 null 저장
		if (mode == 1) {						// mode가 1인 경우 (일반 실행)
			if ((child = fork()) == -1)			// fork 후 자식pid child에 저장, 에러 났는지 확인
				perror("fork error");
			else if (child != 0) {				// 부모 프로세스인 경우
				if(bgEx == 0)					// 백그라운드 실행 안하는 경우
					child = wait();				// 자식 끝날때까지 wait
				else {							// 자식 프로세스인 경우
					printf("[1] %d\n", getpid()); 	// [1] pid 출력
					waitpid(child, NULL, WNOHANG);	// 백그라운드 실행
				}
			} else 
				execvp( tokenedArg[0], tokenedArg );// tokenedArg에 저장된 명령 실행
		} else if(mode > 1){						// mode가 1보다 큰 경우(리다이렉션)
			redirect(mode, bgEx, tokenedArg, in, out);	//리다이렉션 실행
		}
	}
	return 1;
}
int main() {
	while (1) {					// 무한 반복
		char line[1024];		// 명령어 저장용 line배열 생성 
		printf("%s$ ",get_current_dir_name());	//현재 디텍토리명 + $
		fgets(line,sizeof(line),stdin);		// line 입력 받기
		line[strlen(line)-1]='\0';			// 입력 끝을 널문자로 변경
		if (run(line)==0) break;			// run 함수 실행
	}
	printf("쉘을 종료합니다..\n");		//실행 종료 알림
	return 0;
}

