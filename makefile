SRC =  actor.c process_pool.c squirrel.c main.c ran2.c cell.c arralloc.c
LFLAGS=-lm
CC=mpicc -std=c11




all: 
	$(CC) -o main $(SRC) $(LFLAGS)

clean:
	rm -f main
