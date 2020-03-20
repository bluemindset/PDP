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

/*************************LIBRARIES**********************************/
/********************************************************************/
#include "stdio.h"
#include "mpi.h"
#include <time.h>
/********************************************************************/
#include "main.h"
/********************************************************************/



int main(int argc, char *argv[])
{
  double x = 5;

  /****Initialize MPI****/
  MPI_Init(NULL, NULL);

  int rank;
  int size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int *workers = (int *)malloc(size * sizeof(int));
  int statusCode = create_pool();

  _registry_cell *registry = (_registry_cell *)malloc(16 * sizeof(_registry_cell));

  /*Give work to a worker*/
  if (statusCode == 1)
  {
    worker(rank, registry);
  }
  else if (statusCode == 2)
  {
    //08000468687
    /*Worker determines the jobs to give not other workers*/
    startworkers(size - 1, workers);

    /*While the master lives*/
    masterlives();

    /*Send back to master the data */
    //MPI_Recv(data, data_length, MPI_DOUBLE, pid, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }
  // MPI_Barrier(MPI_COMM_WORLD);
  terminate_pool();
  MPI_Finalize();

  return 0;
}

static void initialiseRNG(long *seed)
{
    ran2(seed);
}
