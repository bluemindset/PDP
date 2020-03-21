
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
/********************************************************************/
void master_send_instructions(int num_cells, int num_squirrels, int size,struct Registry_cell * r,int * workers)
{
  /*Master sends the number of squirrels*/
  /*The tags for exchange the messages are 0 */
  //  int number_squirrel = 4;
  //  int number_cells = 16;
  /*Send a broadcast to the workers the two parameters*/
  int i;
       printf("USA");

  /*
    Data package 
    [0] = number of squirrels to instantiate
    [1] = number of cells to instantiate
    [2] = start ID of cell
 */
  int c = 0;
  int data[3];
  
  // for (i = 0; i < size; i++)
  // {
  //    fflush(stdout);
  //    printf("USA");
  //    //printf("U %d",i);
  //    fflush(stdout);
    /*if UE is not active*/
  //  if (workers[i])
  //  {
  //    data[0] = num_squirrels / size;
  //    data[1] = num_cells / size;
  //    data[3] = c;
  //    printf("USA");
  //    printf("U %d",workers[i]);
  //    MPI_Send(&data, 3, MPI_INT,workers[i], _TAG_INITIAL, MPI_COMM_WORLD);
     
  //  }
  //  c += num_cells / size;
  //}
  int reduction_result = 0;

  //MPI_Reduce(&c, &reduction_result, 1, MPI_INT, MPI_SUM, _MASTER, MPI_COMM_WORLD);
 // print_register(r);
}


void masterlives()
{
  int masterStatus = receiving_handle();
  while (masterStatus)
  {
    masterStatus = receiving_handle();
  }
}

 void startworkers(int num_workers, int *workers)
  {
    int i = 0;

    for (i = 0; i < num_workers; i++)
    {
      workers[i] = startWorkerProcess();
      if (workers[i] == 0)
        exit(0);
    }
  }
