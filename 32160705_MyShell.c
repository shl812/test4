#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>

int main(){ //variables
 char buf[1024];
 char *txt1=NULL;
 char *txt2=NULL;
 char *cmd1[4];
 char *cmd2[5];
 int i = 0;
 char *tmp;
 int fd[2];
 int fdr;
 pid_t pid;

 
printf("MyShell >> \n");

 while(1){
  
  for(; i < 1024 ; ++i){
   buf[i]='\0';
  }

  printf("$");

  fgets(buf,sizeof(buf),stdin);

  buf[strlen(buf)-1] ='\0';
  fflush( stdin );

  if(strchr(buf,'|')!=NULL){ //pipe
   txt1 = strtok (buf,"|");
   txt2 = strtok (NULL, "|");
   strcat(txt1,"\0");
   strcat(txt2, "\0");
   i=0;
   tmp = strtok (txt1," ");
   while (tmp != NULL && i<3)
   {
     cmd1[i]=tmp;
       tmp = strtok (NULL, " ");
    i++;
   }
   cmd1[i]=(char*)0;
   i=0;
   tmp = strtok (txt2," ");
   while (tmp != NULL && i<3)
   {
     cmd2[i]=tmp;
       tmp = strtok (NULL, " ");
    i++;
   }
   cmd2[i]=(char*)0;

   if(pipe(fd) == -1){
    printf("pipe call error\n");
    exit(1);
   }
   switch(fork()) //front command process 
   {
    case -1 : perror("fork error"); break;
    case 0 :
     if(close(1)==-1) perror("error occured");
     if(dup(fd[1]) != 0);
     if(close(fd[0]) == -1 || close(fd[1]) == -1){
       perror("error occured");
     }
     execvp(cmd1[0], cmd1);
     printf("No such command \n");       
     exit(0);  
   }
   switch(fork()) //rear command process
   {
    case -1 : perror ("fork error"); break;
    case 0 :
     if(close(0) == -1) perror("error occured");
     if(dup(fd[0]) != 0);
     if(close(fd[0]) == -1 || close(fd[1]) == -1) perror("error occured");
     execvp(cmd2[0], cmd2);
     printf("No such command \n");       
     exit(0);  
   }
 
   if(close(fd[0]) == -1 || close(fd[1]) == -1) perror("error occured");
   while(wait(NULL) != -1);
  }else if(strchr(buf,'>')!=NULL){
   txt1 = strtok (buf,">");
   txt2 = strtok (NULL, ">");
   i=0;
   tmp = strtok (txt1," ");
   while (tmp != NULL && i<3)
   {
     cmd1[i]=tmp;
       tmp = strtok (NULL, " ");
    i++;
   }
   cmd1[i]=(char*)0;
   i=0;
   tmp = strtok (txt2," ");
   while (tmp != NULL && i<3)
   {
     cmd2[i]=tmp;
       tmp = strtok (NULL, " ");
    i++;
   }
   cmd2[i]=(char*)0;
   
   switch(fork())
   {
    case -1 : perror ("fork"); break;
    case 0 :
     fdr = open(cmd2[0], O_WRONLY | O_CREAT | O_TRUNC, 0644);
     if(fdr==-1) {
      perror("file create error");exit(1);
     }
     if( dup2(fdr, 1) == -1){
      perror("dup error");
     }
     close(fdr);
     execvp(cmd1[0], cmd1);
     printf("No such command \n");    
     exit(0);        
      break;
    default : wait(NULL);
   }
  }else if(strchr(buf,'<')!=NULL){
   txt1 = strtok (buf,"<");
   txt2 = strtok (NULL, "<");
   i=0;
   tmp = strtok (txt1," ");
   while (tmp != NULL && i<3)
   {
     cmd1[i]=tmp;
       tmp = strtok (NULL, " ");
    i++;
   }
   cmd1[i]=(char*)0;
   i=0;
   tmp = strtok (txt2," ");
   while (tmp != NULL && i<3)
   {
     cmd2[i]=tmp;
       tmp = strtok (NULL, " ");
    i++;
   }
   cmd2[i]=(char*)0;
   switch(fork())
   {
    case -1 : perror ("fork"); break;
    case 0 :
     fdr = open(cmd1[0], O_WRONLY | O_CREAT | O_TRUNC, 0644);
     if(fdr==-1) {
      perror("file create error");exit(1);
     }
     if( dup2(fdr, 1) == -1){
      perror("dup error");
     }
     close(fdr);
     execvp(cmd2[0], cmd2);
     printf("No such command \n");       
     exit(0);        
      break;
    default : wait(NULL);
   }
  }else{      
   buf[strlen(buf)]='\0';
   i=0;
   tmp = strtok (buf," ");
   while (tmp != NULL && i<3)
   {
     cmd1[i]=tmp;
       tmp = strtok (NULL, " ");
    i++;
   }
   cmd1[i]=(char*)0;
   if(cmd1[0]!=NULL){   
    if(strcmp(cmd1[0],"exit")==0){
     printf("bye... \n");       
     exit(0);
    }else if(strcmp(cmd1[0],"cd")==0){
     chdir(cmd1[1]);
    }else if(fork()==0){
     execvp(cmd1[0], cmd1);
     printf("No such command \n");     
     exit(0);  
    }
    wait(NULL);
   }
   
  }
  
 }
 return 0;
}

