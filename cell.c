
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

void update_cell(struct Cell *cell, int month, int rank, int *stats, int cellID, int num_cells)
{

  int healthy_s_3, healthy_s_2, healthy_s_1;
  int unhealthy_s_3, unhealthy_s_2, unhealthy_s_1;

  if (month == 0)
  {
    /*Healthy and Unhealthy squirrels of THIS month that passed*/
    unhealthy_s_1 = stats[(0 * num_cells * 2 * _MAX_MONTHS_SIMULATION) + (cellID * _MAX_MONTHS_SIMULATION) + month];
    healthy_s_1 = stats[(1 * num_cells * 2 * _MAX_MONTHS_SIMULATION) + (cellID * _MAX_MONTHS_SIMULATION) + month];
    /*Influx is calculated by this month that passed*/
    cell->influx = unhealthy_s_1 + unhealthy_s_2;
    /*Population is calculated by this month that passed*/
    cell->pop = healthy_s_1 + healthy_s_2;
  }
  else if (month == 1)
  {
    /*Healthy and Unhealthy squirrels of the PREVIOUS month that passed*/
    unhealthy_s_1 = stats[(0 * num_cells * 2 * _MAX_MONTHS_SIMULATION) + (cellID * _MAX_MONTHS_SIMULATION) + month];
    unhealthy_s_2 = stats[(0 * num_cells * 2 * _MAX_MONTHS_SIMULATION) + (cellID * _MAX_MONTHS_SIMULATION) + month - 1];
    
    healthy_s_1 = stats[(1 * num_cells * 2 * _MAX_MONTHS_SIMULATION) + (cellID * _MAX_MONTHS_SIMULATION) + month];
    healthy_s_2 = stats[(1 * num_cells * 2 * _MAX_MONTHS_SIMULATION) + (cellID * _MAX_MONTHS_SIMULATION) + month - 1];

    /* VIRUS can only live without a host for two months in the environment*/
    cell->influx = unhealthy_s_1 + unhealthy_s_2;
    /*Population is calculated by the last two months*/
    cell->pop = healthy_s_1 + healthy_s_2;
  }
  else if (month >= 2)
  {
    /*Healthy and Unhealthy squirrels of TWO month AGO that passed*/
    unhealthy_s_1 = stats[(0 * num_cells * 2 * _MAX_MONTHS_SIMULATION) + (cellID * _MAX_MONTHS_SIMULATION) + month];
    unhealthy_s_2 = stats[(0 * num_cells * 2 * _MAX_MONTHS_SIMULATION) + (cellID * _MAX_MONTHS_SIMULATION) + month - 1];
    
    healthy_s_1 = stats[(1 * num_cells * 2 * _MAX_MONTHS_SIMULATION) + (cellID * _MAX_MONTHS_SIMULATION) + month];
    healthy_s_2 = stats[(1 * num_cells * 2 * _MAX_MONTHS_SIMULATION) + (cellID * _MAX_MONTHS_SIMULATION) + month - 1];
    healthy_s_3 = stats[(1 * num_cells * 2 * _MAX_MONTHS_SIMULATION) + (cellID * _MAX_MONTHS_SIMULATION) + month - 2];

    /* VIRUS can only live without a host for two months in the environment if no unhealthy squirrel step in*/
    cell->influx = unhealthy_s_1 + unhealthy_s_2;
    /*Population is calculated by the last three months*/
    cell->pop = healthy_s_1 + healthy_s_2 + healthy_s_3;
  }

  if (_DEBUG)
    printf("Population %d, Influx %d month %d CellID %d rank %d \n", cell->pop, cell->influx, month, cellID, rank);
}