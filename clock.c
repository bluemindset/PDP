
/***************************ACTORS***********************************/
/********************************************************************/
#include "actor.h"
#include "squirrel.h"
#include "registry.h"
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
  int i = 0, j = 0, day = 0;
  int forever = 1;
  struct Registry_cell *current_r = r;
  int workers[workers_size][16];
  int num_workers = 0, k = 0, num_cells = 0, squirrels = 0;
  int *cells;

  /*******Send to all cell workers that day has passed******/
  MPI_Request *rsr = (MPI_Request *)malloc(sizeof(MPI_Request) * num_cells);
    MPI_Request *rss = (MPI_Request *)malloc(sizeof(MPI_Request) * num_cells);

  float data_r[num_cells][5];
  float avg_influx;
  float avg_pop;
  int cell_id;
  /*Squirrels healthy and unhealthy that passed from each cell*/
  int *squirrels_healthy = (int *)calloc(num_cells, sizeof(int));
  int *squirrels_unhealthy = (int *)calloc(num_cells, sizeof(int));
  int y = 0;

 
  while (current_r != NULL)
  {
    /*If the rank contains cells*/
    if (current_r->num_c != 0)
    {
      /* Workers  */
      for (k = 0; k < current_r->num_c; k++)
      {
        /*Send to every worker the cells*/
        printf("[Worker]CLock to Cell ID %d %d \n", current_r->rank, _TAG_CLOCK + current_r->actors_ID[k]);
        MPI_Isend(&clock->timeline->ID, 1, MPI_INT, current_r->rank, _TAG_CLOCK + current_r->actors_ID[k], MPI_COMM_WORLD, &rss[y]);
        /*Receive the from the cell*/

        MPI_Irecv(&data_r[i], 5, MPI_INT, current_r->rank, _TAG_CLOCK + current_r->actors_ID[k], MPI_COMM_WORLD, &rsr[y]);
        y ++;
      }
      num_cells += current_r->num_c;
      num_workers++;
    }
    else
    {
      squirrels += current_r->num_s;
    }
    current_r = current_r->next;
    i++;
  }
  delay(1);

  MPI_Waitall(num_cells, rss, MPI_STATUSES_IGNORE);

  printf("HHHHHHHHHHHHHHHHHH\n");
  MPI_Waitall(num_cells, rsr, MPI_STATUSES_IGNORE);

  /* Receive from each cell
  a) Cell ID
  b) Avg population of the cell
  c) Avg Influx of the cell
  d) Unhealthy squirrells
  e) Healthy squirrels
*/

  for (i = 0; i < num_cells; i++)
  {
    cell_id = data_r[i][4];
    avg_pop += data_r[i][0];
    avg_influx += data_r[i][1];
    squirrels_healthy[cell_id] = data_r[i][2];
    squirrels_unhealthy[cell_id] = data_r[i][3];
  }
  /* Update the timeline of the simulation, by inserting the number of healthy and unhealthy squirrels, influx and
  population from each cell*/
  chronicle(&clock->timeline, squirrels_healthy, squirrels_unhealthy, avg_influx, avg_pop, num_cells);

  printf("Cell has received %f, %f \n", avg_influx, avg_pop);

  if (clock->timeline->ID == 30)
    send_command(_COMPLETE, _MASTER, 0);
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