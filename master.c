/**
 * @Author: B159973
 * @Date:	10/4/2019
 * @Course: Parallel Design Patterns - 2020
 * @University of Edinburgh
*/
/*************************LIBRARIES**********************************/
/********************************************************************/
#include "mpi.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
/***************************ACTORS***********************************/
/********************************************************************/
#include "actor.h"
#include "registry.h"
#include "clock.h"
#include "cell.h"
#include "squirrel.h"
/*************************PROCESS************************************/
/********************************************************************/
#include "process_pool.h"
#include "master.h"
#include "worker.h"

#include "main.h"
#include "ran2.h"
#define NUM_ACTORS 2
/********************************************************************/
/********************************************************************/
void master_send_instructions( int size, struct Registry_cell **r, int *workers)
{
  /*Master sends the number of squirrels*/
  /*
            Data package 
            [0] = start ID of squirrel
            [1] = end ID of squirrel
            OR
            [0] = start ID of cell
            [1] = end ID of cell
            AND
            [2] = instatiate squirrels(0) or cells(1) 
  */
  int i = 0;
  int *workers_cells;
  int *workers_squirrels;
  int ws = 0, wc = 0;

  /***************************************** DIVIDING THE WORK  ****************************************/

  /*A process will handle many actors but only one of a kind*/
  if (size % 2 == 0)
  {
    workers_cells = (int *)malloc(sizeof(int) * (size / 2));
    workers_squirrels = (int *)malloc(sizeof(int) * (size / 2));

    for (i = 0; i < size / 2; i++)
    {
      workers_cells[wc] = workers[i];
      wc++;
    }
    for (i = size / 2; i < size; i++)
    {
      workers_squirrels[ws] = workers[i];
      ws++;
    }
  }
  else
  {
    workers_cells = (int *)malloc(sizeof(int) * (size / 2) + 1);
    workers_squirrels = (int *)malloc(sizeof(int) * (size / 2));
    for (i = 0; i < size / 2 + 1; i++)
    {
      workers_cells[wc] = workers[i];
      wc++;
    }
    for (i = size / 2 + 1; i < size; i++)
    {
      workers_squirrels[ws] = workers[i];
      ws++;
    }
  }

  /***************************************** WORKERS INITIALIZATION ****************************************/
  int data[3];
  int squirrels_startID = 0, cells_startID = 0;
  int squirrels_endID = (_NUM_INIT_SQUIRRELS / ws);
  int cells_endID = (_NUM_CELLS / wc);
  int correctsplit = 1;

  /*Send the squirrels to the workers*/
  for (i = 0; i < ws; i++)
  {
    if (i == ws - 1 && (ws % 2) && ws >1) /* Split it correct the final ID should increment to one*/
    {
      squirrels_endID++;
    }
    data[0] = squirrels_startID;
    data[1] = squirrels_endID;
    data[2] = 0;  /*This indicates what actor type it the worker should handle (Squirrels)*/

    /*Assign the IDs of the actors to the registry and send them to the workers*/
    assign_registry(r, workers_squirrels[i], squirrels_startID, squirrels_endID,
                    0, 0, 0);

    MPI_Send(&data, 3, MPI_INT, workers_squirrels[i], _TAG_INITIAL, MPI_COMM_WORLD);
    if (_DEBUG)
      printf("Sending to rank : %d, the data %d %d %d\n", workers_squirrels[i], data[0], data[1], data[2]);
    squirrels_startID += (_NUM_INIT_SQUIRRELS / ws);
    squirrels_endID += (_NUM_INIT_SQUIRRELS / ws);
  }

  /*Send the cells to the workers*/
  for (i = 0; i < wc; i++)
  {
    if (i == wc-1 && (wc % 2)&& wc>1)
    {
      cells_endID++;
    }
    data[0] = cells_startID;
    data[1] = cells_endID;
    data[2] = 1; /* This indicates what actor type it the worker should handle (Cells)*/

    /*Assign the IDs of the actors to the registry and send them to the workers*/
    assign_registry(r, workers_cells[i], 0, 0,
                    cells_startID, cells_endID, 1);
    MPI_Send(&data, 3, MPI_INT, workers_cells[i], _TAG_INITIAL, MPI_COMM_WORLD);

    if (_DEBUG)
      printf("Sending to rank : %d, the data %d %d %d\n ", workers_cells[i], data[0], data[1], data[2]);

    cells_startID += _NUM_CELLS / wc;
    cells_endID += _NUM_CELLS / wc;
  }

  int success_assign = 0, success_all_assign = 0;
  MPI_Reduce(&success_assign, &success_all_assign, 1, MPI_FLOAT, MPI_SUM, _MASTER, MPI_COMM_WORLD);
  
  printf("[Master]Success spaniwng of actors %d %d \n", success_all_assign, size);
  print_register(*r);

}

void masterlives(Registry_cell *r, int workers_size)
{
  int masterStatus = 1;
  struct Clock clock = Clock.new(_MASTER, 0);

  while (masterStatus)
  {
    /* If the master receives any message from workers it 
    evaluates it */
    MPI_Status status;
    unsigned int retTime = time(0) + MONTH_IN_SEC;
    
    /* For one month, squirrels and cells can exchange messages*/
    while ( (time(0) < retTime) && masterStatus)
    {
      MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      /* If an actor wants access to the registry cell
      for grabbing anothers actor ID in order to communicate
    */
      if (status.MPI_TAG == _TAG_REGISTRY_CELL)
      {
        /* Receive the actor rank and sent back the ID that its looking for*/
        int ID_to_find;
        MPI_Recv(&ID_to_find, 1, MPI_INT, MPI_ANY_SOURCE, _TAG_REGISTRY_CELL, MPI_COMM_WORLD, &status);
        int rank = access_registry(r, 0, ID_to_find);
        MPI_Send(&rank, 1, MPI_INT, status.MPI_SOURCE, _TAG_REGISTRY_CELL, MPI_COMM_WORLD);
        if (_DEBUG)
        {
          printf("[Master] Received Cell %d\n", ID_to_find);
          printf("[Master] Sending to squirrel %d the cell ID:%d\n", rank, ID_to_find);
        }
      }
      /* Else if it because it is about the controling the process (stop, sleep ,etc)*/
      if (status.MPI_TAG == CONTROL_TAG)
      {
        masterStatus = receiving_handle();
      }
    }

    /* When the loop finishes , tick the clock by one month */
    if ((clock.timeline->ID <= _MAX_MONTHS_SIMULATION) && masterStatus)
      clock_work(r, workers_size, &clock);
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
