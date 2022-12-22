/*
main.c		: shell program
@author		: Dongyoung Kim
@date		: 2022. 12. 09
@studentID	: 32190461
@major		: software
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>

#define EOL			1 // 줄의 끝
#define ARG			2 // 정상적 인수
#define	AMPERSAND	3
#define SEMICOLON	4
#define PIPE		5 // 파이프 명령인 경우

#define MAXARG		512 // 명령인수의 최대수
#define	MAXBUF		512 // 입력줄의 최대길이
#define MAXPIPE		5

#define	FOREGROUND	0 // 포어그라운드 실행
#define	BACKGROUND	1 // 백그라운드 실행

static char inpbuf[MAXBUF], tokbuf[2 * MAXBUF], *ptr = inpbuf, *tok = tokbuf; // 프로그램 버퍼 및 작업용 포인터
static char special[] = { ' ', '\t', '&', ';', '\n', '\0' }; // 일반적인 명령어 문자가 아닌 경우
char *prompt = "Command> "; // 프롬프트
static struct sigaction act;

int userin(char*);
int gettok(char**);
int inarg(char);
int procline(void);
int runcommand(char**, int, int);
int join(char**, char**);
void sepstring(char**, char**, char**);
int fatal(char *s) { perror(s);	exit(1); }


int userin(char *p) { // 프롬프트를 프린트하고 한 줄 읽음
	int c, count;
	// 프로그램 버퍼와 작업용 포인터 초기화
	ptr = inpbuf;
	tok = tokbuf;

	printf("%s ", getcwd(NULL,0));
	printf("%s", p); // 프롬프트 표시

	count = 0; // 입력의 크기

	while (1) {
		if ((c = getchar()) == EOF) // 한 줄을 입력 받음
			return EOF;

		if (count < MAXBUF)
			inpbuf[count++] = c; // 단어 하나씩 대입

		if (c == '\n' && count < MAXBUF) {
			inpbuf[count] = '\0'; // 문장의 끝에 널을 넣음
			return count;
		}

		if (c == '\n') { // 줄이 너무 길면 재시작
			printf("smallsh: input line too long\n");
			count = 0;
			
			printf("%s ", getcwd(NULL, 0));
			printf("%s", p);
		}
	}
}

int gettok(char **outptr) {  // 토큰을 가져와서 tokbuf에 넣음
	int type;

	*outptr = tok; // outptr 문자열을 tok로 지정

	while (*ptr == ' ' || *ptr == '\t') // 토큰을 포함한 버퍼로부터 공백 제거
		ptr++;

	*tok++ = *ptr; // 토큰 포인터를 버퍼내 첫 토큰으로 지정

	switch (*ptr++) { // 버퍼내의 토큰에 따라 유형 변수를 지정, 파이프(|) 추가
	case '\n':
		type = EOL;
		break;
	case '&':
		type = AMPERSAND;
		break;
	case ';':
		type = SEMICOLON;
		break;
	case '|':
		type = PIPE;
		break;
	default:
		type = ARG;
		while (inarg(*ptr)) // 유효한 보통문자들을 계속 읽음
			*tok++ = *ptr++;
	}

	*tok++ = '\0';
	return type;
}

int inarg(char c) { // 한 문자가 보통문자인지 검사
	char *wrk;

	for (wrk = special; *wrk; wrk++) {
		if (c == *wrk)
			return 0;
	}

	return 1;
}

int procline(void) { // 입력줄 처리
	char *arg[MAXARG + 1]; // runcommand를 위한 포인터 배열
	int toktype; // 명령내 토큰 유형
	int narg; // 지금까지 인수 수
	int type1;  //FOREGROUND 또는 BACKGROUND
	int type2 = 0; // 파이프 타입

	narg = 0;

	while (1) { // 무한루프
		switch (toktype = gettok(&arg[narg])) { // 토큰 유형에 따라 행동을 함
		case ARG:
			if (narg < MAXARG)
				narg++;
			break;
		case EOL:
		case SEMICOLON:
		case AMPERSAND:
			if (toktype == AMPERSAND) // '&'이면 백그라운드 아니면 포어그라운드
				type1 = BACKGROUND;
			else
				type1 = FOREGROUND;

			if (narg != 0) {
				arg[narg] = NULL;
				if (strcmp(arg[0], "exit") == 0) // 로그아웃인 경우 -1 리턴
				{
					printf("Shell program quit\n");
					return -1;
				}
				runcommand(arg, type1, type2); // 명령어 실행
			}

			if (toktype == EOL) // 한 줄이 끝나면 0 리턴
				return 0;
			narg = 0;
			break;
		case PIPE:
			type2 = toktype; // 파이프 타입임을 설정
			if (narg < MAXARG) // 일단 ARG처럼 문자를 읽음
				narg++;
			break;
		}
	}
}

int runcommand(char **cline, int where, int pipe) { // 명령 수행
	pid_t pid;
	int status;

	if (strcmp(cline[0], "cd") == 0) { // cd 명령어인 경우
		if (chdir(cline[1]) == -1) { // 디렉토리를 바꿔주고 리턴
			fatal("change directory fail");
		}
		return 0;
	}

	switch (pid = fork()) { // 프로세스 fork
	case -1:
		fatal("smallsh");
		return -1;
	case 0: // 자식코드
		if (where == BACKGROUND) { // 백그라운드이면 시그널 무시
			act.sa_handler = SIG_IGN;
			sigaction(SIGINT, &act, NULL);
			sigaction(SIGQUIT, &act, NULL);
		} else { // 포어그라운드이면 시그널 다시 효력 생김
			act.sa_handler = SIG_DFL;
			sigaction(SIGINT, &act, NULL);
			sigaction(SIGQUIT, &act, NULL);
		}

		if (pipe == PIPE) { // 파이프 명령인 경우
			char* tmp[MAXPIPE] = { 0 };
			char* tmp2[MAXPIPE] = { 0 };
			sepstring(tmp, tmp2, cline); // 문자열을 tmp와 tmp2로 분리
			join(tmp, tmp2); // 파이프 실행

			exit(1);
		}
		execvp(*cline, cline); // 명령어 실행
		fatal(*cline);
		exit(1);
	}
	// 부모코드
	if (where == BACKGROUND) { // 만일 백그라운드이면 식별자를 프린트
		printf("Background processing is running\n");
		printf("[Process id %d]\n", pid);
		return 0;
	}

	if (waitpid(pid, &status, 0) == -1)
		return -1;
	else
		return status;
}

int join(char *com1[], char *com2[]) { // 파이프 실행
	int p[2], status;
	char* tmp[MAXPIPE] = { 0 };
	char* tmp2[MAXPIPE] = { 0 };

	if (pipe(p) == -1) // 파이프 생성
		fatal("pipe call in join");

	switch (fork()) { //다른 프로세스 생성
	case -1: // 오류
		fatal("2nd fork call in join");
	case 0:
		dup2(p[1], 1); // 표준 출력이 파이프로 가게 함
		close(p[0]);
		close(p[1]);
		execvp(com1[0], com1);
		fatal("1st execvp call in join");
	default:
		dup2(p[0], 0); //표준 입력이 파이프로부터 오게 함
		close(p[0]);
		close(p[1]);

		sepstring(tmp, tmp2, com2); // 명령어 분리 해줌

		if (tmp2[0] == 0) { // 마지막 명령어가 없으면 파이프 끝임
			execvp(com2[0], com2);
			fatal("2nd execvp call in join");
		} else {
			join(tmp, tmp2); // 재귀실행으로 파이프가 여러개인 것을 지원
		}
	}
}
void sepstring(char *com1[], char *com2[], char *str[]) {
	int j = 0;
	for (int i = 0; str[i] != 0; i++) { // 띄어쓰기로 구분된 str에서 파이프를 구분
		char *tmp = str[i]; // tmp에 str의 요소를 하나 저장
		while (1) {
			if (*tmp == '|') { // 저장한 것이 |인 경우
				j = i + 1;
				while (str[j]) { // | 뒤에 있는 명령어를 com2에 저장
					com2[j - i - 1] = str[j];
					j++;
				}
				com2[j - i - 1] = '\0'; // 문자열의 끝에 널을 넣음
				return;
			}
			if (*tmp++ == '\0') // 문자가 끝나면 종료
				break;
		}
		com1[i] = str[i]; // com1에 문자열 저장
		com1[i + 1] = 0;
	}
}

int main(void) {
	
	printf("Student's ID : 32190461\n");
	printf("Student's name : Dong Young Kim\n");
	printf("Today's date is 2022-12-09\n");
	printf("-----------------------------------------------------\n");
	printf("This is mini shell program\n");
	printf("If you want to quit shell, enter exit\n");
	printf("or if you want to run shell, enter the command\n");
	printf("-----------------------------------------------------\n");
	
	
	act.sa_handler = SIG_IGN; // 시그널을 수신했을 때 무시
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGQUIT, &act, NULL);
	
	
	while (userin(prompt) != EOF) { // 명령어 실행
		act.sa_handler = SIG_IGN;
		sigaction(SIGINT, &act, NULL);
		sigaction(SIGQUIT, &act, NULL);
		if (procline() == -1)
			break;
	}

	return 0;
}