/**
 * @Author: B159973
 * @Date:	10/4/2019
 * @Course: Parallel Design Patterns - 2020
 * @University of Edinburgh
*/
#ifndef _CLOCK_H
#define _CLOCK_H

/***************************************************************************//**
 * Month structure that stores many information about each month of the simulation
 * @squirrels_healthy:      The number of healthy squirrels of the month simulations.
 * @squirrels_unhealthy:    The number of the unhealthy squirrels month simulations.
 * @nextmonth:              Next month of the simulation.
 ******************************************************************************/
typedef struct month {
    int * squirrels_healthy;
    int * squirrels_unhealthy;
    struct month * nextmonth;
    int ID;
}month;

/***************************************************************************//**
 * The Clock Actor
 * @actor:                  The actor class inside the Clock Actor.
 * @timeline:               The list of months of the timeline.(month structure)
 ******************************************************************************/
struct Clock {
    struct Actor actor; 
    month * timeline;
};

/***************************************************************************//**
 * Constructor of the Cell Class. 
 ******************************************************************************/
extern const struct ClockClass{
    struct Clock (*new)(int rank, int ID);
} Clock;

/***************************************************************************//**
 * The clock works, by communicating with all the cells with non blocking 
 * sending and receives with the current day. If all the cells respond, then 
 * only the clock continues to let Master handles other messages. 
 * After that it updates the clock by one day and append
 * the timeline. 
 * 
 * @param r:            The registry that is accessed by Master
 * @param workers_size: The size of the workers. 
 * @param clock:        The clock Actor. 
 * @return:		        Nothing
 ******************************************************************************/
void clock_work(Registry_cell *r, int workers_size, struct Clock *clock);

/***************************************************************************//**
 * Appends the month to the clock timeline. 
 * 
 * @param lastmonth:    The latest month to append into the timeline
 * @param healthy_s:    The number of healthy and unhealhty squirrels of each cell. 
 * @param unhealthy_s:  The number of unhealthy squirrels of each cell. 
 * @return:		        Nothing
 ******************************************************************************/
void chronicle(struct month **lastmonth, int *healthy_s, int *unhealthy_s);

/***************************************************************************//**
 * A delay implemented using milliseconds. 
 * 
 * @param msec:         Number of milliseconds to delay.
 * @return:		        Nothing
 ******************************************************************************/
int msleep(long msec);



#endif