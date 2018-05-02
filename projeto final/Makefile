#Makefile server/client

CC= gcc
CFLAGS= -Wall

all: server #client

server: server.o
	gcc -pthread -o server server.o 
	rm -rf *.o

server.o: server.c
	gcc -c server.c 

#client.o: client.c
#	gcc -c client.c

clean:
	rm -rf *.o
	rm server
	#rm client