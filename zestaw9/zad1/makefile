CC = gcc
CFLAGS = -g -O2 -std=gnu99 -Wall -lpthread

all: $(patsubst %.c, %.out, $(wildcard *.c))

%.out: %.c Makefile
	$(CC) $(CFLAGS) $< -o $(@:.out=)

clean: $(patsubst %.c, %.clean, $(wildcard *.c))

%.clean:
	rm -f $(@:.clean=)
