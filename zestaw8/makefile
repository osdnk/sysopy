CC = gcc
CFLAGS = -g -O0 -std=c11 -static -Wall -Isrc -rdynamic -fomit-frame-pointer
all: compile test

compile:
	$(CC) $(CFLAGS) main.c -o main -lm -Wl,--whole-archive -lpthread -Wl,--no-whole-archive

test:
	# valgrind -v --track-origins=yes --keep-stacktraces=alloc-and-free
	./main 1 "./jablko.pgm" "./edge_detection.txt" "./jablko_1.pgm"
	./main 2 "./jablko.pgm" "./edge_detection.txt" "./jablko_2.pgm"
	./main 4 "./jablko.pgm" "./edge_detection.txt" "./jablko_4.pgm"
	./main 8 "./jablko.pgm" "./edge_detection.txt" "./jablko_8.pgm"

	./main 1 "./melon.pgm" "./edge_detection.txt" "./melon_1.pgm"
	./main 2 "./melon.pgm" "./edge_detection.txt" "./melon_2.pgm"
	./main 4 "./melon.pgm" "./edge_detection.txt" "./melon_4.pgm"
	./main 8 "./melon.pgm" "./edge_detection.txt" "./melon_8.pgm"

	./main 1 "./melon.pgm" "./elephant.txt" "./elephant_1.pgm"
	./main 2 "./melon.pgm" "./elephant.txt" "./elephant_2.pgm"
	./main 4 "./melon.pgm" "./elephant.txt" "./elephant_4.pgm"
	./main 8 "./melon.pgm" "./elephant.txt" "./elephant_8.pgm"
	
	

clean: $(patsubst %.c, %.clean, $(wildcard *.c))

%.clean:
	rm -f $(@:.clean=)
