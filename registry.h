
#ifndef _REGISTRY_H
#define __REGISTRY_H

  typedef struct _registry_cell
{
  int rank;
  float pos_x;
  float pos_y;
  int ID;
  _registry_cell *next;
} _registry_cell;


#endif