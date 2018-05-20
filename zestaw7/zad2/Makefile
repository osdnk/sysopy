CC=gcc
CFLAGS=-Wall -std=gnu99 -lpthread -lrt

.PHONY: clean

all: handler_of_clients barber

handler_of_clients: handler_of_clients.c
	$(CC) $< -o $@ $(CFLAGS)

barber: barber.c
	$(CC) $< -o $@ $(CFLAGS)

clean:
	rm {handler_of_clients,barber} 2> /dev/null
