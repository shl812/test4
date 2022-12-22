int  main(){
    while(1){
        char inputstring[MAX] = {0}; //입력받을 배열
        char path[MAX] = {0};  //현재 위치를 받을 배열
        char **input;  //입력받을 배열을 token화 해서 담을 포인터
        int read_size; 
        int errno;
        getcwd(path,MAX); //현재 위치를 가져와주는 함수 <unistd.h>

        write(STDOUT_FILENO, "HYUNMIN@",9); //HYUNMIN@출력
        write(STDOUT_FILENO, path, MAX); //현재 경로 출력
        write(STDOUT_FILENO, "# ", 3); //# 출력
        //HYUNMIN@workspace/SystemProgramming/shell# 출력

        read_size = read(STDIN_FILENO, inputstring, MAX); 
        //입력받은 문자열을 commandline에 넣기
        if(read_size == 1){
                continue;
        }
        input = tokenize(inputstring); //입력받은 문자열 token화
        if(!strcmp(input[0], "exit")){ //exit을 입력할 경우 shell 종료
                        printf("Bye bye!\n");
                        exit(1);
        }else{
                execute(input); //그게 아닐경우 명령어 실행
        }
    }
}
