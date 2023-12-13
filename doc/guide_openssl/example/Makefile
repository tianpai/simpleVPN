CC=gcc
CFLAGS=-Wall -Wextra -g
LDLIBS=-lssl -lcrypto

all: server client

server: server.c encrypt.o
	$(CC) $(CFLAGS) -o server server.c encrypt.o $(LDLIBS)

client: client.c encrypt.o
	$(CC) $(CFLAGS) -o client client.c encrypt.o $(LDLIBS)

encrypt.o: encrypt.c encrypt.h
	$(CC) $(CFLAGS) -c encrypt.c

clean:
	rm -f server client encrypt.o

