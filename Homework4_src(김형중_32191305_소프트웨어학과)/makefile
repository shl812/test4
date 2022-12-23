#
# makefile
#

CC = gcc -g

all: myshell looptest hello

myshell: main.o command.o inner_cmd.o str_process.o
	$(CC) -o myshell main.o command.o inner_cmd.o str_process.o

main.o: main.c
	$(CC) -o main.o -c main.c

command.o: command.c command.h
	$(CC) -o command.o -c command.c

inner_cmd.o: inner_cmd.c inner_cmd.h
	$(CC) -o inner_cmd.o -c inner_cmd.c

str_process.o: str_process.c str_process.h
	$(CC) -o str_process.o -c str_process.c

looptest: looptest.c
	$(CC) -o looptest looptest.c

hello: hello.c
	$(CC) -o hello hello.c

clean:
	rm -f myshell looptest hello  *.o
