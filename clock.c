
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

void clock_work()
{
    int day = 0;
    int forever = 1;
    /*Broadcast everyone except self*/
    while (forever)
    {
        delay(1);
        day++;
        /*Gather all the workers*/
        MPI_Bcast(&day, 1, MPI_INT, _MASTER, MPI_COMM_WORLD);
        if (day == 30)
            send_command(_COMPLETE, _MASTER, 0);
    }
}

  void delay(unsigned int secs)
  {
    unsigned int retTime = time(0) + secs / 10; // Get finishing time.
    while (time(0) < retTime)
      ; // Loop until it arrives.
  }
