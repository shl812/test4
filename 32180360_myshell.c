//32180360 kim kyung tae

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX 512

void help_print(){//help를 입력했을 때 어떤 기능이 있는지 알려준다
	printf("==========================================\n");
	printf("%-5s : Redirection (in)\n", "<");
	printf("%-5s : Redirection (out)\n", ">");
	printf("%-5s : Redirection (out + append)\n", ">>");
	printf("%-5s : Background\n", "&");
	printf("==========================================\n");
}

int parsing(char* strings, char*args[]){// 파싱을 하는 과정을 보여준다.
	int counter = 0;
	char* arg = strtok(strings, " \n");
	while (arg != NULL){
		args[counter++] = arg;
		arg = strtok(NULL, " \n");
	}
	args[counter] = NULL;
	return counter;
}

int execution(char* strings){// 실행을 위한 작업이다.
	char* argv[MAX];
	char* args[MAX];
	int i, status,fd = -1;
	int flag, number = 0;
	int index;
	pid_t pid;
	int counter = parsing(strings, args);
	if (counter == 0) return 0;

	for (index = 0; index < counter; index++){// redirection이 있는지 확인하는 절차다.
		if (strcmp(args[index], ">") == 0){
			flag = O_WRONLY | O_CREAT | O_TRUNC;
			number=1;
			break;
		}
		if (strcmp(args[index], ">>") == 0){
			flag = O_WRONLY | O_APPEND;
			number=2;
			break;
		}
		if (strcmp(args[index], "<") == 0){
			flag = O_RDWR;
			number=3;
			break;
		}
	}
	if (strcmp(args[0], "help") == 0){//help 라는 단어가 있으면 help_print()를 호출한다
		help_print();
		return 0;	
	}
	else if (strcmp(args[0], "cd") == 0){//디렉토리 이동하는 경우 입력값이 없으면 root로 있으면 그 장소로 간다
		if (counter == 1)
			chdir(getenv("HOME"));
		else if (counter == 2)
		{
			if (chdir(args[1]) != 0){
				printf("Directory is not exist.\n");
			}
		}
		return 0;
	}
	else if ((strcmp(args[0], "exit") == 0))//exit을 입력하면 종료한다
			return 1;
	else{
		pid = fork();

		if (pid == -1){
			perror("fork");
			return 0;
		}
		else if (pid == 0){
			for (i = 0; i < index; i++){
				argv[i] = args[i];
			}
			if (number ==1){
				if ((fd = open(args[index + 1], flag, 0644)) == -1){//redirection > 경우 
					perror("open");
					return 0;
				}
				if (dup2(fd, STDOUT_FILENO) == -1){
					perror("dup2");
					return 0;
				}
				execvp(argv[0], argv);
			}
			else if(number==2){
				if ((fd = open(args[index + 1], flag, 0644)) == -1)// redirection >> 경우
				{
					perror("open");
					return 0;
				}
				if (dup2(fd, STDOUT_FILENO) == -1){
					perror("dup2");
					return 0;
				}
				execvp(argv[0], argv);
			}
			else if(number==3){
				if ((fd = open(args[index + 1], flag)) == -1)// redirection < 경우
				{
					perror("open");
					return 0;
				}
				if (dup2(fd,0) == -1){
					perror("dup2");
					return 0;
				}
				execvp(argv[0], argv);
			}
			execvp(args[0], args);
			printf("%s: Can't execution\n", args[0]);
			return 0;
		}
		else if (strcmp(args[counter - 1], "&"))// 백그라운드를 실행하는 경우 부모 프로세스는 자식 프로세스를 기다리지 않는다
			waitpid(pid, &status, 0);
		return 0;
	}
}

int main(){
	char line[MAX];
	while (1)
	{
		printf("[%s]$",get_current_dir_name());
		fgets(line, MAX-1,stdin);
		if (strcmp(line, "\n") == 0) continue;
		if (execution(line)) break;
	}
	return 0;
}
