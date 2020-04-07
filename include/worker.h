/**
 * @Author: B159973
 * @Date:	10/4/2019
 * @Course: Parallel Design Patterns - 2020
 * @University of Edinburgh
*/
#ifndef _WORKER_H
#define _WORKER_H

/***************************************************************************//**
 * The basic function of the worker. The worker will handle either a group of squirrels or 
 * cells. The function runs inside a while loop, is ends only if the Master commands
 * it or if it ask by itself.
 * @rank:        The rank of the worker
 * @registry:    Size of the workers. 
 * @return:      Nothing
 ******************************************************************************/
void worker(int rank,int size);

/***************************************************************************//**
 * Insert position x and y axis and get the cell ID.
 * @x:           Position in x axis.
 * @y:           Position in y axis. 
 * @return:      Nothing
 ******************************************************************************/
int getCellFromPosition(float x, float y);

/***********************************ACTOR SPAWN********************************/

/***************************************************************************//**
 * Spawn the Cell actors, number of cells starting from startID and end in EndID.
 * 
 * @startID:     Start ID of the first cell to spawn.
 * @endID:       End ID of the last cell to spawn.
 * @return:      Nothing
 ******************************************************************************/
struct Cell *spawnCells(int startID, int endID, int rank);

/***************************************************************************//**
 * Spawn the squirrels actors, number of squirrels starting from startID and end in EndID.
 * 
 * @startID:     Start ID of the first squirrels to spawn.
 * @endID:       End ID of the last squirrels to spawn.
 * @return:      Nothing
 ******************************************************************************/
struct Squirrel *spawnSquirrels(int startID, int endID, int rank,int unhealthy);

/***************************************************************************//**
 * End the program if there more than 200 squirrels.
 * 
 * @num_squirrels:    Number of squirrels.
 * @return:           Nothing
 ******************************************************************************/
int max_threshold(int num_squirrels);

/***************************************************************************//**
 * Erase the last month inside the timeline.
 * @head:        Last month of chronicle.
 * @return:      Nothing
 ******************************************************************************/
void erase_month(struct month *head);

/***************************************************************************//**
 * Get the number of months. Not used (only for delevoping purposes)
 * @head:        Last month of chronicle.
 * @return:      Nothing
 ******************************************************************************/
int length(struct month *head);


#endif