
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
#include "registry.h"
#include "actor.h"
#include "cell.h"
#include "clock.h"
#include "squirrel.h"
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

MPI_Request squirrels_work(struct Squirrel *squirrel, int rank, struct Registry_cell *registry, int data_recv[2], int *alive)
{
  float new_x, new_y;
  long seed = 1;
  if (_DEBUG)
    printf("[Worker] Squirrel is ready  %d\n", squirrel->actor.ID);

  /*If squirrel died then skip the rest and flag to 1*/
  /*Squirrel Moves*/
  squirrelStep(squirrel->pos_x, squirrel->pos_y, &new_x, &new_y, &seed);
  squirrel->pos_x = new_x;
  squirrel->pos_y = new_y;

  /*Send health to the stepping cell*/
  int cellID = getCellFromPosition(squirrel->pos_x, squirrel->pos_y);
  int cell_rank = 0;

  MPI_Request rs[2];
  /*Send the Cell ID to master to return the rank back*/
  MPI_Isend(&cellID, 1, MPI_INT, _MASTER, _TAG_REGISTRY_CELL, MPI_COMM_WORLD, &rs[0]);
  /*Receive the cell rank from the master*/
  MPI_Irecv(&cell_rank, 1, MPI_INT, _MASTER, _TAG_REGISTRY_CELL, MPI_COMM_WORLD, &rs[1]);

  int ready;
  /*Test for all the values to be received from the cell */
  MPI_Testall(2, rs, &ready, MPI_STATUSES_IGNORE);
  while (!ready && (*alive) != 0)
  {
    MPI_Testall(2, rs, &ready, MPI_STATUSES_IGNORE);
    if (should_terminate_worker(0) == 0)
    {
      *alive = 0;
      ready = 1;
    }
  }
  /*Create the package and send it*/
  int data[2];
  data[0] = squirrel->health;
  data[1] = cellID;
  int tag = cellID * _TAG_SQUIRRELS;

  MPI_Request r;
  int ready2;
  MPI_Isend(&data, 2, MPI_INT, cell_rank, tag, MPI_COMM_WORLD, &r);
  if (_DEBUG)
    printf("[Worker] Squirrel %d sending data to cell %d on rank %d with tag %d\n", squirrel->actor.ID, cellID, cell_rank, tag);

  if (_DEBUG)
    printf("[Worker] Squirrel %d issued receive %d on rank %d with tag %d\n", squirrel->actor.ID, cellID, cell_rank, tag);

  MPI_Test(&r, &ready2, MPI_STATUS_IGNORE);
  while (!ready2 && (*alive) != 0)
  {
    MPI_Test(&r, &ready2, MPI_STATUS_IGNORE);
    if (should_terminate_worker(0) == 0)
    {
      *alive = 0;
      ready2 = 1;
    }
  }

  MPI_Request rr;
  MPI_Irecv(data_recv, 2, MPI_INT, cell_rank, tag, MPI_COMM_WORLD, &rr);
  return rr;
}

int squirrel_life(struct Squirrel *squirrel, int influx, int pop, int *num_squirrels,int rank)
{
  long seed = 1;
  int newborn = 0;
  /*Update the averages of the its population and influx*/
  squirrel->update_avgs(influx, pop, squirrel);
  if (1)
    printf("[Worker] Squirrel %d has avg influx: %f and pop %f \n", squirrel->actor.ID, squirrel->avg_influx, squirrel->avg_pop);

  if (willCatchDisease(squirrel->avg_influx, &seed))
  {
    squirrel->health = 0;
  }

  if(willGiveBirth((float)squirrel->avg_pop,&seed)){
    (*num_squirrels)++;
    newborn = 1;
  }
  if (!squirrel->health)    
  {
    if (willDie(&seed))
      squirrel->health = -1;
  }
  return newborn;
}

void squirrelStep(float x, float y, float *x_new, float *y_new, long *state)
{

    float diff = ran2(state);
    *x_new = (x + diff) - (int)(x + diff);

    diff = ran2(state);
    *y_new = (y + diff) - (int)(y + diff);
}

static void update_avgs(int influx, int pop, struct Squirrel *this)
{
    int len;
    if (this->steps == 50){
        this->steps = 0;
        len = 50;
    }
    else {
        len = this->steps;
    }
    this->influx[this->steps] = influx;
    this->pop[this->steps] = pop;
    this->steps++;
    int i;
    double avg_i = 0, avg_p = 0;    

    for (i = 0; i < len; i++)
    {
        //printf("Squirel %d Influx %d ,pop %d steps %d\n",this->actor.ID,this->influx[i],this->pop[i],  this->steps);
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