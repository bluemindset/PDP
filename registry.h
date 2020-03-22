
#ifndef _REGISTRY_H
#define _REGISTRY_H

typedef struct Registry_cell
{
  /*Process rank that have these actors*/
  int rank;
  /*Actors*/
  int  * actors_ID;
  int  type_actor;
  /********/
  int num_s;
  int num_c;
  /*Next cell*/
  struct Registry_cell *next;
} Registry_cell;

void assign_registry(struct Registry_cell **r, int worker_rank, int start_s, int end_s,
        int start_c, int end_c,int actor_type);
int access_registry(Registry_cell *registry, int actor,int ID);
void print_register(struct Registry_cell *head);

#endif