SRC =  actor.c worker.c    process_pool.c squirrel.c cell.c registry.c main.c ran2.c  master.c  
CC=mpicc -lm -fopenmp




all: 
	$(CC) -o main $(SRC) $(LFLAGS)

clean:
	rm -f main
