#ifndef MAIN_H_
#define MAIN_H_
/********************************************************************/
/********************************************************************/
#define _DEBUG 0
#define MAX_REG 10
/**************************TAGS**************************************/
/********************************************************************/
#define _TAG_SQUIRRELS 6
#define _TAG_CELLS 2
#define _TAG_CLOCK 100000
#define _TAG_INITIAL 4
#define _TAG_REGISTRY_CELL 5
#define _MAX_SQUIRRELS 200
#define _TAG_STATS_UNHEALTHY 1
#define _TAG_STATS_HEALTHY 0
/**************************MONTHS**************************************/
/********************************************************************/
#define _MONTHS_INFLUX 2
#define _MONTHS_POP 3
#define  _MAX_MONTHS_SIMULATION 2
#define UNHEALTHY_SQUIRRELS 4
#define MONTH_IN_SEC 2
#define _NUM_CELLS 16
#define _NUM_INIT_SQUIRRELS 34

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

#endif