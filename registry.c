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

void assign_registry(struct Registry_cell **r, int worker_rank, int start_s, int end_s,
                     int start_c, int end_c)
{
  int i = 0, k = 0;
  Registry_cell *cur = malloc(sizeof(struct Registry_cell));

  /*Assign data to the registry cell*/

  cur->squirrels_ID = (int *)malloc(sizeof(int) * (end_s - start_s));
  cur->cells_ID = (int *)malloc(sizeof(int) * (end_c - start_c));
  cur->num_c = end_c - start_c;
  cur->num_s = end_s - start_s;
  printf("I am in print %d %d %d %d :\n", start_c, start_s, end_c, end_s);

  for (i = start_s; i < end_s; i++)
  {
    cur->squirrels_ID[k] = i;
    k++;
  }
  k = 0;
  for (i = start_c; i < end_c; i++)
  {
    cur->cells_ID[k] = i;
    k++;
  }
  cur->rank = worker_rank;
  /*Link to next registry cell*/
  cur->next = *r;
  *r = cur;
}

void print_register(struct Registry_cell *head)
{
  struct Registry_cell *current = head;
  int count = 0;
  int i = 0;
  while (current != NULL)
  {
    printf("I am in\n");
    while (current->num_c > i)
    {
      printf("I am in print C%d:\n", current->cells_ID[i]);
      i++;
    }
    i = 0;
    while (current->num_s > i)
    {
      printf("I am in print S%d:\n", current->squirrels_ID[i]);
      i++;
    }
    i = 0;
    current = current->next;
  }
}

/*Get the Cell Rank
  int actor -> Cell:  0
            -> Squirrel: 1  
*/

int access_registry(int cell_ID, Registry_cell *registry, int actor)
{
  int i = 0;
  struct Registry_cell *current = registry;

  while (current != NULL)
  {
    switch (actor)
    {   
    case 0:
      while (current->num_c > i){
        if(cell_ID == current->cells_ID[i])
            return current->rank; 
        i++;
      }
      break;

    case 1:
      while (current->num_s > i){
        if(cell_ID == current->squirrels_ID[i])
            return current->rank; 
        i++;
      }
      break;
    }
    i = 0 ; 
    current = current->next;
  }
  /* If not successful then return not found*/
  return -1;
}

  // while (current != NULL)
  // {
  //   while ((current->cells) + i != NULL)
  //   {
  //     if (((current->cells) + i)->actor.ID == cell_ID)
  //     {
  //       return ((current->cells) + i)->actor.rank;
  //     }
  //     i++;
  //   }
  //   current = current->next;
  // }

