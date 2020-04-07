/**
 * @Author: B159973
 * @Date:	10/4/2019
 * @Course: Parallel Design Patterns - 2020
 * @University of Edinburgh
*/
#ifndef CELL_H_
#define CELL_H_

 /***************************************************************************//**
 * Cell structure represents a single Cell Actor 
 * @actor:   The Actor class inside the Cell class
 * @ID:		 The identification number of the cell acotr.
 * @pos_x:   The position on the x-axis of the cell. (NOT USED)-Deprecated because of getCellPosition()
 * @pos_y:   The position on the y-axis of the cell. (NOT USED)-Deprecated because of getCellPosition()
 * @influx:	 The influx value of the cell.
 * @pop:     The population value of the cell.
 ******************************************************************************/
struct Cell {
    struct Actor actor; 
    int ID;
    float pos_x;
    float pos_y;
    int influx;
    int pop;
    int (*getCellIDFromPosition) (float, float, struct Cell *);
};

/***************************************************************************//**
 * Constructor of the Cell Class. 
 ******************************************************************************/
extern const struct CellClass{
    struct Cell (*new)(int rank, int ID);
} Cell;

/***************************************************************************//**
 * Update the statistics of the Cell by fetching the healthy and unhealthy squirrels
 * for the last 2 and 3 months. Virus can only live without a host for two months
 * in the environment. 
 * 
 * @param cell:  A pointer to the Cell structure.
 * @param month: The current month that Cell is being updated.
 * @param rank:  The rank of the worker that Cell is handled by.
 * @param stats: The stat table that holds the number of the healhty and unhealthy squirrels
 *               for each month.
 * @param cellID:The cell ID on the particular worker in dynamic memory.
 * @return:		 Nothing
 ******************************************************************************/
void update_cell(struct Cell *cell, int month, int rank,int * stats,int cellID);



#endif