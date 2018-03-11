all: static shared clean

static:
	gcc -c -Wall library.c
	ar rcs library.a library.o

shared:
	gcc -c -Wall -fPIC library.c
	gcc -shared -fPIC -o library.so library.o

clean:
	rm -f *.o

zad3_static:
	make static
	gcc -o test1 main.c library.a -std=c11 -I . -Wall -ggdb
	# works :)
	./test1  620000 11 static  find 30000 remove
	./test1  620000 33 static  change 20000 change 2000
	./test1  600000 12 static  change_alt 123123 change 123123
	./test1  620000 111 static  change 1000 remove

zad3_dynamic:
	make shared
	gcc -o test2 main_dynamic.c -std=c11 -I . -Wall -ggdb
	# works :)
	./test2  620000 11 static  find 30000 remove
	./test2  620000 33 static  change 20000 change 2000
	./test2  600000 12 static  change_alt 123123 change 123123
	./test2  620000 111 static  change 1000 remove

zad3_shared:
	make shared
	gcc -o test3 main.c -L. library.so -std=c11 -I . -Wall -ggdb
	# works :)
	./test3  620000 11 static  find 30000 remove
	./test3  620000 33 static  change 20000 change 2000
	./test3  600000 12 static  change_alt 123123 change 123123
	./test3  620000 111 static  change 1000 remove



