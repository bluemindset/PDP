/**
 * @Author: B159973
 * @Date:	10/4/2019
 * @Course: Parallel Design Patterns - 2020
 * @University of Edinburgh
*/
#ifndef _MASTER_H
#define _MASTER_H

/***************************************************************************//**
 * Update the statistics of the Cell by fetching the healthy and unhealthy squirrels
 * for the last 2 and 3 months. Virus can only live without a host for two months
 * in the environment. 
 * 
 * @param size:     A pointer to the Cell structure.
 * @param r:        The current month that Cell is being updated.
 * @param workers:  The rank of the worker that Cell is handled by.
 * @param stats:    The stat table that holds the number of the healhty and unhealthy squirrels
 *               for each month.
 * @param cellID:   The cell ID on the particular worker in dynamic memory.
 * @return:		 Nothing
 ******************************************************************************/
void master_send_instructions( int size, struct Registry_cell **r, int *workers);
/***************************************************************************//**
 * The master lives and is answering messages that are received from many cells
 * When a day is passed, it stops receiving messages and goes and executes clock_work()
 * 
 * @param r:            The registry as described in the report
 * @param worker_size:  The size of the workers.
 * @return:		        Nothing
 ******************************************************************************/
void masterlives(Registry_cell *r, int workers_size);
/***************************************************************************//**
 * The master starts the workers
 * 
 * @param workers:      The ranks of the workers
 * @param worker_size:  The size of the workers.
 * @return:		        Nothing
 ******************************************************************************/
void startworkers(int num_workers, int *workers);



#endif