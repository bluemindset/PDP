/**
 * @Author: B159973
 * @Date:	10/4/2019
 * @Course: Parallel Design Patterns - 2020
 * @University of Edinburgh
*/
#ifndef _SQUIRREL_H
#define _SQUIRREL_H
#include "mpi.h"

 /***************************************************************************//**
 * Cell structure represents a single Cell Actor 
 * @actor:      The Actor class inside the Cell class
 * @ID:		    The identification number of the cell acotr.
 * @pos_x:      The current position in the map on the x-axis. (NOT USED)-Deprecated because of getCellPosition()
 * @pos_y:      The current position in the map on the y-axis. (NOT USED)-Deprecated because of getCellPosition()
 * @influx:	    The values of the influx of cells that the Squirrel stepped in.
 * @pop:        The values of the population of cells that the Squirrel stepped in.
 * @avg_influx: The average influx value of the squirrel that uses to update itself.
 * @avg_pop:    The average population value of the squirrel uses to update itself.
 * @last_steps: The number of last steps of the Squirrels. Initially is 50. 
 ******************************************************************************/
struct Squirrel {
    struct Actor actor; 
    int steps;
    int seed; 
    int health; 
    float pos_x;
    float pos_y;
    int influx[50];
    int pop[50];
    double avg_influx;
    double avg_pop;
    int last_steps;
    void (*update_avgs)(int influx, int pop, struct Squirrel *this);
};

/***************************************************************************//**
 * Constructor of the Squirrel Class. 
 ******************************************************************************/
extern const struct SquirrelClass{
    struct Squirrel (*new)(int rank, int ID, int steps,int seed,float pos_x,float pos_y);
} Squirrel;

/***************************************************************************//**
 * This is the basic communication function that is executed for each squirrel inside the worker's
 * loop. The squirrel first sends a message to the worker, receives the cell rank and sends back 
 * to the rank its health. After that it wait to the r request to be complete which is the receive
 * influx and population from the cell.   
 * @param squirrel:  A pointer to the Squirrel structure.
 * @param rank:      Rank of the worker that the Squirrel is handled by.
 * @param data:      The data that are received from the Cell(influx and population)
 * @param r:         The MPI request of the cell receive message.
 * @param alive:     Flag that keeps the worker alive
 * @return:		     Nothing
 ******************************************************************************/
void squirrels_comm(struct Squirrel * squirrel, int rank, int  data[2],int * alive,MPI_Request * r);

/***************************************************************************//**
 Squirrel Routine:  1. Update the inner average of the squirrel (inlfux and population) 
 *  2. Calculate the propability to catch a disease. 
 *  3.Caclulate propoability to die and kill the squirrel. 4. Caclulate propability to born 
 * and give life to a newborn. 
 * @param squirrel:     A pointer to the Squirrel structure.
 * @param influx:       The influx received from the Cell that the Squirrel stepped in.
 * @param pop:          The population received from the Cell that the Squirrel stepped in.
 * @param num_squirrel: The number of the squirrels of the worker.
 * @param rank:         Rank of the worker that the Squirrel is handled by.
 * @param dead:         The number of the dead squirrels.
 * @return:		        1 if a squirrel is born else 0.
 ******************************************************************************/
int squirrel_life(struct Squirrel *squirrel, int influx, int pop, int *num_squirrels,int rank,int *dead);

/***************************************************************************//**
 * Update the averages of the Squirrel based on the last 50 steps. 
 * @param influx:    The influx received from the Cell that the Squirrel stepped in.
 * @param pop:       The population received from the Cell that the Squirrel stepped in.
 * @param squirrel:  A pointer to the Squirrel structure.
 * @return:		     Nothing
 ******************************************************************************/
static void update_avgs(int influx, int pop, struct Squirrel *squirrel);

/***************************************************************************//**
 * Store the squirrel ID in th statistics array according to its health status 
 * @param recvID:                   The ID of the squirrel that will be stored 
 * @param squirrels_IDs_healthy:    The list of IDs of healthy squirrels
 * @param squirrels_IDs_unhealthy:  The list of IDs of unhealthy squirrels.
 * @param health:                   The health status of the squirrel.
 * @return:                 		 Nothing
 ******************************************************************************/
void store_squirrel(int recvID, int *squirrels_IDs_healthy, int *squirrels_IDs_unhealthy,int health);

/***************************************************************************//**
 * Initialize the IDs of the squirrels. Both of the list are initialized to -1.
 * @param squirrels_IDs_healthy:    The list of IDs of healthy squirrels
 * @param squirrels_IDs_unhealthy:  The list of IDs of unhealthy squirrels.
 * @return:		                    Nothing
 ******************************************************************************/
void init_squirrel_stats (int *squirrels_IDs_healthy, int *squirrels_IDs_unhealthy);
/***************************************************************************//**
 * Print statistics of the squirrels on that current month of the simulation. 
 * 
 * @param squirrels_IDs_healthy:    The list of IDs of healthy squirrels
 * @param squirrels_IDs_unhealthy:  The list of IDs of unhealthy squirrels.
 * @param month:                    The current month of the simulation
 * @param rank:                    The rank of the worker that will print the stats.
 * @return:		                    Nothing
 ******************************************************************************/
void print_stat_squirrels(int *squirrels_IDs_healthy, int *squirrels_IDs_unhealthy, int month, int rank);

/***************************************************************************//**
 * Simulates the step of a squirrel. You can call this with the arguments (0,0,&x,&y,&state)
 * to determine a random initial starting point.
 * x_new and y_new are the new x and y coordinates, state is used in the random number
 * generator and is also modified.
 * x_new can point to x, and y_new can point to y 
 * Funtion remains unchanged. 
 * @return:		                    Nothing
 * ******************************************************************************/
void squirrelStep(float, float, float *, float *, long *);

/***************************************************************************//**
 * Determines whether a squirrel will give birth or not based upon the average population 
 * and a random seed which is modified. You can enclose this function call in an 
 * if statement if that is useful.
 * Funtion remains unchanged. 
 * @return:		                    1 for giving birth, 0 not giving birth.
 * ******************************************************************************/
int willGiveBirth(float, long *);

/***************************************************************************//**
 * Determines whether a squirrel will catch the disease or not based upon the average 
 * infection level and a random seed which is modified. You can enclose this function
 * call in an if statement if that is useful.
 * @return:		                    1 Cathces disease else it is still healhty
 * ******************************************************************************/
int willCatchDisease(float, long *);

/***************************************************************************//**
 *  Determines if a squirrel will die or not. The state is used in the random number
 *  generation and is modified. You can enclose this function call in an
 *  if statement if that is useful.
 * 
 * @return:		                    1 is Dead, 0 is alive 
 * ******************************************************************************/
int willDie(long *);

/***************************************************************************//**
 * Simple selection sort. Helper function for debbuging code
 ******************************************************************************/
void selection_sort(int * a);
/***************************************************************************//**
 * Finding Maximum. Helper function for debbuging code
 ******************************************************************************/
int  find_max(int * a, int high);
/***************************************************************************//**
 * Swapping variables. Helper function for debbuging code
 ******************************************************************************/
void swap(int * a, int p1, int p2);

#endif