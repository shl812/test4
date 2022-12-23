// 3222804 유주원 2022_12_09

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
	int fd[2];
	char buffer[30];
	pid_t pid;
	int state;
	
	if(pipe(fd)==-1){
		printf("Pipe Error"); // 파이프 에러
		exit(1);
	}
	
	pid=fork();  // fork() 함수 사용
	
	if(pid==-1){  //  pid 이 -1 일때
		printf("pipe error");  // 파이프 에러
		exit(1);
	}
	else if(pid==0){  // pid 이 0 일때
		write(fd[1],"Work\n",25);  // 진행
		sleep(2);  // 2 지연
		read(fd[0],buffer,30);  // 읽어드림
		printf("Output of chlid process : %s\n\n",buffer);  // chlid process
	}
	else{
		read(fd[0],buffer,30);  // 읽어들임
		printf("Output of parent process : %s\n\n",buffer);  //  parent process
		write(fd[1],"Finsh",25);	// 작성
		sleep(2);  //  2 지연
	}
	

	char ch;
 	while((ch=getchar()) != '\x80') // ctrl+z 입력 시 종료
 	{
 	putchar(ch);
 	}
 	return 0;
	
}

