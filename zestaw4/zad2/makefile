CC = gcc
FLAGS = -std=gnu99 -Wall
MAIN_SRC = main
OPTIM ?= -O2

all:
	$(CC) $(FLAGS) $(MAIN_SRC).c -o $(MAIN_SRC)
	$(CC) $(FLAGS) child.c -o child

clean:
	rm $(MAIN_SRC)
	rm child