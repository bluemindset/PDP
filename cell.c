
/*************************LIBRARIES**********************************/
/********************************************************************/
#include "mpi.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
/********************************************************************/
/***************************ACTORS***********************************/
/********************************************************************/
#include "actor.h"
#include "cell.h"
#include "clock.h"
#include "squirrel.h"
#include "registry.h"
/*************************PROCESS************************************/
/********************************************************************/
#include "process_pool.h"
#include "master.h"
#include "worker.h"

#include "main.h"
#include "ran2.h"
/********************************************************************/

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// static int getCellIDFromPosition(float x, float y,struct Cell *this){
//     //return((int)(x*4)+4*(int)(y*4));
//     return this->actor.ID;  .getCellIDFromPosition=&getCellIDFromPosition
// }

static struct Cell new(int rank,int ID, float pos_x,float pos_y) {
	  struct Cell cell={.pos_x= pos_x,.pos_y= pos_y};
        cell.actor = Actor.new(rank, ID);
    return cell; 
}


const struct CellClass Cell = {.new = &new};
/**
 * Returns the id of the cell from its x and y coordinates.
 */

