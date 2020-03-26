
/***************************ACTORS***********************************/
/********************************************************************/
#include "actor.h"
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
#include "registry.h"
/********************************************************************/

void clock_work(Registry_cell *r, int workers_size, struct Clock *clock)
{
  int i=0 , day = 0;
  int forever = 1;
  struct Registry_cell *current_r = r;
  int workers[workers_size][2];
  int num_workers = 0, k=0,num_cells = 0, squirrels = 0;
  int * cells;
  /*******Send to all cell workers that day has passed******/
  while (current_r != NULL)
  {
    /*If the rank contains cells*/
    if (current_r->num_c != 0)
    {
      /* Workers = */
      workers[i][0] = current_r->rank;
      for (k = 0 ; k <current_r->num_c ; k++)
        workers[i][1] = current_r->actors_ID[k];
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

  MPI_Request *rs = (MPI_Request *)malloc(sizeof(MPI_Request) * num_cells*2);
  
  float data_r[num_cells][5];
  float avg_influx;
  float avg_pop;
  int cell_id;
  /*Squirrels healthy and unhealthy that passed from each cell*/
  int *squirrels_healthy = (int *)calloc(num_cells, sizeof(int));
  int *squirrels_unhealthy = (int *)calloc(num_cells, sizeof(int));
  int i,j;

  for (i = 0; i < num_workers; i++)
  { 
    for(j =0 ; j< workers[i][1];j++){
      /*Send to every worker the cells*/
      MPI_Isend(&day, 1, MPI_INT, workers[i][0], _TAG_CLOCK, MPI_COMM_WORLD, rs[i]);
      /*Receive the from the cell*/
      MPI_Irecv(&data_r[i][4], 2, MPI_INT, current_r->rank, _TAG_CLOCK, MPI_COMM_WORLD, rs[i]);
    }
    //workers[i][0]
    //workers[i][1]  
  }
  /********* Receive from cells  ***********/
  delay(1);

  MPI_Waitall(num_cells*2, rs, MPI_STATUSES_IGNORE);

  for (i = 0; i < num_cells; i++)
  {
    cell_id = data_r[i][4];
    avg_pop += data_r[i][0];
    avg_influx += data_r[i][1];
    squirrels_healthy[cell_id] = data_r[i][2];
    squirrels_unhealthy[cell_id] = data_r[i][3];
  }

  chronicle(clock->timeline, squirrels_healthy, squirrels_unhealthy, avg_influx, avg_pop);
  update_cells(clock, cells);

  printf("Cell has received %f, %f \n", avg_influx, avg_pop);

  if (day == 30)
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

  return clock;
}
void update_cells(struct Clock *clock, int num_cells)
{
  int i, sum = 0;
  // this->influx = this->pop = 0;
  int c = 0;
  Day *day = clock->timeline;
  int *pop = (int *)calloc(num_cells, sizeof(int));
  int *influx = (int *)calloc(num_cells, sizeof(int));
  MPI_Request *rs = (MPI_Request *)malloc(sizeof(MPI_Request) * num_cells);
  /*Get current day*/
  int passinflux = _DAYS_INFLUX;
  int passpop = _DAYS_POP;

  if (passpop > clock->timeline->ID)
    passpop = clock->timeline->ID;
  if (passinflux > clock->timeline->ID)
    passinflux = clock->timeline->ID;

  while (day != NULL && passpop != 0)
  {
    if (passinflux > 0)
    {
      for (i = 0; i < num_cells; i++)
      {
        influx[i] += day->squirrels_healthy[i];
        pop[i] += day->squirrels_healthy[i] + day->squirrels_unhealthy[i];
        passpop--;
        passinflux--;
      }
    }
    else if (passpop > 0)
    {
      for (i = 0; i < num_cells; i++)
      {
        pop[i] += day->squirrels_healthy[i] + day->squirrels_unhealthy[i];
        passpop--;
      }
    }
    day = day->nextday;
  }

  /*Send the update influx after 2 months and population after 3 months*/

  int data_send[2];
  for (i = 0; i < num_cells; i++)
  {
    data_send[0] = influx[i];
    data_send[1] = pop[i];
    MPI_Send(&data_send, 2, MPI_INT, i, _TAG_CLOCK, MPI_COMM_WORLD);
  }

}

const struct ClockClass Clock = {.new = &new};