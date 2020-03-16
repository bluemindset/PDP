#ifndef _HELPER_FUNCTIONS_H
#define _HELPER_FUNCTIONS_H

#include "mpi.h"

#endif
 
 
 struct Actor{
    int rank;
    int ID;
    int (*getRank)(struct Actor *this);
    int (*getID)(struct Actor *this);
    void (*send_msg_char)(char x,int _rank, int _tag, MPI_Datatype mpi_type, MPI_Comm comm,struct Actor* this);
    void (*send_msg_int)(int x,int _rank, int _tag, MPI_Datatype mpi_type, MPI_Comm comm,struct Actor* this);
};

/*Constructor*/
extern const struct ActorClass{
    struct Actor (*new)(int rank,int ID);
} Actor;

//static double sum(struct  Actor *this);