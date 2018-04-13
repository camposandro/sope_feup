#Makefile for simgrep

CC= gcc
CFLAGS= -Wall

all: simgrep

simgrep: simgrep.o
	gcc simgrep.o -o simgrep
	rm -rf *.o

simgrep.o: simgrep.c
	gcc -c simgrep.c
