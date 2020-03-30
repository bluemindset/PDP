
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

static struct Cell new(int rank,int ID, float pos_x,float pos_y) {
	  struct Cell cell={.pos_x= pos_x,.pos_y= pos_y};
        cell.actor = Actor.new(rank, ID);
        cell.day_stats =  (struct Day_stat *)malloc(sizeof(struct  Day_stat));
        cell.day_stats->squirrels_healthy = 0;
        cell.day_stats->squirrels_unhealthy = 0;
        cell.day_stats->nextstat = NULL;
        
    return cell; 
}


const struct CellClass Cell = {.new = &new};
/**
 * Returns the id of the cell from its x and y coordinates.
 */

void update_cells(struct Cell *cell, int day)
{
  int i, sum = 0;

  struct Day_stat *day_s = (struct Day_stat *)malloc(sizeof(struct Day_stat));
  struct Day_stat *cur_s = cell->day_stats;
  struct Day_stat **head_s = &cell->day_stats;
  
  /*Thresholds*/
  int passinflux = _DAYS_INFLUX;
  int passpop = _DAYS_POP;
  int influx=0;
  int pop=0;

  day_s->squirrels_healthy =0; 
  day_s->squirrels_unhealthy =0; 
  day_s->nextstat = *head_s;
  *head_s = day_s;

  if (passpop > day)
    passpop = day;
  if (passinflux > day)
    passinflux = day;

  while (cur_s != NULL && passpop > 0 )
  {
    if (passinflux > 0)
    {  
        influx += cur_s->squirrels_healthy;
        pop += cur_s->squirrels_healthy + cur_s->squirrels_unhealthy;
        passpop--;
        passinflux--;
      }
    else if (passpop > 0)
    {
        pop += cur_s->squirrels_healthy + cur_s->squirrels_unhealthy;
        passpop--; 
    }
    cur_s = cur_s->nextstat;
  }

}