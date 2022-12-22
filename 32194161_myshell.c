/**
 * mysh.c : shell program
 * @author : Wookjae Jeong
 * @email : 32194161@gmail.com
 * @version : 1.0
 * @date : 2022.12.09
 **/

#include <stdio.h>			// 표준 입출력 헤더 파일
#include <string.h>	   // 문자열 처리 헤더 파일
#include <stdlib.h>	   // 동적 할당을 위한 헤더 파일
#include <unistd.h>	   // getcwd와 fork를 사용하기 위한 헤더 파일
#include <stdbool.h>   // bool형을 사용하기 위한 헤더 파일
#include <sys/wait.h>  // wait을 사용하기 위한 헤더 파일
#include <sys/types.h> // 시스템에서 사용하는 자료형이 저장되어 있는 헤더 파일
#include <fcntl.h>	   // open 함수를 사용하기 위한 헤더 파일
#define size 30000 // shell이 한번에 읽어올 데이터의 크기

bool mysh_help()	// 도움말 구현
{
	printf("----------------------------도움말---------------------------\n");
	printf("background와 redirection이 가능한 shell\n");
	printf("명령어 사용 방법: command [Option] (Argument)\n");
	printf("기본 명령어들은 모두 사용할 수 있습니다.\n");
	printf("exit를 입력하면 이전 경로로 이동합니다.\n");
	printf("shell을 실행한 위치에서 exit를 입력하면 shell을 종료합니다.\n");
	printf("--------------------------------------------------------------\n");
}

bool mysh_cd(int argc, char* argv[])	// cd 구현
{
	if (argc == 1) {					// 인자가 1개인 경우
		chdir(getenv("HOME"));	// 홈으로 이동 (root)
	}
	else if (argc == 2)	{		// 인자가 2개인 경우
		if (chdir(argv[1]))	{	// 디렉토리가 없거나 오류가 발생한 경우
			printf("디렉토리를 찾을 수 없습니다.\n");
		}
	}
	else {							// 인자가 3개 이상인 경우
		printf("명령어 사용 방법 : cd (디렉토리 이름)\n");
	}
}

char* showPath()	{			 // 현재 위치를 요약해서 출력하기 위해 환경변수를 사용하여 함수 구현
	char* buf, * path;			 // 경로를 저장할 변수
	int i = 0;					     // strtok 함수에서 사용할 인덱스
	path = (char*)malloc(2048); // heap 영역에서 2^11 만큼의 공간을 동적 할당
	char* save[20];				 // 5개의 문자형 포인터 배열 (주소를 저장하는 공간)
	char* show;					 // 마지막 주소값

	if (getcwd(path, 2048) == NULL)	 {  // 현재 작업중인 디렉토리 호출, NULL이면 오류 발생
		printf("오류 발생");
	}
	buf = strtok(path, "/"); // 리눅스는 '/'를 기준으로 경로가 구분되기 때문에 '/'를 기준으로 slice 진행

	while (buf != NULL)  {	// '/'를 기준으로 slice를 하고 그 자리에 \0을 삽입, 마지막에 NULL값 삽입
		save[i] = buf;			// slice한 주소 저장
		buf = strtok(NULL, "/"); // slice한 부분부터 다음 부분을 찾는 역할(strtok 사용방법)
		show = save[i]; // 마지막 주소값
		i++;
	}
	free(path);	  // 동적 할당 해제
	return show; // 마지막 값 반환
}

void input(char* user, char* pwd_path, char value[]) { // 반복적으로 사용되는 부분 (핵심 부분)
	printf("%s@%s $", user, pwd_path);	// 경로를 축약해서 출력
	fgets(value, size, stdin); // 값을 입력받기 위한 함수.
	if (value[strlen(value) - 1] == '\n')
		value[strlen(value) - 1] = '\0'; // fgets에 추가되는 개행문자를 지움
}

int main()
{
	printf("사용자의 학번 입력 : ");
	char user[9] = " ";
	fgets(user, 9, stdin);	 // user에 문자열을 가져옴
	getchar();					 // 입력버퍼 삭제
	char* pwd_path = showPath(); // 유저의 경로 리턴
	char value[size];			 // 입력 받고 저장되는 변수
	int run_var = 1;			 // 프로세스 종료 목적의 변수

	pid_t pid; // pid정보를 저장할 변수
	char* arg; // 입력 받은 값을 slice하고 결과를 저장하기 위한 변수
	int stat;  // pid wait의상태를 저장하는 변수
	
	while (true) {
		input(user, pwd_path, value);

		if (value[strlen(value) - 1] == '&') // background 구현 (명령어에 & 존재)
		{
			value[strlen(value) - 2] = '\0'; // &가 들어갈 자리를 NULL로 바꿈
			if ((pid = fork()) < 0) { // pid는 항상 0보다 크기 때문에 0보다 작으면 오류 발생
				perror("fork 오류");
				exit(1);
			}
			else if (pid == 0) {			 // 자식 프로세스 구현 부분
				char* argv[size];		 // 명령문을 쪼개서 명령문과는 별개로 조건들을 저장하는 공간
				arg = strtok(value, " "); // 공백을 기준으로 구분
				int count = 1;			 // 인자의 수
				argv[0] = arg;			 // 제일 처음단어 다음에 나오는 " "을 \0으로 교체
				
				while (arg != NULL) {
					arg = strtok(NULL, " "); // slice한 부분부터 다음 부분을 찾는 역할
					if (arg != NULL)	{
						argv[count] = arg;
						count++;
					}
				}  //	 count가 1이면 해당 문자의 끝을 의미함 (인자 1개)
				argv[count] == NULL;

				if (count == 1) {					 // 인자가 1개인 경우
					if (!strcmp(argv[0], "cd")) { // cd 명령어
						mysh_cd(count, argv);
						pwd_path = showPath(); // 유저의 경로를 리턴
					}
					else if (!strcmp(argv[0], "exit")) { // exit 명령어
						pid_t t = getppid();
						kill(t, SIGKILL);	// 강제 종료
						exit(0);
					}
					else if (!strcmp(argv[0], "help"))  {	// 도움말 출력
						mysh_help();
					}
					else
						execlp(value, value, (char*)0);
				}
				else  {			// 인자가 2개 이상인 경우, redirection 구현
					int index = 0;		//	redirection의 인덱스
					for (; argv[index] != NULL; index++)	{
						if (!strcmp(argv[index], ">"))  { // 문자열 비교 (같으면 0 반환)
							break;
						}
					}

					if (argv[index] == NULL) {  // redirection이 아닌 경우,
						if (!strcmp(argv[0], "cd")) {	// cd 명령어
							mysh_cd(count, argv);
							pwd_path = showPath(); // 유저의 경로를 리턴
						}
						else
							execvp(argv[0], argv);	// 배열을 기준으로 사용
					}
					else	{ // redirection 인 경우
						int fd = open(argv[index + 1], O_RDWR);
						if (fd == -1)	{
							perror("Open 오류");
							exit(0);
						}
						dup2(fd, STDOUT_FILENO);	// unistd.h 에 저장
						
						if (!strcmp(argv[0], "cd")) {	// cd인 경우
							mysh_cd(count, argv);
							pwd_path = showPath(); // 유저의 경로 리턴
						}
						else {
							for (; argv[index] != NULL; index++) {
								argv[index] = '\0';
							}
							execvp(argv[0], argv);	// 배열을 기준으로 사용
						}
						close(fd);
					}
				}
			}
			printf("백그라운드 실행 완료 \n");
		}

		else
		{
			if ((pid = fork()) < 0) {  // pid는 항상 0보다 크기 때문에 0보다 작으면 오류 발생
				perror("fork 오류");
				exit(1);
			}
			else if (pid == 0) {			 // 자식 프로세스
				char* argv[size];		 // 명령문을 쪼개서 명령문과는 별개로 조건들을 저장하는 공간
				arg = strtok(value, " "); // 공백을 기준으로 구분
				int count = 1;			 // 인자의 수
				argv[0] = arg;			 // 제일 처음단어 다음에 나오는 " "을 \0으로 교체

				while (arg != NULL) {
					arg = strtok(NULL, " "); // slice한 부분부터 다음 부분을 찾는 역할
					if (arg != NULL) {
						argv[count] = arg;
						count++;
					}
				}   //	 count가 1이면 해당 문자의 끝을 의미함 (인자 1개)
				argv[count] == NULL;

				if (count == 1) {					 // 인자가 1개인 경우
					if (!strcmp(argv[0], "cd")) {	// cd 명령어
						mysh_cd(count, argv);
						pwd_path = showPath(); // 유저의 경로를 리턴
					}
					else if (!strcmp(argv[0], "exit")) {	// exit 명령어
						pid_t t = getppid();
						kill(t, SIGKILL);		// 강제 종료
						exit(0);
					}
					else if (!strcmp(argv[0], "help")) {	// 도움말 출력
						mysh_help();
					}
					else
						execlp(value, value, (char*)0);
				}

				else {	// 인자가 2개이상인 경우
					int index = 0; // redirection에 대한 index
					for (; argv[index] != NULL; index++)	{
						if (!strcmp(argv[index], ">"))	{ // 문자열끼리 비교해서 같으면 0반환
							break;
						}
					}
					if (argv[index] == NULL)  {		// redirection이 아닌 경우
						if (!strcmp(argv[0], "cd"))	{	// cd 명령어
							mysh_cd(count, argv);
							pwd_path = showPath(); // 유저의 경로를 리턴
						}
						else
							execvp(argv[0], argv); // 배열을 기준으로 사용
					}
					else  {	// redirection
						int fd = open(argv[index + 1], O_RDWR);
						if (fd == -1)	{
							perror("Open 오류");
							exit(0);
						}
						dup2(fd, STDOUT_FILENO); // unistd.h 에 저장
						
						if (!strcmp(argv[0], "cd"))  { // cd 명령어
							mysh_cd(count, argv);
							pwd_path = showPath(); // 유저의 경로를 리턴
						}
						else  {
							for (; argv[index] != NULL; index++) {
								argv[index] = '\0';
							}
							execvp(argv[0], argv); // 배열을 기준으로 사용
						}
						close(fd);
					}
				}
			}
			else  {	// 자식프로세스가 끝날때까지 부모프로세스 대기, stat는 종료된 자식 프로세스의 상태
				wait(&stat);
			}
		}
	}
}