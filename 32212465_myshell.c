/*
mysh.c: shell program
author: 안정우
email: anjoungwo@gmail.com
version: 1.0
student id: 32212465
date: 2022-12-08
*/



#include <signal.h> //signal 함수들을 이용하기 위한 헤더파일
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> //strtok, strcmp에 사용되는 헤더파일
#include <unistd.h> //리다이렉션의 dup2 함수를 사용하기 위한 헤더파일
#include <stdbool.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

pid_t child=-1; //자식프로세스의 pid
int status; //프로세스의 status(자식프로세스가 종료될때의 상태정보)를 저장
int list_CHILD[100]; //SIGTSTP 문자로 정지된 자식프로세스 목록
//SIGTSTP: 키보드에 의해 발생하는 시그널로 Ctrl+Z로 생성된다. 터미널 (프로세스)정지 문자
int count=0; 




        
bool sh_help( int argc, char* argv[] );
int sh_output_redirecton(char **cmd);
int sh_input_redirecton(char **cmd);
bool sh_cd( int argc, char* argv[] ); 
bool sh_exit( int argc, char* argv[] );       
bool sh_ps(int argc, char* argv[]);                       
bool sh_bg(int argc, char* argv[]);            
           

struct COMMAND{ //커맨드구조체. 커맨드 창에서 전달되는 함수명, 매개변수를 받아오는 구조체이다
   char* name;
   char* desc;
   bool(*func)(int argc,char* argv[]); 
//함수 포인터들이고, 사용할 함수들의 매개변수를 맞춰준다
};




struct COMMAND sh_commands[]={
   {"help","함수의 기능을 알아보는 함수",sh_help},
   {">","명령어의 결과를 파일로 출력함",sh_input_redirecton},
   {"<","file2의 내용이 file1에 출력됨",sh_output_redirecton},
   {"cd","현재 파일의 위치를 바꿈",sh_cd},
   {"exit","cmd창을 닫음",sh_exit},
   {"quit","quit this shell",sh_exit},
   {"ps","show process list",sh_ps},
   {"bg","show stop-status process list",sh_bg}
};



/*

파일 정보를 얻어오는 함수에서 사용하는 
		int lstat(const char *filename, struct stat * buf);
	
	
	여기에서 사용되는 구조체 stat의 구성 요소는 이러하다
	struct stat

{

dev st_dev;    /* device 

ino_t st_ino;    /* inode 

mode_t st_mode;    /* protection 

nlink_t st_nlink;    /* number of hard links 

uid_t st_uid;    /* user ID of owner 

gid_t st_gid;    /* group ID of owner 

dev_t st_rdev;    /* device type (if inode device) 

off_t st_size;    /* total size, in bytes 

unsigned long st_blksize;    /* blocksize for filesystem I/O 

unsigned long st_blocks;    /* number of blocks allocated 

time_t st_atime;    /* time of last access 

time_t st_mtime;    /* time of last modification 

time_t st_ctime;    /* time of last change 



모드 

설명 

 S_ISLNK(m)

심볼릭 링크 파일인지 확인합니다. 

 S_ISREG(m)

단순 파일인지 확인합니다. 

 S_ISDIR(m) 

디렉토리인지 확인합니다. 

 S_ISCHR(m) 

문자 디바이스인지 확인합니다. 

 S_ISBLK(m) 

블록 디바이스인지 확인합니다. 

 S_ISFIFO(m) 

FIFO파일인지 확인합니다. 선입선출 구조의 파일인지 확인하는 것이죠. 

 S_ISSOCK(m) 

네트워크 통신에 필요한 소켓파일인지 확인합니다. 



*/













//cd 메소드는 현재 파일의 위치를 바꿔준다
bool sh_cd(int argc,char* argv[]){
   if(argc==1){
      chdir(getenv("HOME")); //chdir은 현재 디렉토리를 변경한다
      //int chdir(const char *dirname); //dirname은 변경할 drietory
      //의 경로이다. 정상인 경우 반환값 0, 에러시 -1   
   }
   else if(argc==2){ //이건 왜 디렉토리 못찾겠다고 나오는 거지
      if(chdir(argv[1])){
         printf("No directory\n");
      }
   }
   else{
      printf("USAGE: cd dir\n");
   }
   
}

//cmd창을 닫습니다
bool sh_exit(int argc,char* argv[]){
	return false;
}



//함수의 기능을 알아보는 함수
bool sh_help(int argc,char* argv[]){
	int i;
   for(i=0; i<sizeof(sh_commands)/sizeof(struct COMMAND);i++){
	   if(argc==1||strcmp(sh_commands[i].name,argv[1])==0){
		   printf("%-10s: %s\n",sh_commands[i].name,sh_commands[i].desc);
	   }
   }
}




/*
ps 유틸리티는 프로세스에 대한 정보를 표시합니다. ps의 출력에는 다음 필드가 포함될 수 있습니다.

CGROUP
프로세스의 현재 1차 그룹 프로파일입니다.
CMD
가장 최근에 프로세스에서 실행된 프로그램, 메뉴 또는 명령입니다.
CUSER
프로세스의 현재 사용자 프로파일입니다.
DEVICE
프로세스와 연관된 장치 설명 오브젝트의 이름입니다.
ETIME
프로세스가 시작된 후 경과 시간입니다. 이 시간은 [[dd-]hh:]mm:ss 형식으로 표시됩니다. 여기서 dd는 일 수, hh는 시간, mm은 분, ss는 초입니다.
FUNCTION
가장 최근에 프로세스에서 실행된 프로그램, 메뉴 또는 명령입니다.
JOBID
프로세스의 규정된 작업명입니다. 규정된 작업명은 number/user/name 양식의 스트링입니다. number는 6자리 십진수이고, user는 작업이 시작된 사용자 프로파일이고, name은 작업명입니다.
JOBNAME
규정된 작업명의 작업명 구성요소입니다.
JOBNUM
규정된 작업명의 작업 번호 구성요소입니다.
NTHREADS
십진수로 표시되는 현재 프로세스에서 실행 중인 스레드 수입니다.
PCPU
최근 사용된 CPU 시간과 사용 가능한 CPU 시간의 비율을 백분율로 표시합니다.
PGID
십진수로 표시되는 프로세스 그룹 ID 번호입니다.
PID
십진수로 표시되는 프로세스 ID 번호입니다.
PPID
십진수로 표시되는 상위 ID 번호입니다.
PRI
십진수로 표시되는 프로세스의 현재 우선순위입니다. 숫자가 낮을수록 더 높은 우선순위를 나타냅니다.
SBS
프로세스가 실행 중인 서브시스템입니다.
STATUS
프로세스의 현재 상태입니다.
STIME*/
/*프로세스가 시작된 날짜 및 시간입니다. 디폴트로 날짜 및 시간은 mm-dd-yyyy hh:nn:ss 형식으로 표시됩니다. 여기서 mm은 월, dd는 일, yyyy는 연도, hh는 시간, nn은 분, ss는 초입니다. LC_TIME 환경 변수가 설정되면 지정된 로케일의 LC_TIME 범주에서 d_t_fmt 키워드로 지정된 형식을 사용하여 날짜 및 시간이 표시됩니다.
THCOUNT
10진수로 표시되는 현재 프로세스에서 실행 중인 스레드 수입니다.
TIME
프로세스에서 사용되는 CPU 시간(초)입니다. 이 시간은 [[dd-]hh:]mm:ss 형식으로 표시됩니다. 여기서 dd는 일 수, hh는 시간, mm은 분, ss는 초입니다.
TMPSZ
프로세스에서 사용되는 임시 기억장치의 양(MB)을 십진수로 표시합니다.
TYPE
프로세스의 유형입니다.
USER
규정된 작업명의 사용자 프로파일 구성요소입니다.
UID
규정된 작업명의 사용자 프로파일 구성요소에 해당하는 사용자 ID 번호입니다.
*/






//프로세스 목록 출력
bool sh_ps(int argc,char* argv[]){
	DIR *dir;
	FILE *Fsrc;
	char line[300];
	char temppath[300];
	struct dirent *entry;
	struct stat fileStat;
	int pid;	
	
	dir=opendir("\proc");
	Fsrc=fopen(temppath,"r");
	if(dir==NULL){
		return false;
	}
	else{
		
		while((entry=readdir(dir))!=NULL){ //process firectory 순차검색
		lstat(entry->d_name,&fileStat);
		if(!S_ISDIR(fileStat.st_mode)){
			continue;
		}
		pid=atoi(entry->d_name);
		if(pid<=0){
			continue;
		}
		sprintf(temppath,"/proc/%d/line",pid); //cmdline: 프로세스의 이름
		
		memset(line,'\0',sizeof(line));
		fgets(line,256,Fsrc);
		fclose(Fsrc);
		
		if(line[0]!='\0'){
			printf("[%d]\t%s\n",pid,line);
		}
	
}
			closedir(dir);
			return true;
	
	}
		
		}

/*	DIR *dir;
	FILE *Fsrc;
	char line[300];
	char temppath[300];*/





	//stup process 목록 출력
	//포어그라운드의 프로세스를 백그라운드로 전환합니다
	bool sh_bg(int argc, char*argv[]){  //stop process 목록 출력
                                                     //자세한내용은 http://sosal.tistory.com/115
        DIR *dir;
        struct dirent *entry;
        struct stat fileStat;
        int pid;
        char line[256];
        char temppath[256];
        FILE *Fsrc;
        char status[100];
        printf("[pid]   directory\n");
        dir = opendir("/proc");
		Fsrc = fopen(temppath,"r");
                
        if(dir == NULL)
                return false;
        while ((entry = readdir(dir)) != NULL) {
                lstat(entry->d_name, &fileStat);
                if (!S_ISDIR(fileStat.st_mode))
                        continue;
                pid = atoi(entry->d_name);
                if (pid <= 0)
                        continue;
                sprintf(temppath, "/proc/%d/status",pid);
                fgets(status,100,Fsrc);
                fgets(status,10,Fsrc);
                if(status[7] == 'T'){  //process 의 상태가 ‘T’ (stop) 이라면 출력
                        fclose(Fsrc);
                        sprintf(temppath, "/proc/%d/line", pid);
                        memset(line,'\0',sizeof(line));
                        fgets(line,256,Fsrc);
                        printf("[%d]\t%s &\n",pid,line);
                }
                fclose(Fsrc);
        }
        closedir(dir);
        return true;
}


 
	
/*
input 리다이렉션 구현. >입력 시 명령어의 결과를 파일로 출력해준다
dup2를 사용하여 구현한다
*/
	
	
	
int sh_input_redirecton(char **cmd){
	int fd;
	int a=0;
	int i;
	for(i=0;cmd[i]!=NULL;i++){
		if(!strcmp(cmd[i],">")){
			break;
		}
	}
	if(cmd[a]){
		if(!cmd[a+1]){
			return -1;
		}
		else{
			if((fd=open(cmd[a+1],O_RDWR|O_CREAT|S_IROTH,0644))==-1){
				perror(cmd[a+1]);
				return -1;
			}
		}
		
		dup2(fd,STDOUT_FILENO); //>를 입력받았을 시 fd로 출력
		close(fd);
		cmd[a]=NULL;
		cmd[a+1]=NULL;
		for(i=0;cmd[i]!=NULL;i++){
			cmd[i]=cmd[i+2];
		}
		cmd[i]=NULL;
	}
	return 0;
}
	
	
	
	
	
/*
output 리다이렉션 구현
file1<file2 입력 시 file2의 내용이 file1에 출력됨 
*/
	
	
	
int sh_output_redirecton(char **cmd){
	int fd;
	int a=0;
	int i;
	for(i=0;cmd[i]!=NULL;i++){
		if(!strcmp(cmd[i],"<")){
			break;
		}
	}
	if(cmd[a]){
		if(!cmd[a+1]){
			return -1;
		}
		else{
			if((fd=open(cmd[a+1],O_RDONLY))==-1){
				perror(cmd[a+1]);
				return -1;
			}
		}
		
		dup2(fd,STDOUT_FILENO); //>를 입력받았을 시 fd로 출력
		close(fd);
		for(i=0;cmd[i]!=NULL;i++){
			cmd[i]=cmd[i+2];
		}
		cmd[i]=NULL;
	}
	return 0;
}	
	
	
	
	
	
	



/*
명령어를 한 줄로 썼을 때
배열에 할당된 문장들을 함수명, 매개변수 등으로 나눠준 뒤
토큰의 개수를 리턴함
*/
	
	
int tokenize(char *buf,char* delims, char* tokens[], int maxTokens){
   int token_count=0;
   char* token;
   token=strtok(buf,delims);
   /*
   문자열을 token처럼 조각냄.
   char타입의 배열인 str인 buf를 첫 번째 매개변수로 받아서,
   delims를 기준으로 조각조각 나눈다
   */
   
   //토큰이 0이 될 때까지 읽어들인다
   while(token!=NULL && token_count<maxTokens){
      tokens[token_count]=token;
      token_count++;
      token=strtok(NULL,delims); //구분자 바로 뒤에 있는 char변수~구분자 까지 분리
   }
   tokens[token_count]=NULL;
   return token_count;
}
	
	

	
	
	

bool run(char* line){
   char tokens[128];
   int count;
   char delims[]="\r\n\t"; //구분자 배열 [\r, \n, \t]
   
	int i;
   for(i=0;i<strlen(line);i++){ //line의 문자열 하나씩 탐색
      if(line[i]=='&'){
         line[i]='\0'; //끊기
         break;
      }
   }
   
   count=tokenize(line,delims,tokens,sizeof(tokens)/sizeof(char*));
   
   if(count==0){
      return true;
   }
   
   for(i=0;i<sizeof(sh_commands)/sizeof(struct COMMAND);i++){
      if(strcmp(sh_commands[i].name,tokens[0])==0){
         //strcmp는 두 개의 문자열을 비교하여 같으면 0, 다르면 음수나 양수 반환
         return sh_commands[i].func(count,tokens);
      }
      
   }
   //handling internal command suck as cd,stty and exit
   //cd, stty 및 exit와 같은 내부 명령 처리
   if((child=fork())==0){
      execvp(tokens[0],tokens); 
      printf("No such file\n");
         _exit(0);
   }
   
   else if(child<0){
      printf("Failed to fork()!");
      _exit(0);
   }
   
   else {
      waitpid(child,&status,WUNTRACED);//중단된 자식 프로세스의 상태를 받음
   }
   return true;
}

	
	
	

	
	
	

int main(){
        char line[1024];
        while( 1 )
        {
            printf( "%s $ ", get_current_dir_name() );
            fgets( line, sizeof( line ) - 1, stdin );
            if( run( line ) == false ) break;
        }

}
