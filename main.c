
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
  
  /*****Declare TAGS*****/
  int *tags = (int *)(malloc(sizeof(int) * num_cells * num_squirrels));

  int k,i,j = 0;
  for (i = 0; i < num_cells; i++)
  {
    for (j = 0; j < num_squirrels; j++)
    {
      /*For each cell put the squirrel tags*/
      *(tags + i * num_squirrels + j) = k++;
    }
  }

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
    // print_register(registry);
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

void squirrels_work(struct Squirrel * squirrel, int rank, struct Registry_cell *registry)
{
  int die = 0;
  float new_x, new_y;
  long seed = 1;
  printf("[Worker]Squirrel is ready  %d\n",squirrel->actor.ID);

  //while (!die)
  // {
  int data[3];
  float influx;
  float pop;

  int debug = 0;

  /*If squirrel died then skip the rest and flag to 1*/
  if (!squirrel->health)
  {
    if (willDie(&seed))
      squirrel->health =0;
  }
  /*Squirrel Moves*/

  squirrelStep(squirrel->pos_x, squirrel->pos_y, &new_x, &new_y, &seed);

  squirrel->pos_x = new_x;
  squirrel->pos_y = new_y;

  /*Send health to the stepping cell*/
  int cellID = getCellFromPosition(squirrel->pos_x, squirrel->pos_y);

  /*Send the Cell Id to master to return the rank back*/
  MPI_Ssend(&cellID, 1, MPI_INT, _MASTER, _TAG_REGISTRY_CELL, MPI_COMM_WORLD);

  int cell_rank;
  /*Receive the cell rank from the master*/
  MPI_Recv(&cell_rank, 1, MPI_INT, _MASTER, _TAG_REGISTRY_CELL, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  /*Create the package and send it*/
  
  data[0] = squirrel->health;
  data[1] = squirrel->actor.ID;

  MPI_Send(&data, 2, MPI_INT, cell_rank, cellID*_TAG_SQUIRRELS , MPI_COMM_WORLD);
  printf("[WORKER] Squirrel %d sending data to cell %d\n\n",squirrel->actor.ID ,cellID);
  
  MPI_Status s1;

  int f;
  MPI_Request r1;


  /*Receive message from the cell*/
  MPI_Iprobe(cell_rank, squirrel->actor.ID*_TAG_CELLS , MPI_COMM_WORLD, &f, &s1);
  //printf("CELL IS %d RANK IS IN CELL %d\n", cellID, cell_rank);
  if (f)
  {
    float data_r[2];
    MPI_Recv(&data, 2, MPI_FLOAT, cell_rank,  squirrel->actor.ID*_TAG_CELLS, MPI_COMM_WORLD,&s1);
    printf("INFLUX %fPOP %f\n", data_r[0], data_r[1]);
  }
  //if (debug){

  // squirrel->update_avgs(influx, pop, squirrel);

  //  if (willCatchDisease(squirrel->avg_influx, &seed))
  //  {
  //    squirrel->health = 0;
  //   }
  //  }
  // squirrel->willGiveBirth(seed);
  // }
}

void cells_work(struct Cell *cell, int rank, struct Registry_cell *registry)
{
  int forever = 1;
  int seed = 0;
  int influx,pop,f,s_health;
  int data[2];
  
  MPI_Status status;
  /*Check if there is message from any other actor and act accordiclty*/
  
  MPI_Iprobe(MPI_ANY_SOURCE, cell->actor.ID*_TAG_SQUIRRELS , MPI_COMM_WORLD, &f, &status);

  /*Check the TAG*/
  if (f)
  {
  printf("AM IN %d\n\n", cell->actor.ID);

  //  if (if_squirrels_msg(status))
   // {
      MPI_Request request_inflix;
      MPI_Request request_pop;
      //IMPLEMENT EVERYHTING WITH A TAG

      /*Send the inlfux and population values to the incoming squirrel */
      MPI_Recv(&data, 2, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
      float send_data[2] = {(cell->influx), (cell->pop)};
      MPI_Send(&send_data, 2, MPI_FLOAT, status.MPI_SOURCE, data[1] * _TAG_CELLS , MPI_COMM_WORLD);
      /*Receive health value from the squierrel*/
      //data[0] is squirrel health
      //data[1] is the cell ID

      if (data[0])
        cell->squirrels_day->squirrels_healthy++;
      else
        cell->squirrels_day->squirrels_unhealthy++;

  //  }
    if (if_clock_msg(status))
    {
      /* Perfome update of the cell influx and population values*/
      /*This means that a day passed and the squirrels have to be updated*/
      update_cell_day(cell);
      int day;
      MPI_Recv(&day, 1, MPI_INT, MPI_ANY_SOURCE, _TAG_CLOCK, MPI_COMM_WORLD, &status);

      if (day > _DAYS_POP)
      {
        chronicle(&cell->squirrels_day, 0, 0);
        erase_day(cell->squirrels_day);
      }
      else
      {
        chronicle(&cell->squirrels_day, 0, 0);
      }
    }
    //forever = 0; //should_terminate_worker();
  }
}