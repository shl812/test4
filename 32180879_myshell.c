/*
 * myshell.c 
 * system programming 2022 12 01
 * 32180879 Kim won sub
 */ 

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<fcntl.h>


#define MAX_BUF 50
#define MAX_OP 10

void parsing(const char buf[],char (*op)[20]){
	
	char del[]=" "; //구분자는 띄어쓰기
	int i=1; 
	char *ptr = strtok(buf,del); 
		//printf("ptr = %s\n",ptr);
	strcpy(op[0],ptr);
		//printf("op[0] %s\n",op[0]);
	while(1){
		ptr=strtok(NULL,del);
			//printf("ptr = %s\n",ptr);
		if(ptr ==NULL){ break;}
		strcpy(op[i],ptr);
			//printf("op[%d] = %s\n",i,op[i]);
		i++;
	}
}

void command(char (*op)[20],char *buf){
	
	char full[MAX_BUF]="/bin/";
	pid_t pid; //명령 프로세스들을 위한 pid
	int fd,i;

	strcat(full,buf);


	if(op[0][0]=='.'){ //'.'현재 디렉토리 또는 '..'상위 디렉토리에서의 실행
		system(buf);
	}
	
	else if(strcmp(op[0],"ls")==0){
		system(full);

	}
	else if(strcmp(op[0],"pwd")==0){
		system("/bin/pwd");
	}
	else if(strcmp(op[0],"ps")==0){
		system("/bin/ps");
	} 
	else if(strcmp(op[0],"cd")==0){ //직접 구현
		
		if(op[1][0]=='~'){
			strcpy(op[1],"/home/");
			strcat(op[1],getlogin());
			if(strcmp("root",getlogin())==0){ //root 인 경우
				strcpy(op[1],"/");
			}
		}
		else if(op[1][0]==NULL){
			strcpy(op[1],"/home/");
			strcat(op[1],getlogin());
			if(strcmp("root",getlogin())==0){ //root 인 경우
				strcpy(op[1],"/");
			}
		
		}
		
	    chdir(op[1]);
	}
	else if(strcmp(op[0],"cat")==0){
		pid= fork();
		if(pid==0){
			execl("/bin/cat",op[0],op[1],NULL);
			exit(0);
		}
		else wait();
	}
	else if(strcmp(op[0],"more")==0){
		system(full);
	}
	else if(strcmp(op[0],"date")==0){
		pid=fork();
		if(pid==0){
			execl("/bin/date",op[0],NULL);
			exit(0);
		}
		else wait();
	}
	else if(strcmp(op[0],"clear")==0){
		pid=fork();
		if(pid==0){
			execl("/usr/bin/clear",op[0],NULL);
			exit(0);
		}
		else wait();
	}
	else if(strcmp(op[0],"mkdir")==0){
		system(full);
	}
	else if(strcmp(op[0],"rmdir")==0){
		system(full);
	}
	else if(strcmp(op[0],"cp")==0){
		system(full);
	}
	else if(strcmp(op[0],"tail")==0){
		strcpy(full,"/usr/bin/");
		strcat(full,buf);
		system(full);
	}
	else if(strcmp(op[0],"mv")==0){
		system(full);
	}
	else if(strcmp(op[0],"rm")==0){
		system(full);
	}
	else if(strcmp(op[0],"ln")==0){
		system(full);
	}
	else if(strcmp(op[0],"touch")==0){
		strcpy(full,"/usr/bin/");
		strcat(full,buf);
		system(full);
	}
	else if(strcmp(op[0],"vim")==0){
		strcpy(full,"/usr/bin/");
		strcat(full,buf);
		system(full);
	}
	else if(strcmp(op[0],"whoami")==0){//직접 구현
		printf("%s\n",getlogin());
	}
	else if(strcmp(op[0],"id")==0){
		strcpy(full,"/usr/bin/");
		strcat(full,buf);
		system(full);
	}
	else if(strcmp(op[0],"who")==0){
		strcpy(full,"/usr/bin/");
		strcat(full,buf);
		system(full);
	}
	else printf("wrong command\n");

}

int main(void){
	
	char buf[MAX_BUF]; // 입력 임시저장
	char com[MAX_BUF];	// 입력 전체를 저장 buf는 나중에 변경됨
	char op[MAX_OP][20]; // 명령어 입력을 parsing해서 저장

	char pwd[MAX_BUF]; // 현재 위치
	int bgr=0,redi=0; // 백그라운드인지 아닌지, 리다이렉션인지 아닌지 flag역할
	pid_t pid_b; // 백그라운드를 위한 pid
	
	int originOut,fd;
	originOut=dup(1);

	printf("============================================================\n");
	printf("\t\tSystem programming\n\t\t\tmy shell (by 32180879)\n");
	printf("============================================================\n");

	while(1){
		memset(buf,0x00,MAX_BUF);
		memset(com,0x00,MAX_BUF);
		memset(op,0x00,sizeof(op));
		
		dup2(STDOUT_FILENO,originOut);
		getcwd(pwd,MAX_BUF); //현재 위치 받아오기
		printf("%s/myshell@@ ",pwd);
		
		gets(buf); //명령어 입력

		printf("gets : %s\n",buf);
		strcpy(com,buf);
		
		parsing(buf,op); 
		
		if(strcmp(op[0],"exit")==0)	break; // myshell을 종료
		
		
		for(int i=0;i<=MAX_OP;i++){
			if(strcmp(op[i],"&")==0){ //& 백그라운드로 실행시키는지 확인
				bgr=1;
				for(int j =i;j<MAX_OP;j++){ //"&" 표시 제거
					if(op[j][0]==NULL)break;
					strcpy(op[j],op[j+1]);
				}
			}
			if(strcmp(op[i],">")==0){ //> 리다이렉션으로 실행시키는지 확인
				redi=1;
			}
		}
		
		
		
		if(redi==1){
			
			int i;
			for(i=0;i<MAX_OP;i++){
				if(strcmp(op[i],">")==0){
					i++;
					break;
				}
			}
			fd= open(op[i],O_RDWR|O_CREAT,0641);
			memset(op[i-1],0x00,sizeof(op[i-1]));
			memset(op[i],0x00,sizeof(op[i]));
			dup2(fd,STDOUT_FILENO);
			close(fd);
		}
		
		if(bgr==1){
			pid_b=fork();
			if(pid_b==0){
				
				command(op,com);
				exit(0);
			}
			else {
				waitpid(pid_b,0, WNOHANG); //WNOHANG인 경우 기다리지 않고 0 인 경우 wait과 같은 기능을 한다
				bgr=0;
			}
		}
		else{
			command(op,com);
		}
		
		if(redi==1){
			fflush(stdout); //buffer에 있는 값을 비운다.
			dup2(originOut,STDOUT_FILENO);
			redi=0;
		}
		
	}
	
	printf("finish\n");
	return 0;	
}

