
#include "actor.h"
#include "cell.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ran2.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int getCellIDFromPosition(float x, float y,struct Cell *this){
    //return((int)(x*4)+4*(int)(y*4));
    return this->actor.ID;
}


static struct Cell new(int rank,int ID, float pos_x,float pos_y) {
	  struct Cell cell={.pos_x= pos_x,.pos_y= pos_y, .getCellIDFromPosition=&getCellIDFromPosition};
      cell.actor = Actor.new(rank, ID);
      cell.c = 0;
      return cell; 
}

const struct CellClass Cell={.new=&new};
/**
 * Returns the id of the cell from its x and y coordinates.
 */


  void cells_work(struct Cell * this, int rank, _registry_cell *registry)
  {
    int forever = 0;
    while (!forever)
    {
      int seed = 0;
      int influx;
      int pop;
      int f;
      MPI_Status status;
      /*Check if there is message from any other actor and act accordiclty*/
      MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &f, &status);
      int s_health;
      if (f)
      {
        if (if_squirrels_msg(status))
        {

          MPI_Request request_inflix;
          MPI_Request request_pop;
          /*Send the inlfux and population values to the incoming squirrel */
          MPI_Isend(&(this->influx), 1, MPI_FLOAT, status.MPI_SOURCE, _TAG_SQUIRRELS, MPI_COMM_WORLD, request_inflix);
          MPI_Isend(&(this->pop), 1, MPI_FLOAT, status.MPI_SOURCE, _TAG_SQUIRRELS, MPI_COMM_WORLD, request_pop);
          /*Receive health value from the squierrel*/
          MPI_Recv(&s_health, 1, MPI_INT, MPI_ANY_SOURCE, _TAG_SQUIRRELS, MPI_COMM_WORLD, &status);

          if (s_health)
            this->squirrels_day->squirrels_healthy++;
          else
            this->squirrels_day->squirrels_unhealthy++;
        }
        else if (if_clock_msg(status))
        {
          /* Perfome update of the cell influx and population values*/
          //int * health_sq =  (int *)malloc(sizeof(int));
          /*This means that a day passed and the squirrels have to be updated*/
          update_cell_day(this);
          int day;
          MPI_Recv(&day, 1, MPI_INT, MPI_ANY_SOURCE, _TAG_CLOCK, MPI_COMM_WORLD, &status);

          if (day > _DAYS_POP)
          {
            chronicle(&this->squirrels_day, 0, 0);
            erase_day(this->squirrels_day);
          }
          else
          {
            chronicle(&this->squirrels_day, 0, 0);
          }
        }
        forever = shouldworkerstop();
      }
    }
  }


