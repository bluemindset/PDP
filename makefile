SRC =  actor.c process_pool.c squirrel.c main.c 
LFLAGS=-lm
CC=mpicc


all: 
	$(CC) -o main $(SRC) $(LFLAGS)

clean:
	rm -f main
