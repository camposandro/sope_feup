# Makefile for simgrep

CC= gcc
CFLAGS= -Wall

all: simgrep

simgrep: main.o utils.o sighandlers.o
	gcc main.o utils.o sighandlers.o -o simgrep
	rm -rf *.o

main.o: main.c
	gcc -c main.c

utils.o: utils.c
	gcc -c utils.c

sighandlers.o: sighandlers.c
	gcc -c sighandlers.c

clean:
	rm -rf *.o simgrep

