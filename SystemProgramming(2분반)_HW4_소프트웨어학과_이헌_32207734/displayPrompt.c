/**
 * displayPrompt.c : file that include prompt display function
 * @author		: Lee Heon (32207734)
 * @email		: knife967@gmail,com
 * @version		: 1.0
 * @date		: 2022.11.21 ~ 2022.12.09
**/

#include "myshell.h"


void *displayPrompt(char charBuffer[]) {
	char hostNameBuffer[MAX], userNameBuffer[MAX], dirNameBuffer[MAX];
	void *line;
	
	// user@host:directories# 출력을 위해 해당 값 얻어오기
	gethostname(hostNameBuffer, MAX);
	getlogin_r(userNameBuffer, MAX);
	getcwd(dirNameBuffer, MAX);
	
	// 색상 설정 (십의 자리 3 -> 9 사용 시, 기존 터미널과 동일 색상으로 출력 가능)
	printf("\x1b[36m[myShell]");
	printf("\x1b[32m%s@%s", userNameBuffer, hostNameBuffer);
	printf("\x1b[37m:");
	printf("\x1b[34m@%s", dirNameBuffer);
	printf("\x1b[37m# ");

	// 명령어 라인 받아오기
	line = fgets(charBuffer, MAX, stdin);

	// '\n'를 null character값으로 치환함으로써 엔터 입력 시 String으로 인식되도록 전환
	if (charBuffer[strlen(charBuffer) - 1] == '\n') {
		charBuffer[strlen(charBuffer) - 1] = '\0';
	}

	return line;
}
