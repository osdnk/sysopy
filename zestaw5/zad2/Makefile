GCC = gcc -Wall -std=c99 -ggdb
M_PATH = master
M_PATH2 = slave

FNAME = "FF"

build:
	$(GCC) -o $(M_PATH) master.c
	$(GCC) -o $(M_PATH2) slave.c

run_1:
	./$(M_PATH) $(FNAME)

run_2:
	./$(M_PATH2) $(FNAME) 10