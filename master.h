

#ifndef _MASTER_H
#define _MASTER_H



void master_send_instructions(int num_cells, int num_squirrels, int size,struct Registry_cell **registry,int * workers );

void masterlives(Registry_cell * r);

void startworkers(int num_workers, int *workers);

void squirrel_life(struct Squirrel *squirrel, int influx, int pop);


#endif