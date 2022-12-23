/*
* 이름: 오세강
* 학번: 32212610
* 마지막 수정일: 2022.12.08
*/

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define ENDOFLINE	1 // 라인의 끝인 경우
#define ARGUMENT	2 // 올바른 인수인 경우
#define	AMP			3 // &인 경우(백그라운드 명령)
#define PIPE		4 // 파이프 명령인 경우

#define	BUFMAX		512 // 받을 수 있는 입력문의 최대 길이
#define ARGMAX		128 // 받을 수 있는 인수의 최대 갯수
#define MAXPIPE		5

#define	FOREGROUND	0 // foreground 실행
#define	BACKGROUND	1 // background 실행

char *prompt = "Myshell> "; // prompt
static struct sigaction act;
static char inputbuffer[BUFMAX], tokkenbuffer[2 * BUFMAX], *inp = inputbuffer, *tok = tokkenbuffer; // 프로그램의 버퍼 저장 변수 및 포인터
static char unnomal[] = { ' ', '\t', '&', ';', '\n', '\0' }; // 특별한 문자인 경우

int input_commend(char*);
int distok(char**);
int disarg(char);
int disline(void);
int perform(char**, int, int);
int perpipe(char**, char**);
void seperate(char**, char**, char**);
int fatal(char *s) { perror(s);	exit(1); }

int input_commend(char *prom) { // prompt를 출력한 후 한줄을 받아 읽음
	int c, cnt;
	inp = inputbuffer;
	tok = tokkenbuffer;

	printf("%s", prom); // prompt 출력

	cnt = 0; // 입력 받은 크기를 저장할 변수 초기화

	while (1) {
		if ((c = getchar()) == EOF) // 한 줄을 입력 받음
			return EOF;

		if (cnt < BUFMAX)
			inputbuffer[cnt++] = c; // 단어별로 하나씩 넣기

		if (c == '\n' && cnt < BUFMAX) {
			inputbuffer[cnt] = '\0'; // 마지막에 null 추가
			return cnt;
		}

		if (c == '\n') { // 입력받은 명령어가 버퍼의 크기를 넘어갈 경우 에러 메세지 출력 및 다시 시작
			printf("Error: Please reduce the number of characters entered\n");
			cnt = 0;
			printf("%s", prom);
		}
	}
}


int perform(char **arg, int isForB, int ispipe) { // 명령 수행
	pid_t pid;
	int sta;

	if (strcmp(arg[0], "cd") == 0) { // 명령어가 cd인 경우 chdir을 통해 디렉토리를 바꾼후 리턴
		if (chdir(arg[1]) == -1) { 
			fatal("can not change directory");
		}
		return 0;
	}

	switch (pid = fork()) { // 포크를 통해 프로세스 생성
	case -1:
		fatal("smallsh");
		return -1;
    //자식 코드일 경우
	case 0:
		if (isForB == BACKGROUND) { // 만약 background라면 시그널 무시
			act.sa_handler = SIG_IGN;
			sigaction(SIGINT, &act, NULL);
			sigaction(SIGQUIT, &act, NULL);
		} else { // foreground인 경우
			act.sa_handler = SIG_DFL;
			sigaction(SIGINT, &act, NULL);
			sigaction(SIGQUIT, &act, NULL);
		}

		if (ispipe == PIPE) { // 만약 파이프 명령이라면
			char* temp0[MAXPIPE] = { 0 };
			char* temp1[MAXPIPE] = { 0 };
			seperate(temp0, temp1, arg); // temp0와 temp1로 나누기
			perpipe(temp0, temp1); // 파이프 실행

			exit(1);
		}
		execvp(*arg, arg); // 명령어 실행
		fatal(*arg);
		exit(1);
	}
	// 부모코드 일 경우
	if (isForB == BACKGROUND) { // background 라면 식별자 출력
		printf("PROCESS ID: %d\n", pid);
		return 0;
	}

	if (waitpid(pid, &sta, 0) == -1)
		return -1;
	else
		return sta;
}


int disarg(char c) { // 문자가 unnomal에 해당하는지 검사
	char *argchar;

	for (argchar = unnomal; *argchar; argchar++) {
		if (c == *argchar)
			return 0;
	}

	return 1;
}


int distok(char **argtok) {  // tokkenbuffer에 토큰은 넣는 함수
	int istype;

	*argtok = tok; // outptr 문자열을 tok로 지정

	while (*inp == ' ' || *inp == '\t') // 토큰을 포함한 버퍼로부터 공백 제거
		inp++;

	*tok++ = *inp; // 토큰 포인터를 버퍼내 첫 토큰으로 지정

	switch (*inp++) { // 버퍼내의 토큰에 따라 유형 변수를 지정, 파이프(|) 추가
	case '\n':
		istype = ENDOFLINE;
		break;
	case '&':
		istype = AMP;
		break;
	case '|':
		istype = PIPE;
		break;
	default:
		istype = ARGUMENT;
		while (disarg(*inp))
			*tok++ = *inp++;
	}

	*tok++ = '\0';
	return istype;
}


int disline(void) { // 입력문을 검사하여 유형 설정
	char *arg[ARGMAX + 1];
	int tokkentype; // 명령이 어떤 타입인지 저정할 변수
	int argcnt; // 인수들의 갯수 카운트할 변수
	int isForB;  // 포그라운드 인지 백그라운드 인지 저장할 변수
	int ispipe = 0; // 파이프 타입 초기설정
	static struct sigaction act;

	argcnt = 0;

	while (1) {
		switch (tokkentype = distok(&arg[argcnt])) { //토큰 타입을 확인하여 유형 설정
		case ARGUMENT:
			if (argcnt < ARGMAX)
				argcnt++;
			break;
		case ENDOFLINE:
		case AMP:
			if (tokkentype == AMP) // 만약 토큰이 &라면 백그라운드
				isForB = BACKGROUND;
			else
				isForB = FOREGROUND;

			if (argcnt != 0) {
				arg[argcnt] = NULL;
				if (strcmp(arg[0], "exit") == 0) // exit 명령의 경우라면 -1 return
					return -1;
				perform(arg, isForB, ispipe); // 명령어 실행
			}

			if (tokkentype == ENDOFLINE) // 줄의 끝이라면 0 return
				return 0;
			argcnt = 0;
			break;
		case PIPE:
			ispipe = tokkentype; // 파이프 타입임을 설정
			if (argcnt < ARGMAX) // 계속해서 문자를 읽음
				argcnt++;
			break;
		}
	}
}


int perpipe(char *commend0[], char *commend1[]) { // 파이프를 구현하기 위한 함수
	int pip[2], status;
	char* temp0[MAXPIPE] = { 0 };
	char* temp1[MAXPIPE] = { 0 };

	if (pipe(pip) == -1) // 파이프 생성
		fatal("pipe call in join");

	switch (fork()) { // 포크를 통해 다른 프로세스 생성
	case -1:
		fatal("2nd fork call in join");
	case 0:
		dup2(pip[1], STDOUT_FILENO); // dup2 함수를 통해 표준출력이 파이프로 가도록 함
		close(pip[0]);
		close(pip[1]);
		execvp(commend0[0], commend0);
		fatal("1st execvp call in join");
	default:
		dup2(pip[0], STDIN_FILENO); // dup2 함수를 통해 표준입력이 파이프에서 오도록 함
		close(pip[0]);
		close(pip[1]);

		seperate(temp0, temp1, commend1); // 명령어 나누기  

		if (temp1[0] == 0) { // 파이프 종료
			execvp(commend1[0], commend1);
			fatal("2nd execvp call in join");
		} else {
			perpipe(temp0, temp1); // 함수의 재귀를 이용하여 파이프가 여러개일때를 지원하도록 작성
		}
	}
}



void seperate(char *commend0[], char *commend1[], char *string[]) {
	int i, j = 0;
	for (i = 0; string[i] != 0; i++) { 
		char *temp = string[i];
		while (1) {
			if (*temp == '|') { // |일 경우
				j = i + 1;
				while (string[j]) { // | 후에 오는 명령어를 commend1에 저장
					commend1[j - i - 1] = string[j];
					j++;
				}
				commend1[j - i - 1] = '\0'; // 마지막에 null 삽입
				return;
			}
			if (*temp++ == '\0') // 종료
				break;
		}
		commend0[i] = string[i]; // commend1에 문자열 저장
		commend0[i + 1] = 0;
	}
}


int main(void) {
	act.sa_handler = SIG_IGN;
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGQUIT, &act, NULL);
	while (input_commend(prompt) != EOF) { // prompt 실행
		act.sa_handler = SIG_IGN;
		sigaction(SIGINT, &act, NULL);
		sigaction(SIGQUIT, &act, NULL);
		if (disline() == -1)
			break;
	}

	return 0;
}
