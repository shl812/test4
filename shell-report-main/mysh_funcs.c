/**
* shell.c:shell program
* author:lee seo bin
* email:sltenic@naver.com
* version:1.0
* date:2022.12.06
**/

char** get_tokens(char* CommandLine) {
	char** tokens;//토큰들을 저장할 배열
	char* token;
	int COUNT = 0;
	//\n 대신 \0넣음
	CommandLine[strlen(CommandLine) - 1] = '\0';
	//토큰 저장할 배열 크기
	tokens = malloc((sizeof(char*)) * MAX);
	//" "을 구분자로 자름
	token = strtok(CommandLine, " ");
	while (token != NULL) {
		//\0포함해야하므로 +1
		tokens[COUNT] = malloc(strlen(token) + 1);
		//동적할당 후 내용복사 
		strncpy(tokens[COUNT], token, strlen(token) + 1);
		token = strtok(NULL, " ");
		COUNT++;
	}
	//토큰들 입력 끝나면 null
	tokens[COUNT] = NULL;
	return tokens;
}
//////////////////////////////////////////////////////////////////////////////////////
/////명령을 받으면 수행 
void execute_com(char** commands) {
	int fork_process;
	int status;
	int i = 0;
	char* dir;
	///////////////////////////////////////////////
	/////////shell builtin command  
	if (!strcmp(commands[0], "cd")) {
		if (commands[1] == NULL) {
			printf("USEAGE : cd directory\n");
		}
		chdir(commands[1]);
	}
	else if (!strcmp(commands[0], "pwd")) {
		printf("Present Working Directory: %s\n", getcwd(dir, MAX));
	}
	///////////////////////////////////////////////
	///////////저장되어있는 명령어(/bin안에 존재)
	else {
		if ((fork_process = fork()) == 0) {
			//execv'P' 는 PATH에 등록된 모든 디렉토리에 있는 프로그램을 실행
				//명령어 끝까지 읽어서 각각 상황에 따라 실행
			while (commands[i]) {
				if (!strcmp(commands[i], ">")) {
					redirection_com(commands, i);
				}
				else if (!strcmp(commands[i], "&")) {
					background_com(commands, i);
				}
				i++;
			}
			//잘못된 commands[0]가 들어오면 -1 반환함
			if (execvp(commands[0], commands) == -1) {
				printf("%s : command not found\n", commands[0]);
				exit(1);
			}
		}
		else if (fork_process < 0) {
			printf("fork error\n");
			exit(1);
		}
		else
			wait(&status);
	}
}

//////////////////////////////////////////////////////////////////////
///////////명령어에 리다이렉션이 있을경우 수행
//ex) cat a.txt > b.txt
void redirection_com(char** commands, int i) {
	char* to = commands[i + 1];
	char* from = commands[i - 1];
	int to_fd;

	to_fd = open(to, O_WRONLY | O_CREAT, 0641);
	if (to_fd < 0) {
		printf("open error\n");
		exit(1);
	}
	//STDOUT_FILENO이 to_fd를 가리키게함
	dup2(to_fd, STDOUT_FILENO);
	//>자리를 NULL로
	commands[i] = NULL;
	//명령어 수행
	if (execvp(commands[0], commands) == -1) {
		printf("No such command : %s\n", commands[0]);
		exit(1);
	}
}
/////////////////////////////////////////////////////////////////////////
////////명령어에 &이 있을경우 background processing 수행
void background_com(char** commands, int i) {
	commands[i] = NULL;
	int fork_process;
	if ((fork_process = fork()) == 0) {
		if (execvp(commands[0], commands) == -1) {
			printf("No such command : %s\n", commands[0]);
			exit(1);
		}
	}
	else if (fork_process < 0) {
		printf("fork error\n");
		exit(1);
	}
	//기다리지 않음
	else {
		exit(1);
	}
};