
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

void clock_work(Registry_cell *r, int workers_size,struct Clock * clock)
{
  int i, day = 0;
  int forever = 1;
  struct Registry_cell *current_r = r;
  int workers[workers_size];
  int w_count=0,cells =0 ;

  /*******Send to all cell workers that day has passed******/
  while (current_r != NULL)
  {
    /*If the rank contains cells*/
    if (current_r->num_c != 0)
    {
      workers[i] = current_r->rank;
      w_count++;
      cells+=current_r->num_c;
    }
    current_r = current_r->next;
    i++;
  }
  MPI_Request *rs = (MPI_Request *)malloc(sizeof(MPI_Request) * w_count);
  float data_r[cells][2];
  float avg_influx; 
  float avg_pop; 
  
  for (i = 0; i < w_count; i++){
    MPI_Isend(&day, 1, MPI_INT, current_r->rank, _TAG_CLOCK, MPI_COMM_WORLD, rs[i]);
    MPI_Irecv(&data_r[i][2],2,MPI_INT, current_r->rank, _TAG_CLOCK, MPI_COMM_WORLD, rs[i]);
  }
  /********* Receive from cells  ***********/
  delay(1);
  for (i = 0; i < cells; i++){
    avg_influx += data_r[i][0];
    avg_pop += data_r[i][1];
  }

  chronicle(clock->timeline, 0,0, avg_influx,avg_pop);
  MPI_Waitall(w_count, rs, MPI_STATUSES_IGNORE);
  
  printf("Cell has received %f, %f \n",avg_influx,avg_pop);
 
  if (day == 30)
    send_command(_COMPLETE, _MASTER, 0);
}

void delay(unsigned int secs)
{
  unsigned int retTime = time(0) + secs / 10; // Get finishing time.
  while (time(0) < retTime)
    ; // Loop until it arrives.
}

static struct Clock new(int rank,int ID) {
	  struct Clock clock;
    
    clock.actor = Actor.new(rank, ID);
    clock.timeline = (struct Day*)malloc(sizeof(struct Day));
   
    return clock; 
}


const struct ClockClass Clock = {.new = &new};