
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
  Registry_cell *registry = NULL;
  //registry = malloc(sizeof(struct Registry_cell)); // allocate 3 nodes in the heap
  //registry->next = NULL;
  /****Initialize MPI****/
  MPI_Init(NULL, NULL);

  int rank;
  int size;
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

void squirrels_work(struct Squirrel *squirrel, int rank, struct Registry_cell *registry, int tid)
{
  int die = 0;
  float new_x, new_y;
  long seed = 1;
  printf("SQUE %d, TID %d, RANK %d\n", squirrel->actor.ID, tid, rank);

  /*Drop the squirrel somewhere inside the map*/
  squirrelStep(0, 0, &new_x, &new_y, &seed);
  squirrel->pos_x = new_x;
  squirrel->pos_y = new_y;

  //printf("Squirrel is ready %d TID %d RANK %d\n",squirrel->actor.ID,tid,rank);
  //while (!die)
 // {
    int data[2];
    float influx;
    float pop;

    int debug = 0;

    /*If squirrel died then skip the rest and flag to 1*/
    if (!squirrel->health)
    {
      die = willDie(&seed);
    //  continue;
    }
    /*Squirrel Moves*/
    
    squirrelStep(squirrel->pos_x, squirrel->pos_y, &new_x, &new_y, &seed);

    squirrel->pos_x = new_x;
    squirrel->pos_y = new_y;

    /*Send health to the stepping cell*/
    int cellID = getCellFromPosition(squirrel->pos_x, squirrel->pos_y);
    printf("Squirrel Pos %f, %f\n",squirrel->pos_x, squirrel->pos_y);

    /*Access registry and find the cell*/
    printf("SEND CELL %d, TID %d, RANK %d\n",cellID, tid, rank);
  
    MPI_Ssend(&cellID, 1, MPI_INT, _MASTER, _TAG_REGISTRY_CELL, MPI_COMM_WORLD);

    
    int worker_rank;
    MPI_Recv(&worker_rank, 1, MPI_INT, _MASTER, _TAG_REGISTRY_CELL, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
    printf("CELL IS %d RANK IS IN CELL %d\n", cellID ,worker_rank);
      /*Create the package and send it*/
      data[0] = squirrel->health;
      data[1] = cellID;

      MPI_Send(&data, 2, MPI_INT, worker_rank, _TAG_SQUIRRELS, MPI_COMM_WORLD);
      //MPI_Recv(&worker_rank, 1, MPI_INT, _MASTER, _TAG_SQUIRRELS, MPI_COMM_WORLD,MPI_STATUS_IGNORE);

      MPI_Status s1;
      MPI_Status s2;
      MPI_Status s3;

      MPI_Request r1;
      MPI_Request r2;
      MPI_Request r3;
      /*Receive message from the cell*/
      //IMPLEMENT EVERYHTING WITH A TAG

      MPI_Irecv(&influx, 1, MPI_FLOAT , MPI_ANY_SOURCE, _TAG_SQUIRRELS, MPI_COMM_WORLD, &r1);
      MPI_Irecv(&pop, 1, MPI_FLOAT, MPI_ANY_SOURCE, _TAG_SQUIRRELS, MPI_COMM_WORLD, &r2);

      MPI_Wait(&r1, &s1);
      MPI_Wait(&r2, &s2);
      printf("INFLUX %dPOP %d\n", influx ,pop);
      //if (debug){
      MPI_Wait(&r3, &s3);
     // squirrel->update_avgs(influx, pop, squirrel);

    //  if (willCatchDisease(squirrel->avg_influx, &seed))
    //  {
    //    squirrel->health = 0;
   //   }
  //  }
    // squirrel->willGiveBirth(seed);
 // }
}

void cells_work(struct Cell *cell, int rank, struct Registry_cell *registry, int tid)
{
  int forever = 1;
  printf("CELL%d, TID %d, RANK %d\n", cell->actor.ID, tid, rank);

  while (forever)
  {
    int seed = 0;
    int influx;
    int pop;
    int f;
    int data[2];
    MPI_Status status;
    /*Check if there is message from any other actor and act accordiclty*/
    MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &f, &status);
    int s_health;
    if (f)
    {
      if (if_squirrels_msg(status))
      {
        MPI_Request request_inflix;
        MPI_Request request_pop;
        MPI_Request r3;
        MPI_Status s3;
        MPI_Wait(&r3, &s3);
        //IMPLEMENT EVERYHTING WITH A TAG

        /*Send the inlfux and population values to the incoming squirrel */
        MPI_Ssend(&(cell->influx), 1, MPI_FLOAT, status.MPI_SOURCE, _TAG_SQUIRRELS, MPI_COMM_WORLD);
        MPI_Ssend(&(cell->pop), 1, MPI_FLOAT, status.MPI_SOURCE, _TAG_SQUIRRELS, MPI_COMM_WORLD);
        /*Receive health value from the squierrel*/
        MPI_Recv(&data, 2, MPI_INT, MPI_ANY_SOURCE, _TAG_SQUIRRELS, MPI_COMM_WORLD, &status);
        printf("DDDDDDDDDDDDDDDDDDDD\n");
        //data[0] is squirrel health
        //data[1] is the cell ID

        if (data[0])
          cell->squirrels_day->squirrels_healthy++;
        else
          cell->squirrels_day->squirrels_unhealthy++;

        //  int cellID = access_registry(, registry)
      }
      else if (if_clock_msg(status))
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
      forever = 0;//should_terminate_worker();
    }
  }
}