/*
 * @shell.c		: make a shell 
 * @author		: Juyeol Yang
 * @studentID   : 32192539
 * @email      	: wnduf212@gmail.com
 * @version    	: 2.0
 * @date       	: 2022. 12. 09
 * @Requirement	:
 * 		1. Background Processing
 *   		1. Don't use wait()
 *		2. Redirection
 *   		1. STDIN / STDOUT을 사용하지 않고 execve()를 사용하기 전 dup2()를 사용한다.
 *		3. Pipe
 *	  		1. 두 개의 프로세스를 만들고 pipe통해 소통이 가능하게 해라
 *   		2. STDIN / STDOUT을 사용하지 않고 execve()를 사용하기 전에 fd[0] / fd[1]을 pipe()와 dup2()를 사용해라
 *		4. parsing logic
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#define MAX 256

void *prompt(char Buf[]);   // shell의 prompt역할을 한다.
int cmd_line(char Buf[]);   // 명령어를 수행한다. pipe, redirection이 들어오면 구분해서 처리한다
int cmd_pipe(int argc, char *argv[]);   // pipe 수행
int cmd_redirection(int argc, char *argv[], int type);  // redirection 수행

int main(){
    char Buf[MAX];
    char *argv[10];
    int status;
    int pid;
    
    while(prompt(Buf)){
        if(strcmp(Buf, "exit") == 0){
            printf("Good Bye\n");
            exit(0);
        }
        if((pid = fork()) < 0){ // child process 생성
            perror("fork error");
        }
        else if(pid == 0){
            cmd_line(Buf); // 명령어 전달
        }
        waitpid(pid, &status, 0);
    }
    return 0;
}
void *prompt(char Buf[]){
	void *ret;
	printf("myshell> ");
	ret = fgets(Buf, MAX, stdin);   // 입력

	if(Buf[strlen(Buf)-1] == '\n')
		Buf[strlen(Buf)-1] = 0;

	return ret;
}
int cmd_line(char Buf[]){
    int argc = 0;
    char *argv[10], *ptr;
    /*============================= pipe =============================*/
    if(strchr(Buf, '|') != NULL){
        // '|' 기준으로 나눔
        argv[argc] = strtok(Buf, "|");
        argc += 1;
        //pipe가 여러 개 있을 경우 더 나눠준다. 최대 4개까지 pipe처리 가능
        while((argv[argc] = strtok(NULL, "|")) != NULL){
            argc++;
        }
        // pipe 실행
        cmd_pipe(argc, argv);
    }
    /*============================= Redirection =============================*/
    /*
     * type rule
     * >> : 0
     *  > : 1
     * << : 2
     *  < : 3
     */
    //Output Redirection인 경우
    else if((ptr = strchr(Buf, '>')) != NULL){
        // '>' 기준으로 나눔
        argv[argc] = strtok(Buf, ">");
        argc += 1;
        while((argv[argc] = strtok(NULL, ">")) != NULL){
            argc++;
        }
        // Output Redirection 실행
        if(ptr[1] == '>'){
            // >>
            cmd_redirection(argc, argv, 0);
        }
        else{
            // >
            cmd_redirection(argc, argv, 1);
        }
    }
    // Input Redirection인 경우
    else if((ptr = strchr(Buf, '<')) != NULL){
        // '<' 기준으로 나눔
        argv[argc] = strtok(Buf, "<");
        argc += 1;
        while((argv[argc] = strtok(NULL, "<")) != NULL){
            argc++; 
        }
        // Input Redirection 실행
        if(ptr[1] == '<'){
            // <<
            cmd_redirection(argc, argv, 2);
        }
        else{
            // <
            cmd_redirection(argc, argv, 3);
        }
    }
    /*============================= normal command =============================*/
    else{
        // 빈칸 기준으로 나누기
        argv[argc] = strtok(Buf, " ");
        argc += 1;
        while((argv[argc] = strtok(NULL, " ")) != NULL){
            argc += 1;
        }

        if(argv[0] != NULL){    // 명령어가 들어온 경우
            // cd 명령어는 따로 처리해준다.
            if(strcmp(argv[0], "cd") == 0){
                if(chdir(argv[1]) < 0){
                    printf("cd: %s: No such file or directory\n", argv[1]);
                    exit(1);
                }
            }
            // cd명령어 이외의 명령어
            else{
                if(execvp(argv[0], argv) < 0){
                    printf("%s: command not found\n", argv[0]);
                    exit(1);
                }
            }
        }
        else{   // 명령어를 입력하지 않은 경우
            exit(0);
        }
    }
}

// pipe 명령어를 처리한다.
int cmd_pipe(int argc, char *argv[]){
    char *pipe1_argv[5], *pipe2_argv[5], *pipe3_argv[5], *pipe4_argv[5];
    int fd1[2], fd2[2], fd3[2];
    int pid1, pid2, pid3, pid4;
    int index, status;
    /*
     * pipe_argv: command와 argument값이 저장되어 있다. {command | arguement} |  {command | arguement}라고 하면
                  앞에 부분이 pipe1_argv, 뒤에가 pipe2_argv이다
     * fd: pipe 역할을 해주는 변수
     * pid: child process를 생성할 때마다 pid값을 저장한닫
     * index: parsing할 때 쓰이는 변수. 크게 의미있는 변수는 아니다
     * status: child process를 기다릴 때 쓰는 변수이다.
    */
    switch (argc)// pipe를 최대 3개까지 처리할 수 있기 때문에 2<= argc <= 4가 된다
    {
    case 4:
        if(pipe(fd3) < 0){// pipe 생성
            printf("pipe error: %d\n", errno);
            exit(1);
        }

        //argv를 parsing하여 pipe_argv에 저장한다
        index = 1;
        pipe4_argv[0] = strtok(argv[3], " ");
        while((pipe4_argv[index] = strtok(NULL, " ")) != NULL){
            index += 1;
        }
        pipe4_argv[index+1] = (char *)'\0';;
    case 3:
        if(pipe(fd2) < 0){// pipe 생성
            printf("pipe error: %d\n", errno);
            exit(1);
        }

        //argv를 parsing하여 pipe_argv에 저장한다
        index = 1;
        pipe3_argv[0] = strtok(argv[2], " ");
        while((pipe3_argv[index] = strtok(NULL, " ")) != NULL){
            index += 1;
        }
        pipe3_argv[index+1] = (char *)'\0';;
    case 2:
        if(pipe(fd1) < 0){// pipe 생성
            printf("pipe error: %d\n", errno);
            exit(1);
        }

        //argv를 parsing하여 pipe_argv에 저장한다
        index = 1;
        pipe2_argv[0] = strtok(argv[1], " ");
        while((pipe2_argv[index] = strtok(NULL, " ")) != NULL){
            index += 1;
        }
        pipe2_argv[index+1] = (char *)'\0';;
    default:

        //argv를 parsing하여 pipe_argv에 저장한다
        index = 1;
        pipe1_argv[0] = strtok(argv[0], " ");
        while((pipe1_argv[index] = strtok(NULL, " ")) != NULL){
            index += 1;
        }
        pipe1_argv[index+1] = (char *)'\0';
        break;
    }
    
    // "command argument |", 이렇게만 사용했을 때 뒤에 명령어 입력을 수행 
    if(argv[1] == NULL){
        char temp_Buf[MAX];
        printf("pipe> ");
        fgets(argv[1], MAX, stdin);
    }

    if(pid1 = fork() == 0){
        close(STDOUT_FILENO); close(fd1[0]);
        dup2(fd1[1], STDOUT_FILENO);
        close(fd1[1]);

        if(execvp(pipe1_argv[0], pipe1_argv) < 0){
            printf("%s: command not found\n", pipe1_argv[0]);
            exit(0);
        }
        exit(0);
    } else if(pid1 < 0){
        printf("pid1 fork error");
        exit(1);
    }

    if((pid2 = fork()) == 0){
        // pipe를 1개 사용했을 경우
        if(argc == 2){
            close(STDIN_FILENO); close(fd1[1]);
            dup2(fd1[0], STDIN_FILENO);
            close(fd1[0]);
        }
        // pipe를 1개 이상 사용했을 경우
        else{
            dup2(fd2[1], STDOUT_FILENO);
            dup2(fd1[0], STDIN_FILENO);
            
            close(fd2[0]);
            close(fd2[1]);
            // 이 아래 pipe를 close하지 않으면 자식 프로세스가 계속 input 받기를 대기하여
            // 프로그램이 먹통이 되어버린다
            close(fd1[1]); 
            close(fd1[0]);
        }
        if(execvp(pipe2_argv[0], pipe2_argv) < 0){
            printf("%s: command not found\n", pipe2_argv[0]);
            exit(0);
        }
        exit(0);
    } else if(pid2 < 0){
        printf("pid2 fork error");
        exit(1);
    }

    if((pid3 = fork()) == 0){
        // pipe를 2개 미만으로 사용했을 경우
        if(argc < 3){
            exit(0);
        }
        // pipe를 2개 사용했을 경우
        if(argc == 3){
            dup2(fd2[0], STDIN_FILENO);
            close(fd1[0]);
            close(fd1[1]);
            close(fd2[1]);
            close(fd2[0]);
        }
        // pipe를 2개 이상 사용했을 경우
        else{
            dup2(fd2[0], STDIN_FILENO);
            dup2(fd3[1], STDOUT_FILENO);

            close(fd3[0]);
            close(fd3[1]);
            close(fd2[1]);
            close(fd2[0]);
            close(fd1[1]); 
            close(fd1[0]);
        }

        if(execvp(pipe3_argv[0], pipe3_argv) < 0){
            printf("%s: command not found\n", pipe3_argv[0]);
            exit(0);
        }
        exit(0);
    }else if(pid3 < 0){
        printf("pid3 fork error");
        exit(1);
    }
    if((pid4 = fork()) == 0){
        // pipe를 3개 미만으로 사용했을 경우
        if(argc < 4){
            exit(0);
        }
        // pipe를 3개 사용했을 경우
        if(argc == 4){
            dup2(fd3[0], STDIN_FILENO);
            close(fd3[1]);
            close(fd3[0]);
            // 
            close(fd1[0]);
            close(fd1[1]);
            close(fd2[0]);
            close(fd2[1]);
        }
        if(execvp(pipe4_argv[0], pipe4_argv) < 0){
            printf("%s: command not found\n", pipe4_argv[0]);
            exit(0);
        }
        exit(0);
    } else if(pid4 < 0){
        printf("pid4 fork error");
        exit(1);
    }
    close(fd3[0]); close(fd3[1]);
    close(fd2[0]); close(fd2[1]);
    close(fd1[0]); close(fd1[1]);
    waitpid(pid1, &status, 0); waitpid(pid2, &status, 0);
    waitpid(pid3, &status, 0); waitpid(pid4, &status, 0);
    exit(0);
}

int cmd_redirection(int argc, char *argv[], int type){
    /*
     * type rule
     * >> : 0
     *  > : 1
     * << : 2
     *  < : 3
     */
    int fd, read_size, write_size, pid;
    char *parent_argv[5], *child_argv[5];
    int index;
    /*
     * fd: fild을 오픈했을 때 fd값
     * read_size: 파일을 읽은 byte 저장
     * write_size: 파일을 쓴 byte 저장
     * parent_argv: parent process에서 사용할 argv
     * child_argv: child process에서 사용할 argv 
     */
    
    // 빈 칸 기준으로 parsing 진행
    index = 1;
    parent_argv[0] = strtok(argv[0], " ");
    while((parent_argv[index] = strtok(NULL, " ")) != NULL){
        index += 1;
    }

    index = 1;
    child_argv[0] = strtok(argv[1], " ");
    while((child_argv[index] = strtok(NULL, " ")) != NULL){
        index += 1;
    }

    if(type < 2){// >>, >
        if((pid = fork()) == 0){
            switch (type)
            {
            case 0:// >>
                fd = open(child_argv[0], O_WRONLY | O_CREAT | O_APPEND, 0644);
                break;
            case 1: // >
                fd = open(child_argv[0], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                break;
            default:
                break;
            }
            //file open 오류 처리
            if(fd < 0){
                perror("file open error\n");
                exit(1);
            }

            // 출력 위치 변경
            dup2(fd, STDOUT_FILENO);
            close(fd);

            if(execvp(parent_argv[0], parent_argv) < 0){
                perror("execvp error\n");
                exit(1);
            }
        }
        waitpid(pid, NULL, 0);
        exit(0);
    }
    else{// <<, <
        if(type == 2){
            char temp_file[10] = ".saveFile";
            // temp_file에 문자열이 바뀌어도 아래를 수정하지 않게 1번에는 NULL값을 넣었다
            char *remove_temp_file[3] =  {"rm", NULL, NULL}; 
            char cmd[MAX];
            char tag[MAX];
            /*
             * temp_file: tag입력 전까지 입력받은 문자열들을 저장하는 파일
             * remove_temp_file: 임시 저장 파일을 삭제하는 command와 argument가 담겨져 있다
             * cmd: tag입력 전까지 받을 문자열을 cmd에 저장
             * tag: tag를 저장
             */
            remove_temp_file[1] = temp_file; // 삭제할 파일 입력
            
            //tag를 저장
            strcpy(tag, child_argv[0]);
            strcat(tag, "\n");// 개행문자를 포함해야 strcmp시 버그가 생기지 않는다

            if((pid = fork()) == 0){
                fd = open(temp_file,  O_WRONLY | O_CREAT | O_EXCL | O_TRUNC, 0644);
                if(fd < 0){
                    perror("file open error\n");
                    exit(1);
                }
                while(1){ // tag를 입력받을 때까지 반복 수행
                    printf("heredoc>");
                    fgets(cmd, MAX, stdin);
                    if(strcmp(cmd, tag) == 0) break;
                    write_size = write(fd, cmd, strlen(cmd));
                }

                close(fd); // 쓰기 종료

                //명령어에 입력하기 위해 방금 저장했던 파일 open
                fd = open(temp_file, O_RDONLY);
                dup2(fd, STDIN_FILENO);
                close(fd);

                if(execvp(parent_argv[0], parent_argv) < 0){
                    perror("execvp error\n");
                    exit(1);
                }
                exit(0);
            }
            waitpid(pid, NULL, 0);
            // 임시 저장 파일 삭제
            execvp(remove_temp_file[0], remove_temp_file);
        }
        // <
        else if(type == 3){
            if((pid = fork()) == 0){
                // 입력받을 파일 open
                fd = open(child_argv[0], O_RDONLY);

                if(fd < 0){
                    printf("file open error: %d\n", errno);
                    exit(1);
                }

                dup2(fd, STDIN_FILENO);
                close(fd);

                if(execvp(parent_argv[0], parent_argv) < 0){
                    printf("execvp error: %d\n", errno);
                    exit(1);
                }
                exit(0);
            }
            waitpid(pid, NULL, 0);
            exit(0);
        }
        else{
            perror("not redirection\n");
        }
    }
}