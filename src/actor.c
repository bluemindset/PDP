
/**
 * @Author: B159973
 * @Date:	10/4/2019
 * @Course: Parallel Design Patterns - 2020
 * @University of Edinburgh
*/
#include "mpi.h"
#include "actor.h"
#include "stdio.h"

static double getRank(struct Actor *this){
      return (this->rank); 
}

static int getID(struct  Actor *this)
{
      return (this->ID);
}

static struct Actor new(int rank,int ID) {
	return (struct Actor){.rank=rank , .ID=ID, .getID = &getID, .getRank = &getRank  };
}

const struct ActorClass Actor={.new=&new};

/***********************************************************************************************/
