
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



static void send_msg_int (int x,int _rank, int _tag, MPI_Datatype mpi_type, MPI_Comm comm,struct Actor* this)
{
  printf("%s\t%d\n", __func__, x);
}

static void send_msg_char (char x, int _rank, int _tag, MPI_Datatype mpi_type, MPI_Comm comm,struct Actor* this)
{
  printf("%s\t%c\n", __func__, x);
}


/*Create the new Actor*/
static struct Actor new(int rank,int ID) {
	return (struct Actor){.rank=rank ,.getID = &getID, .ID=ID, .getRank = &getRank, .send_msg_char=&send_msg_char ,.send_msg_int=&send_msg_int  };
}

const struct ActorClass Actor={.new=&new};

/***********************************************************************************************/
