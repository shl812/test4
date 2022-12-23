#!/bin/bash
clear
rm *.o
rm my_shell
gcc -c myshell.c -o myshell.o
gcc -c main.c -o main.o
gcc -o my_shell main.o myshell.o
