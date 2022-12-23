#
#makefile
#

CC = gcc -g

all: my_shell

match: main.o mysh.o
	$(CC) -o match main.o mysh.o

myshell: myshell.c myshell.h
	$(CC) -o myshell.o -c myshell.c

main: main.c
	$(CC) -o main.o -c main.c

clean:
	rm my_shell *.o
