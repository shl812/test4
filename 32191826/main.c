
//이름 : 박주희
//학번 : 32191826
//날짜 : 2022.12.09


#include "myshell.h"

int main(){
    char buffer[256];
    int num_args = 0, i=0, num_arg=0;
    char* args[MAXARG];
    char* arg[MAXARG];
    char* s;
    char* saveptr;
    int pid, status;
    
    int num = 0;
    int j = 0;
    int m = 0;
    char input[20] = "";
    char output[20] = "";
    int is_bg = 0;
    static const char delim[] = " ";
    while (1) {
        printf("myshell$ ");
        gets(buffer);  //명령어 읽기
        
		//예외처리
        if (buffer != NULL && checkInput(buffer)==0) {
            num_args =0;
            if (strchr(buffer, '|') != NULL) {   //파이프라인
                num_args = tokenized(buffer, args, "|");
            } else {
                num_args = 1;
                args[0] = buffer;
                args[1] = (char *)0;
            }
            if (!strcmp(args[0], "quit")) {  //쉘 종료
                printf("Goodbye\n");
                break;
            }
            
            for (i = 0; i<num_args; i++) {  //인자 개수만큼 실행
                num = 0;
                m = 0;
                is_bg = 0;
                s = strtok_r(args[i], delim, &saveptr);  //공백 기준으로 자르기
                
				//읽으면서 m 변수로 구분
                while(s) {
                 	if (strcmp(s, "<") == 0) {
                        m = 2;
                    } else if (strcmp(s, ">") == 0) {
                            m = 3;
                    } else if(strcmp(s, "&") == 0) {
                        is_bg = 1;
                    } else {
                        if(m < 2)
                            arg[num++] = s;
                        else if(m == 2)
                            strcpy(input, s);
                        else
                            strcpy(output, s);
                    }
                    
                    s = strtok_r(NULL, delim, &saveptr);
                    
                }
                arg[num] = (char *)0;
                
                if (m == 0) {
                    execute(is_bg, arg); //execute 함수 실행
                }
				else if(m >= 2) {
                    redirect(m, is_bg, arg, input, output);  //redirect 함수 실행
                }       
            }
        }
    }
    exit(0);
}
