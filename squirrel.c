


#include "actor.h"
#include "main.h"
#include "squirrel.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ran2.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * Initialises the random number generator, call it once at the start of the program on each process. The input
 * value should be negative, non-zero and different on each process, I suggest something like -1-rank where
 * rank is the MPI rank.
 * ran2 is a flawed RNG and must be used with particular care in parallel however it is perfectly sufficient
 * for this course work
 * The seed is modified and then passed to each other subsequent function that relies on the random generator,
 * which also modify the seed
 */
static void initialiseRNG(long *seed)
{
    ran2(seed);
}

/**
 * Simulates the step of a squirrel. You can call this with the arguments (0,0,&x,&y,&state)
 * to determine a random initial starting point.
 * x_new and y_new are the new x and y coordinates, state is used in the random number
 * generator and is also modified.
 * x_new can point to x, and y_new can point to y
 */
void squirrelStep(float x, float y, float* x_new, float* y_new, long * state){

    float diff=ran2(state);
    *x_new=(x+diff)-(int)(x+diff);

    diff=ran2(state);
    *y_new=(y+diff)-(int)(y+diff);
}

static void update_avgs(int influx, int pop, struct Squirrel *this)
{

    if (this->steps == 50)
        this->steps = 0;
   
    this->influx[this->steps] = influx;
    this->pop[this->steps] = pop;

    (this->steps)++;
    int i;
    double avg_i = 0, avg_p = 0;

    for (i = 0; i < 50; i++)
    {
        avg_i += this->influx[i];
        avg_p += this->pop[i];
    }
    avg_i = avg_i / 50;
    avg_p = avg_p / 50;
    this->avg_influx = avg_i;
    this->avg_pop = avg_p;
}
/**
 * Determines whether a squirrel will give birth or not based upon the average population and a random seed
 * which is modified. You can enclose this function call in an if statement if that is useful.
 */
int willGiveBirth(float avg_pop, long * state) {
	float probability=100.0; // Decrease this to make more likely, increase less likely
    float tmp=avg_pop/probability;

    return (ran2(state)<(atan(tmp*tmp)/(4*tmp)));
}

/**
 * Determines whether a squirrel will catch the disease or not based upon the average infection level
 * and a random seed which is modified. You can enclose this function call in an if statement if that is useful.
 */
int willCatchDisease(float avg_inf_level, long * state) {
	float probability=1000.0; // Decrease this to make more likely, increase less likely
    return(ran2(state)<(atan(((avg_inf_level < 40000 ? avg_inf_level : 40000))/probability)/M_PI));
}


/**
 * Determines if a squirrel will die or not. The state is used in the random number generation and
 * is modified. You can enclose this function call in an if statement if that is useful.
 */
int willDie(long * state) {
    return(ran2(state)<(0.166666666));
}

/**
 * Returns the id of the cell from its x and y coordinates.
 */



/* Squirrel Routine 
      1. Move 
      2. Send Message to cell health status and wait until successfull
      3. Caclulate prop die 
      4. Caclulate prop born 
     */
  void squirrels_work(struct Squirrel * squirrel, int rank, _registry_cell *registry)
  {
    int die = 0;
    while (!die)
    {
      int seed = 0;
      int influx;
      int pop;

      /*If squirrel died then skip the rest and flag to 1*/
      if (!squirrel->health)
      {
        die = squirrel->willDie(seed, squirrel);
        continue;
      }

      /*Send health to the stepping cell*/
      int cell_ID = getCellFromPosition(squirrel->pos_x, squirrel->pos_y);
      access_registry(cell_ID, registry);
      send_msg_sq(registry[cell_ID].rank, _TAG_SQUIRRELS, MPI_INT, MPI_COMM_WORLD, &(squirrel->actor));

      MPI_Status s1;
      MPI_Status s2;
      MPI_Request r1;
      MPI_Request r2;

      /*Receive message from the cell*/
      MPI_IRecv(&influx, 1, MPI_INT, MPI_ANY_SOURCE, _TAG_SQUIRRELS, &s1, &r1);
      MPI_IRecv(&pop, 1, MPI_INT, MPI_ANY_SOURCE, _TAG_SQUIRRELS, &s2, &r2);

      /*Squirrel Moves*/
      (squirrel)->squirrelStep(&seed, (squirrel));

      MPI_Wait(&s1, &r1);
      MPI_Wait(&s2, &r2);

      squirrel->update_avgs(influx, pop, squirrel);
      squirrel->willCatchDisease(seed, squirrel);
      squirrel->willGiveBirth(seed, squirrel);
    }
  }