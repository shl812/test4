#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER_LEN 1024

int main() {
    char user_input[BUFFER_LEN];    		// 커맨드 라인 가져오기
    char* argv[120]; 						// 유저 커맨드
    int argc; 								// 인자 개수
    char* path = "/bin/";  					// bin에 path 설정
    char file_path[50]; 					// 최대 파일

    while(1) {
        printf("\n\n\n\n******************"
        "************************");
   		printf("\n\n\n\t     - Han SeungJae -");
    	printf("\n\n\n*******************"
        "***********************\n\n\n\n\n");
        printf("My Shell>> ");             
        
        if (!fgets(user_input, BUFFER_LEN, stdin)) {
            break;  						// 정의된 BUFFER_LEN 을 넘을 경우 break
        }
        
        size_t length = strlen(user_input);

        if (length == 0){
            break;
        }

        if (user_input[length - 1] == '\n') {
            user_input[length - 1] = '\0'; 	// 새 line의 char일 경우 마지막 char를 '\0'로 대체
        }
        
        char *token;                  
        token = strtok(user_input," ");
        int argc = 0;
        if (token == NULL) {
            continue;
        }
        while (token != NULL) {
            argv[argc] = token;      
            token = strtok(NULL, " ");
            argc++;
        }
        
        argv[argc] = NULL; 
        
        strcpy(file_path, path);  			// path를 file_path로 변경
        strcat(file_path, argv[0]); 		// command와 파일 path를 결합    

        if (access(file_path,F_OK) == 0) {  // /bin 내에서 커맨드가 유효한지 검사
        
            pid_t pid, wpid;
            int status;
            
            pid = fork();
            if (pid == 0) { 
                if (execvp(file_path,argv) == -1) {
                  perror("Child proccess end"); 
                }
                exit(EXIT_FAILURE);
            } 
            else if (pid > 0) { 
                wpid = waitpid(pid, &status, WUNTRACED); 
                while (!WIFEXITED(status) && !WIFSIGNALED(status)){
                    wpid = waitpid(pid, &status, WUNTRACED); 
                }
            } 
            else {
                perror("Fork Failed"); 		// process id는 NULL이 될 수 없음
            }
        }
        else {
            printf("Command is not available in the bin\n");
        }

    }
} 