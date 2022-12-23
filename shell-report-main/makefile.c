mysh: mysh.o mysh_funcs.o
gcc - o mysh mysh.o mysh_funcs.o

mysh.o : mysh.h mysh.c
gcc - c mysh.c

mysh_funcs.o : mysh.h mysh_funcs.c
gcc - c mysh_funcs.c