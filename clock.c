
/***************************ACTORS***********************************/
/********************************************************************/
#include "actor.h"
#include "registry.h"
#include "squirrel.h"
#include "cell.h"
#include "clock.h"
/*************************PROCESS************************************/
/********************************************************************/
#include "process_pool.h"
#include "master.h"
#include "worker.h"
/*************************LIBRARIES**********************************/
/********************************************************************/
#include "stdio.h"
#include "mpi.h"
#include <time.h>
/********************************************************************/
#include "main.h"
/********************************************************************/

void clock_work(Registry_cell *r, int workers_size, struct Clock *clock)
{
   /* Clock, one day passes */  
  int num_cells = 16;
  int i = 0, j = 0, day = 0, y=0;
  int forever = 1;
  struct Registry_cell *current_r = r;
  int workers[workers_size][16];
  int num_workers = 0, k = 0, squirrels = 0;
  int *cells;

  /*******Send to all cell workers that day has passed******/
  MPI_Request rsr [num_cells];
  MPI_Request rss [num_cells];

  int data_r[num_cells][3];
  float avg_influx  =0 ;
  float avg_pop = 0;
  int cell_id;

  /*Squirrels healthy and unhealthy that passed from each cell*/
  int *squirrels_healthy = (int *)calloc(num_cells, sizeof(int));
  int *squirrels_unhealthy = (int *)calloc(num_cells, sizeof(int));

  while (current_r != NULL)
  {
    /*If the rank contains cells*/
    if (current_r->num_c != 0)
    {
      /* Workers  */
      for (k = 0; k < current_r->num_c; k++)
      {
        /*Send to every worker the cells*/
        if(_DEBUG)
        printf("[Worker]Clock to Cell ID %d %d \n", current_r->rank, _TAG_CLOCK + current_r->actors_ID[k]);
        MPI_Isend(&clock->timeline->ID, 1, MPI_INT, current_r->rank, _TAG_CLOCK + current_r->actors_ID[k], MPI_COMM_WORLD, &rss[y]);
        /*Receive the from the cell*/
        MPI_Irecv(&data_r[i], 3, MPI_INT, current_r->rank, _TAG_CLOCK + current_r->actors_ID[k], MPI_COMM_WORLD, &rsr[y]);
        y++;
        i++;
      }
      
      num_workers++;
    }
    else
    {
      squirrels += current_r->num_s;
    }
    current_r = current_r->next;
  }

  MPI_Waitall(num_cells, rss, MPI_STATUSES_IGNORE);
  MPI_Waitall(num_cells, rsr, MPI_STATUSES_IGNORE);


  /*Receive from each cell
    a) Cell ID
    b) Avg population of the cell
    c) Avg Influx of the cell
    d) Unhealthy squirrells
    e) Healthy squirrels
*/
  for (i = 0; i < num_cells; i++)
  {
    squirrels_healthy[cell_id] = data_r[i][0];
    squirrels_unhealthy[cell_id] = data_r[i][1];
    cell_id = data_r[i][2];
  }
  /* Update the timeline of the simulation, by inserting the number of healthy and unhealthy squirrels, influx and
  population from each cell*/

  chronicle(&clock->timeline, squirrels_healthy, squirrels_unhealthy, avg_influx, avg_pop, num_cells);

 // if(_DEBUG)
  printf("Day is Clock %d \n",clock->timeline->ID );


}

void delay(unsigned int secs)
{
  unsigned int retTime = time(0) + secs / 10; // Get finishing time.
  while (time(0) < retTime)
    ; // Loop until it arrives.
}

static struct Clock new (int rank, int ID)
{
  struct Clock clock;

  clock.actor = Actor.new(rank, ID);
  clock.timeline = (struct Day *)malloc(sizeof(struct Day));
  clock.timeline->ID = 0;
  clock.timeline->squirrels_healthy = 0;
  clock.timeline->squirrels_unhealthy = 0;
  clock.timeline->avg_influx = 0;
  clock.timeline->avg_pop = 0;
  
  return clock;
}

const struct ClockClass Clock = {.new = &new};