

#ifndef _WORKER_H
#define _WORKER_H


void worker(int rank,struct Registry_cell *registry,int size);

void print_pos(struct Squirrel *this);

int getCellFromPosition(float x, float y);

int update_cell_day(struct Cell *this);

int if_squirrels_msg(MPI_Status status);

int if_clock_msg(MPI_Status status);

/*************************ACTOR SPAWN********************************/
/********************************************************************/
struct Cell *spawnCells(int num_cells, int start_id, int rank);
struct Squirrel *spawnSquirrels(int num_squirrels,int rank);

/*************************DAYS MANIPULATION***************************/
/********************************************************************/
void erase_day(struct Day *lastday);
int length(struct Day *head);
void chronicle(struct Day **lastday, int healthy_s, int unhealthy_s);


#endif