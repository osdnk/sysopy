all: static shared clean

static:
	gcc -c -Wall library.c
	ar rcs library.a library.o

shared:
	gcc -c -Wall -fPIC library.c
	gcc -shared -fPIC -o library.so library.o

clean:
	rm -f *.o