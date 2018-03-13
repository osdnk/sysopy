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
	make tests


tests:
	# static
	./test  30 11 static
	./test  620000 11 static  find 30000 remove
	./test  620000 33 static  change 20000 change 2000
	./test  600000 12 static  change_alt 123123 change 123123
	./test  620000 111 static  change 1000 remove
	
	# dynamic
	./test  30 11 dynamic
	./test  620000 11 dynamic  find 30000 remove
	./test  620000 33 dynamic  change 20000 change 2000
	./test  600000 12 dynamic  change_alt 123123 change 123123
	./test  620000 111 dynamic  change 1000 remove




