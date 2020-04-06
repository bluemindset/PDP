
#include "mpi.h"
#include "actor.h"
#include "stdio.h"

#define send_msg(x, mpi_rank, mpi_tag, mpi_type, this) _Generic((x,mpi_rank, mpi_tag, mpi_type,this), int: send_msg_int, char: send_msg_char)(x,mpi_rank, mpi_tag, mpi_typ,this);

static double getRank(struct Actor *this){
      return (this->rank); 
}

static int getID(struct  Actor *this)
{
      return (this->ID);
}


/*Create the new Actor*/
static struct Actor new(int rank,int ID) {
	return (struct Actor){.rank=rank , .ID=ID, .getID = &getID, .getRank = &getRank  };
}

const struct ActorClass Actor={.new=&new};

/***********************************************************************************************/
