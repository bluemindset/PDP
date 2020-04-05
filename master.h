

#ifndef _MASTER_H
#define _MASTER_H



void master_send_instructions( int size, struct Registry_cell **r, int *workers);

void masterlives(Registry_cell *r, int workers_size);

void startworkers(int num_workers, int *workers);



#endif