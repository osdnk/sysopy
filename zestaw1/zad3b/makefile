CC = gcc -Wall -std=c11 -ggdb


all: static shared clean

static:
	$(CC) -c library.c
	ar rcs library.a library.o

shared:
	$(CC) -c -fPIC library.c
	$(CC) -shared -fPIC -o library.so library.o

clean:
	rm -f *.o

zad3_static:
	make static
	$(CC) -o test main.c library.a -I . -O$(Olevel)
	# works :)
	make tests

zad3_dynamic:
	make shared
	$(CC) -o test main_dynamic.c -I . -O$(Olevel)
	# works :)
	make tests

zad3_shared:
	make shared
	$(CC) -o test main.c -L. library.so -I . -O$(Olevel)
	# works :)
Smolnik

tests:
	# static
	./test  301231 43 static
	./test  620000 51 static  find 5402300
	./test  620000 33 static  change 200000
	./test  600000 32 static  change_alt 123123
	
	# dynamic
	./test  301231 43 dynamic
	./test  620000 51 dynamic  find 5402300
	./test  620000 33 dynamic  change 200000
	./test  600000 32 dynamic  change_alt 123123




