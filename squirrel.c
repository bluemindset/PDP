#include "actor.h"
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
static void squirrelStep(long *state, struct Squirrel *this)
{
    float diff1 = ran2(state);
    float diff2 = ran2(state);
    this->pos_x = (this->pos_x + diff1) - (int)(this->pos_x + diff1);
    this->pos_y = (this->pos_y + diff2) - (int)(this->pos_y + diff2);
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
static int willGiveBirth(float avg_pop, long *state, struct Squirrel *this)
{
    float probability = 100.0; // Decrease this to make more likely, increase less likely
    float tmp = avg_pop / probability;

    return (ran2(state) < (atan(tmp * tmp) / (4 * tmp)));
}

/**
 * Determines whether a squirrel will catch the disease or not based upon the average infection level
 * and a random seed which is modified. You can enclose this function call in an if statement if that is useful.
 */
static int willCatchDisease(float avg_inf_level, long *state, struct Squirrel *this)
{
    float probability = 1000.0; // Decrease this to make more likely, increase less likely
    return (ran2(state) < (atan(((avg_inf_level < 40000 ? avg_inf_level : 40000)) / probability) / M_PI));
}

/**
 * Determines if a squirrel will die or not. The state is used in the random number generation and
 * is modified. You can enclose this function call in an if statement if that is useful.
 */
static int willDie(long *state, struct Squirrel *this)
{
    return (ran2(state) < (0.166666666));
}

/**
 * Returns the id of the cell from its x and y coordinates.
 */
static int getCellFromPosition(float x, float y, struct Squirrel *this)
{
    return ((int)(x * 4) + 4 * (int)(y * 4));
}

static struct Squirrel new (double addr, int ID, int steps, int seed, float p_x, float p_y)
{
    struct Squirrel squirrel = {.steps = steps, .seed = seed, .pos_x = p_x, .pos_y = p_y,.update_avgs= &update_avgs, .squirrelStep = &squirrelStep, .willGiveBirth = &willGiveBirth, .willCatchDisease = &willCatchDisease, .willDie = &willDie};
    squirrel.actor = Actor.new(addr, ID);
    squirrel.health = 1;
    squirrel.avg_influx = 0;
    squirrel.avg_pop = 0;
    return squirrel;
}

const struct SquirrelClass Squirrel = {.new = &new};
