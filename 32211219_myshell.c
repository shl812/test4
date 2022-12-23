/*
* @Author			:Taewon Kim.c
* @Student No	:32211219
* @Major		:Software
* @Date			:2022.12.9
*/
//#define DEBUG

#include <stdio.h>
#include<stdlib.h>
#include<memory.h>
#include<sys/types.h>
#include<unistd.h>
#include<stdbool.h>
#include<errno.h>
#include<fcntl.h>

#ifdef DEBUG
#define ONDEBUG(x) x
#else
#define ONDEBUG(x) /**/
#endif

static inline size_t getline_grow(size_t initial){
	return initial *2;
}
static const size_t getline_default=10;
char * mygetline(){
	size_t size=getline_default;
	size_t offset=0;
	char * buf=(char*)malloc(sizeof(char)*size);
	while(1){
		int input=getchar();

		if(input=='\n'){
			if(!(offset>0 && buf[offset-1]=='\\')) break; 
		}else if(input<0 || input=='\0'){
			break;
		}

		buf[offset++]=input;
		if(offset>=size){
			size=getline_grow(size);
			buf=realloc(buf,size);
		}
	}
	cleanup:
	buf=realloc(buf,offset+1);
	buf[offset]='\0';
	return buf;
}
// &과 &&이 다르니 그 점을 유의해야 한다.
char * mytokand2(char * str){
	char * prev=str;
	if(!prev) return NULL;
	str++;
	while(*str){
		if(*str=='&' &&*str==*prev){
			*prev='\0';
			return str+1;
		}
		str++;
		prev++;
	}
	return NULL;
}
// 여기서는 절대로 &&인데 &로 착각하면 안된다...
char * mytokand(char * str){
	char * prev=str;
	if(!str) return NULL;
	if(*str=='&' && *(str+1)=='\0'){
		*str='\0';
		return str+1;
	} 
	while(*str){
		if(*str=='&'){
			if(str[1]!='&'){
				*str='\0';
				return str+1;
			}else{
				str++;
			}
		}
		str++;
	}
	return NULL;
}
char * mytokpipe(char *str){
	char * prev=str;
	while(*str){
		if(*str=='|'){
			*str='\0';
			return str+1;
		}
		str++;
	}
	return NULL;
}
char * mytokredir(char *str,int * maybe){
	char * prev=str;
	if(*str=='>' || *str=='\0')return NULL;
	str++;
	while(*str){
		if(*str=='>'){
			*str='\0';
			if('0'<=str[-1]&& str[-1]<='9') if(maybe)*maybe=str[-1]-'0';
			return str+1;
		}
		str++;
	}
	return NULL;
}
char * mytokredir2(char * str,int * maybe){
	char * prev=str;
	if(!prev) return NULL;
	str++;
	while(*str){
		if(*str=='>' &&*str==*prev){
			*prev='\0';
			if('0'<=str[-1]&& str[-1]<='9') if(maybe)*maybe=str[-1]-'0';
			return str+1;
		}
		str++;
		prev++;
	}
	return NULL;
}

/*
bool strcmp(char * src,char * dst){
	while(*src++ && *dst++){
		if(*src!=*dst)return false;
	}
	
	return *(--src)==*(--dst);
}*/
static const size_t argsplit_default_size=5;
static inline size_t argsplit_grow(size_t size){
	return size *2;
}
char * lstrip(char * inp){
	while(*inp!='\0' && (*inp==' ' || *inp == '\t')){
		inp++;
	}
	return inp;
}
char ** split_args(char * input){
	char ** buf;
	size_t size=argsplit_default_size,offset=0;
	buf=(char**)malloc(sizeof(char**)*size);
	buf[offset++]=input;
	while(*input!='\0'){
		if(*input==' '){
			*input++='\0';
			while(*input==' '&& *input!='\0'){input++;}
			if(*input=='\0') break;
			buf[offset++]=input;
			if(offset>=size){
				size=argsplit_grow(size);
				buf=(char**)realloc(buf,sizeof(char**)*size);
			}
		}else{
			input++;
		}
	}
	buf=realloc(buf,sizeof(char**)*(offset+1));
	buf[offset]=NULL;
	return buf;
}
#define DUPLECATE_FD_BUFSIZE 64
bool duplecate_fd(int fd,int fd1){ 
	//fd에 쓰면 fd와 fd2에 동시에 쓴 효과가 난다.
	pid_t pid;
	int fdin[2],original;
	original=dup(fd);
  	if (pipe(fdin))return false;
	dup2(fdin[1],fd);
	pid=fork();
	if(pid<0) return false;
	if(pid==0){
		char buf[DUPLECATE_FD_BUFSIZE];
		ssize_t readsize;
		int in=fdin[0];
		close(fdin[1]);
      	while ((readsize = read(in,buf, DUPLECATE_FD_BUFSIZE))>0){
        	write(fd1,buf,readsize);
			write(original,buf,readsize);
      	}
		close(fd1);
		close(original);
		_exit(0);
		return true;
	}else{
		close(fdin[0]);
		return true;
	}
}
bool handle_special(char * input){
	ONDEBUG(printf("handle_special(%s)\n",input);)
	if(input[0]=='c'&&input[1]=='d'&&input[2]==' '){
		chdir(input+3);
		return true;
	}else if(input[0]=='e'&&
			 input[1]=='x'&&
			 input[2]=='i'&&
			 input[3]=='t'&&
			 input[4]=='\0')
	{
		exit(0);
		ONDEBUG(puts("exit not worked");)
		return true;
	}
	return false;
}

void handle_one(char * input){
	pid_t fork_return;
	int exit_status=-1;
	ONDEBUG(printf("handle_one(%s)\n",input);)
	if(handle_special(input)){
		return;	
	}{
		if(lstrip(input)=='\0')return;
	}
	fork_return=fork();
	if(fork_return==-1){
		ONDEBUG(puts("ERROR\n");)
		//handle error...
	}else if(fork_return==0){
		char ** argv=split_args(input);
		ONDEBUG(char ** temp;
		temp=argv;
		printf("forked with args:");
		while(*temp){
			printf("%s ",*temp);
			temp++;
		}
		printf("\n");)
		if(execvp(argv[0],argv)==-1){
			free(argv);
			printf("execvp went worng with errno=%d\n",errno);
			_exit(errno);
		}
		puts("this line won't be executed");
		free(argv);
		 //then is this not-freeing be okay?
	}else{
		fork_return=wait(fork_return,&exit_status,0);
		//deal with exit status
		ONDEBUG(printf("exit status was %d\n",exit_status);)
	}
}
void handle_pipe(char * input){
	char * res;
	res=mytokpipe(input);
	if(res==NULL){
		handle_one(input);
	}else{
		int pip[2];
		pid_t pid;
		pipe(pip);
		pid=fork();
		if(pid==0){
			close(pip[0]);
			dup2(pip[1],STDOUT_FILENO);
			handle_one(input);
			_exit(errno);
		}else{
			int exit_status;
			close(pip[1]);
			dup2(pip[0],STDIN_FILENO);
			handle_pipe(lstrip(res));
			pid=wait(pid,&exit_status,0);
		}
	}
}
void handle_redirectionto(char * prog, char * input,int maybe){
	char * res;
	int mymaybe=STDOUT_FILENO;
	//int maybe=STDOUT_FILENO;
	res=mytokredir(input,&mymaybe);
	int fd=open(lstrip(input),O_WRONLY|O_CREAT|O_TRUNC);
	ONDEBUG(printf("redirect %d> to %s\n",maybe,input);)
	//duplecate_fd(maybe,fd);
	dup2(fd,maybe); //a > b > c를 하면 b,c에 다 a가 들어가야 할 줄 알았다... 하지만 구름 ide의 shell 기준으로는 아닌 듯 하다.
	if(res==NULL){
		handle_pipe(prog);
	}else{
		handle_redirectionto(prog,lstrip(res),mymaybe);
	}
	close(fd);
}
void handle_redirection(char * input){
	char * res;
	int maybe=STDOUT_FILENO;
	res=mytokredir(input,&maybe);
	if(res==NULL){
		handle_pipe(input);
	}else{
		//int stdout=dup(STDOUT_FILENO);
		pid_t pid=fork();
		if(pid==0){
			handle_redirectionto(input,lstrip(res),maybe);
			_exit(errno);
		}//dup2(stdout,STDOUT_FILENO);
		else{
			int exit_status;
			ONDEBUG(printf("forked > process pid=%d",pid);)
			pid=wait(pid,&exit_status,0);
			ONDEBUG(printf("redirection process pid=%d,exit_status=%d",pid,exit_status);)
		}
	}
}
void handle_d_redirectionto(char * prog,char * input,int maybe){
	char * res;
	int mymaybe=STDOUT_FILENO;
	//int maybe=STDOUT_FILENO;
	res=mytokredir2(input,&mymaybe);
	int fd=open(lstrip(input),O_WRONLY|O_APPEND|O_CREAT);
	ONDEBUG(printf("redirect %d>> to %s\n",maybe,input);)
	//duplecate_fd(maybe,fd);
	dup2(fd,maybe);
	if(res==NULL){
		res=mytokredir(input,&mymaybe);
		if(res==NULL){
			handle_pipe(prog);
		}else{
			handle_redirectionto(prog,lstrip(res),mymaybe);
		}
	}else{
		handle_d_redirectionto(prog,lstrip(res),mymaybe);
	}
	close(fd);
}

void handle_d_redirection(char * input){
	int maybe=STDOUT_FILENO;
	char * res;
	res=mytokredir2(input,&maybe); //>>을 기준으로 분리. 1>나 2>도 처리 가능하게 함.
	if(res==NULL){
		handle_redirection(input);
	}else{
		pid_t pid=fork();
		if(pid==0){
			handle_d_redirectionto(input,lstrip(res),maybe);
			_exit(errno);
		}//dup2(stdout,STDOUT_FILENO);
		else{
			int exit_status;
			pid=wait(pid,&exit_status,0);
			ONDEBUG(printf("redirection process pid=%d,exit_status=%d",pid,exit_status);)
		}
	}
}

void handle_sync(char * input){
	ONDEBUG(printf("handle_sync(%s)\n",input);)
	if(*input=='\0')return;
	const int original=dup(STDIN_FILENO);
	char * res=mytokand2(input); //&&을 기준으로 분리
	while(res!=NULL){
		//pid_t fork_return;
		//int exit_status=-1;
		//fork_return=fork();
		//if(fork_return==-1){
		//	ONDEBUG(puts("ERROR\n");)
		//handle error...
		//}else if(fork_return==0){
			handle_d_redirection(lstrip(input));
		//	exit(0);
			 //then is this not-freeing be okay?
		//}else{
		//	fork_return=wait(fork_return,&exit_status,0);
			//deal with exit status
		//	ONDEBUG(printf("exit status for piper was %d\n",exit_status);)
		//}
		
		
		input=res;
		res=mytokand2(input);
	}
	handle_d_redirection(lstrip(input));
	dup2(original,STDIN_FILENO);
}
#define HANDLE_ASYNC_BUFSIZE 
void handle_async(char * input,unsigned int th){
	ONDEBUG(printf("handle_async(%s)\n",input);)
	//unsigned int th=1;
	char * res;
	pid_t pid;
	pid=fork();
	if(pid==0){
		//close(pip[0]);
		//dup2(pip[1],STDOUT_FILENO);
		printf("\n[%d] %d\n",th,getpid());
		handle_sync(input);
		printf("[%d] Done  %s\n",th,input);
		_exit(0);
	}
}
void handle(char * input){
	unsigned int th=1;
	char * res;
	input=lstrip(input);
	res=mytokand(input);
	while(res!=NULL){
		res=lstrip(res);
		//if(*res=='\0'){
		//	return;
		//}
		handle_async(input,th++);
		input=res;
		res=mytokand(input);
	}
	handle_sync(input);
}

int main(int argc, char* argv[]) {
	#ifdef DEBUG
	ONDEBUG({char vec[]="what is  this";
		char **v;
		char **tmp;
	v=tmp=split_args(vec);
	while(*v){
		printf("%s ",*v);
		v++;
	}
	free(tmp);
	})
	#endif
	while(true){
		char * inp;
		char * cwd;
		cwd=getcwd(NULL,0);
		printf("%s:%s$",getenv("USER"),cwd);
		free(cwd);
		inp = mygetline();
		//if(*inp=='\0') return 0;
		handle(inp);
		free(inp);
	}
	return 0;
}
