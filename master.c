
/**************************************************************/
/**************************************************************/
#include "actor.h"
#include "squirrel.h"
#include "cell.h"
#include "process_pool.h"
#include "stdio.h"
#include "mpi.h"
#include "main.h"
#include <time.h>
/**************************************************************/
/**************************************************************/
void master_send_instructions(int num_cells, int num_squirrels, int size)
{

  /*Master sends the number of squirrels*/
  /*The tags for exchange the messages are 0 */
  //  int number_squirrel = 4;
  //  int number_cells = 16;
  /*Send a broadcast to the workers the two parameters*/
  int i;
  /*Number of squirrels*/
  /*Number of cells*/
  /*Number of cell IDs*/
  int c = 0;
  int data[4];
  for (i = 0; i < size - 1; i++)
  {
    /*if UE is not active*/
    if (UEs_state[i])
    {
      data[0] = num_squirrels;
      data[1] = num_cells;
      data[3] = c;
      data[4] = c + size;
      MPI_Send(&data, 4, MPI_INT, i + 1, _TAG_SQUIRRELS, MPI_COMM_WORLD);
    }
    c += size;
  }

  configure_regirstry();//todo
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
      if (workers[i] == NULL)
        exit(0);
    }
  }
