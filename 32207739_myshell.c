#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>

#define EOL			1 // ���� ��
#define ARG			2 // ������ �μ�
#define	AMPERSAND	3
#define SEMICOLON	4
#define PIPE		5 // ������ ����� ���

#define MAXARG		512 // ����μ��� �ִ��
#define	MAXBUF		512 // �Է����� �ִ����
#define MAXPIPE		5

#define	FOREGROUND	0 // ����׶��� ����
#define	BACKGROUND	1 // ��׶��� ����

static char inpbuf[MAXBUF], tokbuf[2 * MAXBUF], *ptr = inpbuf, *tok = tokbuf; // ���α׷� ���� �� �۾��� ������
static char special[] = { ' ', '\t', '&', ';', '\n', '\0' }; // �Ϲ����� ��ɾ� ���ڰ� �ƴ� ���
char *prompt = "Command> "; // ������Ʈ
static struct sigaction act;

int userin(char*);
int gettok(char**);
int inarg(char);
int procline(void);
int runcommand(char**, int, int);
int join(char**, char**);
void sepstring(char**, char**, char**);
int fatal(char *s) { perror(s);	exit(1); }
int userin(char *p) { // ��>�� ���Ŀ� ���� ������Ʈ ����Ʈ�ϰ� �д� �Լ�
	int c, count;
	// ���α׷� ���ۿ� �۾��� ������ �ʱ�ȭ
	ptr = inpbuf;
	tok = tokbuf;

	printf("%s", p); // ������Ʈ ǥ��

	count = 0; // �Է��� ũ��

	while (1) {
		if ((c = getchar()) == EOF) // �� �� �Է� ����
			return EOF;

		if (count < MAXBUF)
			inpbuf[count++] = c; // inpbuf�� �ܾ �ϳ��� ����

		if (c == '\n' && count < MAXBUF) {
			inpbuf[count] = '\0'; // ������ ���� null�� ����
			return count;
		}

		if (c == '\n') { // count == MAXBUF�� �Ǹ� �����
			printf("smallsh: input line is too long\n");
			count = 0;
			printf("%s", p);
		}
	}
}

int gettok(char **outptr) {  // ��ū�� �����ͼ� tokbuf�� �ִ� �Լ�
	int type;

	*outptr = tok; // outptr�� ����Ű�� ���ڿ��� tok���� ��

	while (*ptr == ' ' || *ptr == '\t') // ���� ����
		ptr++;

	*tok++ = *ptr; // ��ū ������(*tok)�� ���۳� ù ��ū���� ����

	switch (*ptr++) { // ��ū�� ���� ���� ������ ����
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
		while (inarg(*ptr)) // ��ȿ�� ���빮�ڵ��� ��� ����
			*tok++ = *ptr++;
	}

	*tok++ = '\0';
	return type;
}

int inarg(char c) { // ��ȿ�� ���빮�ڵ��� ��� �о���̴� �Լ�
	char *wrk;

	for (wrk = special; *wrk; wrk++) {
		if (c == *wrk)
			return 0;
	}

	return 1;
}

int procline(void) { // �Է��� ó��
	char *arg[MAXARG + 1]; // runcommand�� ���� ������ �迭
	int toktype; // ��� �� ��ū�� ����
	int narg; // number of arguments
	int type1;  // FOREGROUND of BACKGROUND
	int type2 = 0; // PIPE type

	narg = 0;

	while (1) { // ���ѷ���
		switch (toktype = gettok(&arg[narg])) { // ��ū ������ ���� �ൿ�� ��
		case ARG:
			if (narg < MAXARG)
				narg++;
			break;
		case EOL:
		case SEMICOLON:
		case AMPERSAND:
			if (toktype == AMPERSAND) // '&'�̸� ��׶��� or ����׶���
				type1 = BACKGROUND;
			else
				type1 = FOREGROUND;

			if (narg != 0) {
				arg[narg] = NULL;
				if (strcmp(arg[0], "logout") == 0) 
					return -1;	// �α׾ƿ��ϸ� -1
				runcommand(arg, type1, type2); // ��ɾ� ����
			}

			if (toktype == EOL) 
				return 0;		// �� ���� ������ 0 ����
			narg = 0;
			break;
		case PIPE:
			type2 = toktype;	// type2 : ������Ÿ��
			if (narg < MAXARG)	
				narg++;
			break;
		}
	}
}

int runcommand(char **cline, int where, int pipe) { // ��� ����
	pid_t pid;
	int status;

	if (strcmp(cline[0], "cd") == 0) { // cd ��ɾ�
		if (chdir(cline[1]) == -1) { // ���丮 �ٲ�
			fatal("change directory fail");
		}
		return 0;
	}

	switch (pid = fork()) { // ���μ��� fork()
	case -1:
		fatal("smallsh");
		return -1;
	case 0: // �ڽ��ڵ�
		if (where == BACKGROUND) { // ��׶��忡���� �ñ׳� ����
			act.sa_handler = SIG_IGN;
			sigaction(SIGINT, &act, NULL);
			sigaction(SIGQUIT, &act, NULL);
		} else { // ����׶��忡�� �ñ׳� ȿ�� ����
			act.sa_handler = SIG_DFL;
			sigaction(SIGINT, &act, NULL);
			sigaction(SIGQUIT, &act, NULL);
		}

		if (pipe == PIPE) { // ������ ����� ���
			char* tmp[MAXPIPE] = { 0 };
			char* tmp2[MAXPIPE] = { 0 };
			sepstring(tmp, tmp2, cline); // ���ڿ��� tmp�� tmp2�� �и�
			join(tmp, tmp2); // ������ ����

			exit(1);
		}
		execvp(*cline, cline); // ��ɾ� ����
		fatal(*cline);
		exit(1);
	}
	// �θ��ڵ�
	if (where == BACKGROUND) { // ��׶���� �ĺ��� ���
		printf("[Process id %d]\n", pid);
		return 0;
	}

	if (waitpid(pid, &status, 0) == -1)
		return -1;
	else
		return status;
}

int join(char *com1[], char *com2[]) { // ������ ����
	int p[2], status;
	char* tmp[MAXPIPE] = { 0 };
	char* tmp2[MAXPIPE] = { 0 };

	if (pipe(p) == -1) // ������ ����
		fatal("pipe call in join");

	switch (fork()) { //�ٸ� ���μ��� ����
	case -1: // ����
		fatal("2nd fork call in join");
	case 0:
		dup2(p[1], 1); // ǥ�� ����� �������� ���� ��
		close(p[0]);
		close(p[1]);
		execvp(com1[0], com1);
		fatal("1st execvp call in join");
	default:
		dup2(p[0], 0); //ǥ�� �Է��� �������κ��� ���� ��
		close(p[0]);
		close(p[1]);

		sepstring(tmp, tmp2, com2);

		if (tmp2[0] == 0) { // ������ ��ɾ ������ ������ ����
			execvp(com2[0], com2);
			fatal("2nd execvp call in join");
		} else {
			join(tmp, tmp2); // ��ͽ������� �������� �������� ���� ����
		}
	}
}
void sepstring(char *com1[], char *com2[], char *str[]) {
	int j = 0;
	for (int i = 0; str[i] != 0; i++) { 
		char *tmp = str[i]; // tmp�� str�� ��Ҹ� �ϳ��� ����
		while (1) {
			if (*tmp == '|') { // ������ ���� |�� ���
				j = i + 1;
				while (str[j]) { // | �ڿ� �ִ� ��ɾ com2�� ����
					com2[j - i - 1] = str[j];
					j++;
				}
				com2[j - i - 1] = '\0'; // ���ڿ��� ���� null�� ����
				return;
			}
			if (*tmp++ == '\0') // ���ڿ� ��(null)�� ������ ����
				break;
		}
		com1[i] = str[i]; // com1�� ���ڿ� ����
		com1[i + 1] = 0;
	}
}

int main(void) {
	act.sa_handler = SIG_IGN; 
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGQUIT, &act, NULL);
	while (userin(prompt) != EOF) {		// ��ɾ� ����
		act.sa_handler = SIG_IGN;
		sigaction(SIGINT, &act, NULL);
		sigaction(SIGQUIT, &act, NULL);
		if (procline() == -1)
			break;
	}

	return 0;
}