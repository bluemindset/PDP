
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

// static int getCellIDFromPosition(float x, float y,struct Cell *this){
//     //return((int)(x*4)+4*(int)(y*4));
//     return this->actor.ID;  .getCellIDFromPosition=&getCellIDFromPosition
// }

static struct Cell new (int rank, int ID, float pos_x, float pos_y)
{
  struct Cell cell = {.pos_x = pos_x, .pos_y = pos_y};
  cell.actor = Actor.new(rank, ID);
  cell.influx = 0;
  cell.pop = 0;
  
  //cell.month_stats = (struct month_stat *)malloc(sizeof(struct month_stat));
  //cell.squirrels_healthy = (int *)calloc( _MAX_monthS_SIMULATION,sizeof(int));
  //cell.squirrels_unhealthy = (int *)calloc( _MAX_monthS_SIMULATION,sizeof(int));
 // cell.month_stats->nextstat = NULL;

  return cell;
}

const struct CellClass Cell = {.new = &new};
/**
 * Returns the id of the cell from its x and y coordinates.
 */

void update_cell(struct Cell *cell, int month, int rank,int * stats,int cellID,int num_cells)
{
  int i, sum = 0;

  

  /*Thresholds*/
  int passinflux = _MONTHS_INFLUX;
  int passpop = _MONTHS_POP;
  int influx = 0;
  int pop = 0;
  
  //printf("month %d Squirrels in cells %d  rank: %d \n", month, cur_s->squirrels_healthy, rank);

  /*Append the list of the months*/

  /* If this is not the first month*/
  if (month != 0)
  { /* If the month is less than the thresholds*/
    if (passpop > month)
      passpop = month;
    if (passinflux > month)
      passinflux = month;
  }
  else
  {
    passpop = 1;
    passinflux = 1;
  }
  int month_index=0;
      //printf("Population1 %f, Influx %f month %d \n", cell->influx, cell->pop,month);
 

  while (month > month_index && passpop > 0 && month>0)
  {
      int unhealthy_s=  stats[(0 * num_cells *2* _MAX_MONTHS_SIMULATION )+ (cellID * _MAX_MONTHS_SIMULATION )+ month_index];
      int healthy_s=  stats[(1 * num_cells *2* _MAX_MONTHS_SIMULATION )+ (cellID * _MAX_MONTHS_SIMULATION) + month_index];
  //    printf("UNHEALT%d\n",unhealthy_s);
   //   printf("HEALT %d\n",healthy_s);
   // printf("month %d, month %d Squirrels in cells %d  rank: %d \n", d,month, cur_s->squirrels_healthy, rank);
    if (passinflux > 0)
    {
      influx +=   unhealthy_s;

      pop += healthy_s+unhealthy_s;

      passpop--;
      passinflux--;
    }
    else if (passpop > 0)
    {
      pop += healthy_s+unhealthy_s;
      passpop--;
    }
    month_index++;
  }

  cell->pop = pop;
  cell->influx = influx;
  //cell->squirrels_healthy[month] = 0;
  if (0)
      printf("Population %d, Influx %d month %d CellID %d rank %d \n",  cell->pop,cell->influx,month,cellID,rank);
  //cell->squirrels_unhealthy[month] = 0;
  
}