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
#include "registry.h"
#include "cell.h"
#include "clock.h"
#include "squirrel.h"
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
                     int start_c, int end_c, int actor_type)
{
  int i = 0, k = 0;
  Registry_cell *cur = malloc(sizeof(struct Registry_cell));

  /*Assign data to the registry cell*/
  cur->rank = worker_rank;
  /*Assign the actors ID to the registry cell*/
  switch (actor_type)
  {
  case 0:
    cur->actors_ID = (int *)malloc(sizeof(int) * (end_s - start_s));
    cur->num_s = end_s - start_s;
    cur->num_c =0;
    for (i = start_s; i < end_s; i++)
    {
      cur->actors_ID[k] = i;
      k++;
    }
    cur->type_actor = 0;
    break;
  case 1:
    cur->actors_ID = (int *)malloc(sizeof(int) * (end_c - start_c));
    cur->num_c = end_c - start_c;
    cur->num_s =0;
    for (i = start_c; i < end_c; i++)
    {
      cur->actors_ID[k] = i;
      k++;
    }
    cur->type_actor = 1;
    break;
  }
  
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
      while (current->num_c > i)
      {
        printf("R%d - Cell ID%d\n", current->rank, current->actors_ID[i]);
        i++;
      }
    i =0;
      while (current->num_s > i)
      {
        printf("R%d - Squirrel ID%d\n", current->rank, current->actors_ID[i]);
        i++;
      }

    
    current = current->next;
  }
}

/*Get the Cell Rank
  int actor -> Cell:  0
            -> Squirrel: 1  
*/
int access_registry(Registry_cell *registry, int actor, int ID)
{
  int i = 0;
  struct Registry_cell *current = registry;

  while (current != NULL)
  {
    switch (actor)
    {
    case 0:
      while (current->num_c > i)
      {
        if (ID == current->actors_ID[i])
          return current->rank;
        i++;
      }
      break;
    case 1:
      while (current->num_s > i)
      {
        if (ID == current->actors_ID[i])
          return current->rank;
        i++;
      }
      break;
    }
    i = 0;
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
