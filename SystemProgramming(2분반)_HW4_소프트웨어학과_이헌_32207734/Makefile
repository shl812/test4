CC = gcc
CFLAGS = -W -WALL
TARGET = myShell

$(TARGET) : displayPrompt.o getDataNum.o forkNexec.o myRedirection.o myPipe.o main.o
	$(CC) $(CFLAGS) -o $(TARGET) displayPrompt.o getDataNum.o forkNexec.o myRedirection.o myPipe.o main.o
	
displayPrompt.o : displayPrompt.c
	$(CC) $(CFLAGS) -c -o displayPrompt.o displayPrompt.c
	
getDataNum.o : getDataNum.c
	$(CC) $(CFLAGS) -c -o getDataNum.o getDataNum.c
	
forkNexec.o : forkNexec.c
	$(CC) $(CFLAGS) -c -o forkNexec.o forkNexec.c
	
myRedirection.o : myRedirection.c
	$(CC) $(CFLAGS) -c -o myRedirection.o myRedirection.c
	
myPipe.o : myPipe.c
	$(CC) $(CFLAGS) -c -o myPipe.o myPipe.c
	
main.o:  main.c
	$(CC) $(CFLAGS) -c -o main.o main.c
	
clean :
	rm  *.o myShell
