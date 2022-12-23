/**
* main.c : main program to run mini shell
* author : Jiyun Song
* @email : 32202341@dankook.ac.kr
* @version : 1.0
* date : 2022.12.1
**/

#include "includes.h" 

// shell 실행 함수
int main(){
    char buffer[256]; // 크기가 256인 버퍼 선언
	char input[20] = "";
    char output[20] = "";
	
    char* args[MAX_NUM];
    char* arg[MAX_NUM];
	char *pipe_arg1[MAX_NUM];
    char *pipe_arg2[MAX_NUM];
	
    char* s;
    char* saveptr;
    
    int num = 0;
    int flag = 0;
    int brpoint = 0;
    int isbg = 0;
	int num_args = 0, i = 0, j = 0; 
	
    static const char delim[] = " \t\n"; // delimiter 선언
	
    while (1) { // 명령어 입력 받는 동작 반복
        printf("$ ");
        fgets(buffer, 100, stdin); // 최대 길이 1024의 명령어를 입력 받아 버퍼에 저장
        
        if (buffer != NULL && checkCommand(buffer)==0) {
            num_args =0;
            if (strchr(buffer, ';') != NULL) {
                num_args = tokenized(buffer, args, ";");
            } else {
                num_args = 1;
                args[0] = buffer;
                args[1] = (char *)0;
            }
            if (strncmp(buffer, "quit", 4)==0) { 
				printf("Terminate mini shell::\n");
				exit(0);
            }
            
            for (i = 0; i<num_args; i++) {
                num = 0;
                brpoint = 0;
                flag = 0;
                isbg = 0;
                s = strtok_r(args[i], delim, &saveptr);
                
                while(s) {
                    if (strcmp(s, "|") == 0) {
                        flag = 1;
                        brpoint = num;
                        for (j =0; j < brpoint; j++) {
                            pipe_arg1[j] = arg[j];
                        }
                        pipe_arg1[brpoint] = (char *) 0;
                    } else if (strcmp(s, "<") == 0) {
                        flag = 2;
                        brpoint = num;
                    } else if (strcmp(s, ">") == 0) {
                        if (flag == 2)
                            flag = 4;
                        else
                            flag = 3;
                    } else if(strcmp(s, "&") == 0) {
                        isbg = 1;
                    } else {
                        if(flag < 2)
                            arg[num++] = s;
                        else if(flag ==2)
                            strcpy(input, s);
                        else
                            strcpy(output, s);
                    }
                    
                    s = strtok_r(NULL, delim, &saveptr);
                    
                }
                arg[num] = (char *)0;
                
                if (flag == 0) {
                    executesLine(isbg, arg);
                }
                else if (flag == 1) {
                    for (j =0; j < num - brpoint; j++) {
                        pipe_arg2[j]= arg[j+brpoint];
                    }
                    pipe_arg2[num-brpoint] = (char *) 0;
                    doPipe(pipe_arg1, pipe_arg2);
                } else if(flag >= 2) {
                    doRedirection(flag, isbg, arg, input, output);
                }       
            }
        }
    }
    exit(0);
}
