
/*************************LIBRARIES**********************************/
/********************************************************************/
#include "mpi.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
/********************************************************************/
/***************************ACTORS***********************************/
/********************************************************************/
#include "actor.h"
#include "cell.h"
#include "clock.h"
#include "squirrel.h"
#include "registry.h"
/*************************PROCESS************************************/
/********************************************************************/
#include "process_pool.h"
#include "master.h"
#include "worker.h"

#include "main.h"
#include "ran2.h"
/********************************************************************/
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void squirrelStep(float x, float y, float *x_new, float *y_new, long *state)
{

    float diff = ran2(state);
    *x_new = (x + diff) - (int)(x + diff);

    diff = ran2(state);
    *y_new = (y + diff) - (int)(y + diff);
}

static void update_avgs(int influx, int pop, struct Squirrel *this)
{
    
    if (this->steps == 50)
        this->steps = 0;

    this->influx[this->steps] = influx;
    this->pop[this->steps] = pop;
    this->steps++;
  //printf("Squirel Steps%d\n",this->steps);
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

int willGiveBirth(float avg_pop, long *state)
{
    float probability = 100.0; // Decrease this to make more likely, increase less likely
    float tmp = avg_pop / probability;

    return (ran2(state) < (atan(tmp * tmp) / (4 * tmp)));
}

int willCatchDisease(float avg_inf_level, long *state)
{
    float probability = 1000.0; // Decrease this to make more likely, increase less likely
    return (ran2(state) < (atan(((avg_inf_level < 40000 ? avg_inf_level : 40000)) / probability) / M_PI));
}

int willDie(long *state)
{
    return (ran2(state) < (0.166666666));
}

static struct Squirrel new (int rank, int ID, int steps, int seed, float p_x, float p_y)
{
    struct Squirrel squirrel = {.steps = steps, .seed = seed, .pos_x = p_x, .pos_y = p_y,.update_avgs= &update_avgs};
    squirrel.actor = Actor.new(rank, ID);
    squirrel.health = 1;
    squirrel.avg_influx = 0;
    squirrel.avg_pop = 0;
    squirrel.steps =0;
    return squirrel;
}

const struct SquirrelClass Squirrel = {.new = &new};