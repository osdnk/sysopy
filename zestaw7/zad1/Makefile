CC=gcc
CFLAGS=-Wall -std=gnu99

.PHONY: clean

all: clients barber

clients: clients.c
	$(CC) $< -o $@ $(CFLAGS)

barber: barber.c
	$(CC) $< -o $@ $(CFLAGS)

clean:
	rm {clients,barber} 2> /dev/null
