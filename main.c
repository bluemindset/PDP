
/*************************LIBRARIES**********************************/
/********************************************************************/
#include "mpi.h"
#include "omp.h"
#include <math.h>
#include "stdio.h"
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

int main(int argc, char *argv[])
{

  /*Create a registry*/
  Registry_cell *registry = NULL;

  /****Initialize MPI****/
  int num_cells = 16;
  int num_squirrels = 4;

  int rank;
  int size;

  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int *workers = (int *)malloc(size * sizeof(int));
  int statusCode = create_pool();

  /*Give work to a worker*/
  if (statusCode == 1)
  {
    worker(rank, registry, size - 1);
  }
  else if (statusCode == 2)
  {
    //08000468687
    /*Worker determines the jobs to give not other workers*/
    startworkers(size - 1, workers);

    master_send_instructions(16, 4, size - 1, &registry, workers);
    
    /*While the master lives*/
    masterlives(registry);

    /*Send back to master the data */
  }
  terminate_pool();
  MPI_Finalize();

  return 0;
}

static void initialiseRNG(long *seed)
{
  ran2(seed);
}

MPI_Request squirrels_work(struct Squirrel *squirrel, int rank, struct Registry_cell *registry, int data_recv[2])
{

  if (squirrel->health)
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

    /*Send the Cell ID to master to return the rank back*/
    MPI_Ssend(&cellID, 1, MPI_INT, _MASTER, _TAG_REGISTRY_CELL, MPI_COMM_WORLD);

    /*Receive the cell rank from the master*/
    int cell_rank;
    MPI_Recv(&cell_rank, 1, MPI_INT, _MASTER, _TAG_REGISTRY_CELL, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    
    /*Create the package and send it*/
    int data[2];
    data[0] = squirrel->health;
    data[1] = squirrel->actor.ID;
    int tag = cellID * _TAG_SQUIRRELS;
    MPI_Send(&data, 2, MPI_INT, cell_rank, tag, MPI_COMM_WORLD);
    if (_DEBUG)
      printf("[Worker] Squirrel %d sending data to cell %d on rank %d with tag %d\n", squirrel->actor.ID, cellID, cell_rank, tag);

    MPI_Request r;
    if (_DEBUG)
      printf("[Worker] Squirrel %d issued receive %d on rank %d with tag %d\n", squirrel->actor.ID, cellID, cell_rank, tag);
    
    MPI_Irecv(data_recv, 2, MPI_INT, cell_rank, tag, MPI_COMM_WORLD, &r);
    return r;
  }
}

void squirrel_life(struct Squirrel *squirrel, int influx, int pop)
{
  long seed = 1;
  /*Update the averages of the its population and influx*/
  squirrel->update_avgs(influx, pop, squirrel);
  if (_DEBUG)
    printf("[Worker] Squirrel %d has avg influx: %f and pop %f \n", squirrel->actor.ID, squirrel->avg_influx, squirrel->avg_pop);

  if (willCatchDisease(squirrel->avg_influx, &seed))
  {
    squirrel->health = 0;
  }

  //squirrel->willGiveBirth(seed);
  if (!squirrel->health)
  {
    if (willDie(&seed))
      squirrel->health = 0;
  }
}

void cells_work(struct Cell *cell, int rank, struct Registry_cell *registry)
{
  int forever = 1;
  int seed = 0;
  int influx, pop, f, s_health;

  MPI_Status status;
  /*Check if there is message from any other actor and act accordiclty*/
  int tag = cell->actor.ID * _TAG_SQUIRRELS;
  //printf("[Worker] Cell:%d issued Probe on Rank:%d with tag %d \n", cell->actor.ID,rank,tag);
  MPI_Iprobe(MPI_ANY_SOURCE, cell->actor.ID * _TAG_SQUIRRELS, MPI_COMM_WORLD, &f, &status);

  /*Check the TAG*/
  if (f)
  {

    if (if_squirrels_msg(status))
    {
      MPI_Request request_inflix;
      MPI_Request request_pop;
      //IMPLEMENT EVERYHTING WITH A TAG
      int recv_data[2];
      int send_data[2] = {(cell->influx), (cell->pop)};
      
      /*Send the inlfux and population values to the incoming squirrel */
      MPI_Recv(&recv_data, 2, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
      // printf("[Worker]Cell ID %d Received Data from %d  \n",cell->actor.ID,status.MPI_TAG);

      MPI_Send(&send_data, 2, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD);
      // printf("[Worker]%d %d \n",send_data[0],send_data[1]);
      if(_DEBUG)
      printf("[Worker]Cell ID %d Sending Data to %d  \n", cell->actor.ID, status.MPI_TAG);

      /*Receive health value from the squierrel*/
      //data[0] is squirrel health
      //data[1] is the cell ID

       if (recv_data[0])
         cell->squirrels_healthy++;
       else
         cell->squirrels_unhealthy++;
      printf("[Worker]Cell ID %d Sending Data to %d  \n", cell->actor.ID, status.MPI_TAG);

    }
    if (if_clock_msg(status))
    {
      /* Perfome update of the cell influx and population values*/
      /*This means that a day passed and the squirrels have to be updated*/
      
      int data_send[5];
      int data_recv[2];
      data_send[0]= cell->pop;
      data_send[1]= cell->influx;
      data_send[2]= cell->squirrels_healthy;
      data_send[3]= cell->squirrels_healthy;
      data_send[4] =cell->actor.ID;
      int k; 
      MPI_Recv(&k, 1, MPI_INT, _MASTER, _TAG_CLOCK, MPI_COMM_WORLD, &status);
      MPI_Send(&data_send,5,MPI_INT, _MASTER, _TAG_CLOCK+cell->actor.ID,MPI_COMM_WORLD);
      MPI_Recv(&data_recv,2,MPI_INT, _MASTER, _TAG_CLOCK+cell->actor.ID,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
      
      /*Update individual cell data*/
      cell->influx = data_recv[0];
      cell->pop = data_recv[1];
    }
    //forever = 0; //should_terminate_worker();
  }
}

// /*****Declare TAGS*****/
// int *tags = (int *)(malloc(sizeof(int) * num_cells * num_squirrels));

// int k,i,j = 0;
// for (i = 0; i < num_cells; i++)
// {
//   for (j = 0; j < num_squirrels; j++)
//   {
//     /*For each cell put the squirrel tags*/
//     *(tags + i * num_squirrels + j) = k++;
//   }
// }
