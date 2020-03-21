
#ifndef _REGISTRY_H
#define _REGISTRY_H

typedef struct Registry_cell
{
  /*Process rank that have these actors*/
  int rank;
  /*Actors*/
  struct  Cell * cells;
  struct  Squirrel * squirrels;
  /*Next cell*/
  struct Registry_cell *next;
} Registry_cell;

void assign_registry(struct Registry_cell ** r, int worker_rank, struct Squirrel * squirrels, struct Cell * cells );
int access_registry(int cell_ID, Registry_cell * registry);

#endif