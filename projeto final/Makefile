#Makefile server-client

CC= gcc
CFLAGS= -Wall

all: server client

server: server.o utils.o
	gcc -pthread -o server server.o utils.o

client:	client.o utils.o
	gcc -o client client.o utils.o

server.o: server.c server.h
	gcc -c server.c

client.o: client.c client.h
	gcc -c client.c

utils.o: utils.c utils.h
	gcc -c utils.c

clean:
	rm -rf *.o *.gch *.txt
	rm server
	rm client