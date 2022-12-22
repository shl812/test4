#
#makefile
#all: mysh
#
mysh: shell.o shell_func.o
        gcc -o mysh shell.o shell_func.o

main.o: shell.c shell.h
        gcc -c shell.c

shell_func.o: shell_func.c shell.h
        gcc -c shell_func.c

clean:
        rm mysh *.o
