#ifndef MAIN_H_
#define MAIN_H_
/********************************************************************/
/********************************************************************/
#define _DEBUG 1
#define MAX_REG 10
/**************************TAGS**************************************/
/********************************************************************/
#define _TAG_SQUIRRELS 6
#define _TAG_CELLS 2
#define _TAG_CLOCK 3
#define _TAG_INITIAL 4
#define _TAG_REGISTRY_CELL 5

/**************************DAYS**************************************/
/********************************************************************/
#define _DAYS_INFLUX 5
#define _DAYS_POP 6

/**
 * Initialises the random number generator, call it once at the start of the program on each process. The input
 * value should be negative, non-zero and different on each process, I suggest something like -1-rank where
 * rank is the MPI rank.
 * ran2 is a flawed RNG and must be used with particular care in parallel however it is perfectly sufficient
 * for this course work
 * The seed is modified and then passed to each other subsequent function that relies on the random generator,
 * which also modify the seed
 */
static void initialiseRNG(long *seed);
MPI_Request  squirrels_work(struct Squirrel * squirrel, int rank, struct Registry_cell *registry, int  data[2]);
void cells_work(struct Cell *cell, int rank, struct Registry_cell *registry);
#endif