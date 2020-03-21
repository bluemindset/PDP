
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
void master_send_instructions(int num_cells, int num_squirrels, int size, struct Registry_cell *r, int *workers)
{
  /*Master sends the number of squirrels*/
  /*The tags for exchange the messages are 0 */
  //  int number_squirrel = 4;
  //  int number_cells = 16;
  /*Send a broadcast to the workers the two parameters*/
  int i;
  /*
    Data package 
    [0] = number of squirrels to instantiate
    [1] = number of cells to instantiate
    [2] = start ID of cell
 */
  int c = 0;
  int data[4];
  int squirrels_startID = 0, cells_startID = 0;
  int squirrels_endID = num_squirrels / size;
  int cells_endID = num_cells / size;

  for (i = 0; i < size; i++)
  {
    /*if UE is not active*/
    if (workers[i])
    {

      if (i == size - 1)
      {
        cells_endID++;
        squirrels_endID++;
      }
      data[0] = squirrels_startID;
      data[1] = squirrels_endID;
      data[2] = cells_startID;
      data[3] = cells_endID;

      assign_registry(&r, workers[i], squirrels_startID, squirrels_endID,
                      cells_startID, cells_endID);

      MPI_Send(&data, 4, MPI_INT, workers[i], _TAG_INITIAL, MPI_COMM_WORLD);
      printf("Sending to rank : %d, the data %d %d %d %d\n", workers[i], data[0], data[1], data[2], data[3]);

      squirrels_startID += num_squirrels / size;
      cells_startID += num_cells / size;

      cells_endID += num_cells / size;
      squirrels_endID += num_squirrels / size;
    }
  }
  int msg = 0;
  int success_assign = 0, success_all_assign = 0;

  MPI_Reduce(&success_assign, &success_all_assign, 1, MPI_FLOAT, MPI_SUM, _MASTER,
             MPI_COMM_WORLD);
  printf("Success spaniwng of actors", success_all_assign, size);
  if (success_all_assign == (size))
    print_register(r);
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
