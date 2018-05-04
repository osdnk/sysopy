CC=gcc
CFLAGS=-Wall -std=gnu99 -static -Wall -Wextra -Isrc -rdynamic

.PHONY: clean

all: client server

client: client.c
	$(CC) $< -o $@ $(CFLAGS)

server: server.c
	$(CC) $< -o $@ $(CFLAGS)

clean:
	 rm {client,server} 2> /dev/null
