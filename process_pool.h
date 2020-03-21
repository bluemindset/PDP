#ifndef _POOL_H
#define _POOL_H

#include <stdlib.h>
#include <stdio.h>
#include "mpi.h"


#define CONTROL_TAG 100
#define PID_TAG 200
#define MSG_ITEMS 3
#define _MASTER 0


static MPI_Datatype Message_Control_DataType;

static int *UEs_state = NULL;
static int processes_waiting_to_start;
static struct Message_Command incoming_msg;
static MPI_Request req_poll = MPI_REQUEST_NULL;
static int recv_handler_worker();
void send_command(int,int,int);
int startWorkerProcess();
int receiving_handle();
int create_pool();
int should_terminate_worker();
int workerSleep();
void terminate_pool();
void  init_UEs(int size);
int startAwaitingTask(int processes_waiting_to_start,int incoming_rank);
enum _Command{
    _KILL = 0,
    _STOP = 1,
    _START = 2,
    _SLEEP = 3, 
    _COMPLETE = 4,
    _IDLE = 5,
};

typedef struct Message_Command{
        enum _Command com;
        int rank;
        int context;
} Message_Command;


#endif

//Create a command data type


