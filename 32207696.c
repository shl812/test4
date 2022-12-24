/**
* mycp.c    : copy command program
* @author   : Joungwoo Lee
* @email    : xw21yog@dankook.ac.kr
* @version : 1.0
* @date     : 2022. 11. 04
**/

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#define MAX_BUF 64
 
int main(int argc, char *argv[]){
    char buf[MAX_BUF];
    int fd, fd2, read_size , write_size ;
	struct stat stt;
	   
        if(argc != 3){
            printf("mycp 실행 오류입니다.\n");
			exit(1);
        }
	
    fd = open(argv[1],O_RDONLY);
    fstat(fd, &stt);
    fd2 = open(argv[2], O_RDWR | O_CREAT | O_EXCL, stt.st_mode);
 
	    
        if(fd<0 || fd2<0 ){
			printf("Can't create file with errno %d\n", errno);
			exit(1);
		}
	read_size=read(fd, buf, MAX_BUF);
	    if(read_size<0){
			printf("파일을 읽을 수 없습니다.\n");
			exit(1);
		}
	write_size=write(fd2, buf, read_size);
	    if(write_size<0){
			printf("파일을 작성할 수 없습니다.\n");
			exit(1);
		}
            
 
        while(1)
				{
			    read_size = read(fd, buf, MAX_BUF);
				 if(read_size==0)
					break;
                 write_size= write(STDOUT_FILENO, buf ,read_size);
                }
    printf("복사 완료!!\n");
    close(fd);
    close(fd2);
	return 0;
    exit(0);
	
}