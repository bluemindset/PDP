/**
 * @Author: B159973
 * @Date:	10/4/2019
 * @Course: Parallel Design Patterns - 2020
 * @University of Edinburgh
*/
#ifndef MAIN_H_
#define MAIN_H_

/********************************************************************/
/********************************************************************/
#define _DEBUG 0

/***************************************************************************//**
 * MPI TAGS
 ******************************************************************************/
#define _TAG_SQUIRRELS 6
#define _TAG_CELLS 2
#define _TAG_CLOCK 100000
#define _TAG_INITIAL 4
#define _TAG_REGISTRY_CELL 5
#define _TAG_STATS_UNHEALTHY 1
#define _TAG_STATS_HEALTHY 0

/***************************************************************************//**
 * Max Number of Squirrels in the simulation
 ******************************************************************************/
#define _MAX_SQUIRRELS 199

/***************************************************************************//**
 * Number of Months to look back that influx value in each cell is updated
 ******************************************************************************/
#define _MONTHS_INFLUX 3

/***************************************************************************//**
 * Number of Months to look back that population value in each cell is updated
 ******************************************************************************/
#define _MONTHS_POP 4

/***************************************************************************//**
 * Number of max months of the simualtion
 ******************************************************************************/
#define  _MAX_MONTHS_SIMULATION 24

/***************************************************************************//**
 * Number of unhealthy squirrels in start of the simualtion
 ******************************************************************************/
#define UNHEALTHY_SQUIRRELS 4

/***************************************************************************//**
 * Each month lasts these seconds
 ******************************************************************************/
#define MONTH_DURATION 2

/***************************************************************************//**
 * Initial Number of cells in the simulation
 ******************************************************************************/
#define _NUM_CELLS 16

/***************************************************************************//**
 * Initial Number of squirrels in the simulation
 ******************************************************************************/
#define _NUM_INIT_SQUIRRELS 34

/***************************************************************************//**
 * Number of last steps of each squirrel
 ******************************************************************************/
#define _STEPS 50

/***************************************************************************//**
 * Squirrel Laziness
 ******************************************************************************/
#define MAX_SQUIRRELS_LAZINESS 20

/***************************************************************************//**
 * Initialises the random number generator, call it once at the start of the program on each process. The input
 * value should be negative, non-zero and different on each process, I suggest something like -1-rank where
 * rank is the MPI rank.
 * ran2 is a flawed RNG and must be used with particular care in parallel however it is perfectly sufficient
 * for this course work
 * The seed is modified and then passed to each other subsequent function that relies on the random generator,
 * which also modify the seed
 ******************************************************************************/
static void initialiseRNG(long *seed);

#endif