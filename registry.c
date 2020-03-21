/*************************LIBRARIES**********************************/
/********************************************************************/
#include "mpi.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
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
#include <time.h>
/********************************************************************/

void assign_registry(struct Registry_cell **r, int worker_rank, struct Squirrel * squirrels, struct Cell *cells)
{
  Registry_cell *cur = malloc(sizeof(struct Registry_cell));
  /*Assign data to the registry cell*/
  cur->squirrels = squirrels;
  cur->cells = cells;
  cur->rank = worker_rank;
  /*Link to next registry cell*/
  cur->next = *r;
  *r = cur;
}

void print_register(struct Registry_cell *head)
{
  struct Registry_cell *current = head;
  int count = 0;
  int i = 0 ;
  while (current != NULL)
  {
    while ( (current->cells) + i != NULL)
    {
      printf("CELL ID : %d , RANK ID : %d \n", ((current->cells) + i)->actor.ID,current->rank);
      i++;
    }
    current = current->next;
  }
}

/*Get the Cell Rank*/
int access_registry(int cell_ID, Registry_cell *registry)
{
  int i = 0;
  struct Registry_cell *current = registry;

  while (current != NULL)
  {
    while ((current->cells) + i != NULL)
    {
      if (((current->cells) + i)->actor.ID == cell_ID)
      {
        return ((current->cells) + i)->actor.rank;
      }
      i++;
    }
    current = current->next;
  }
  return current->rank;
}
