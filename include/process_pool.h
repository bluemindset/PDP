/**
 * @Author: B159973
 * @Date:	10/4/2019
 * @Course: Parallel Design Patterns - 2020
 * @University of Edinburgh
*/

#ifndef _POOL_H
#define _POOL_H

#include <stdlib.h>
#include <stdio.h>
#include "mpi.h"

#define CONTROL_TAG 100
#define PID_TAG 200
#define MSG_ITEMS 3
#define _MASTER 0

/***************************************************************************//**
 * Enumaration of Commands
 ******************************************************************************/
enum _Command{
    _KILL = 0,
    _STOP = 1,
    _START = 2,
    _SLEEP = 3, 
    _COMPLETE = 4,
    _RUN = 5,
};
/***************************************************************************//**
 * Control message structure.
 * @com:             Command Enumaration
 * @rank:            Rank to deliver the message
 * @context:         Context of the message
 ******************************************************************************/
typedef struct Message_Command{
        enum _Command com;
        int rank;
        int context;
} Message_Command;

/***************************************************************************//**
 * MPI control message datat type
 ******************************************************************************/
static MPI_Datatype Message_Control_DataType;

/***************************************************************************//**
 * Units of Execution state
 ******************************************************************************/
static int *UEs_state = NULL;

/***************************************************************************//**
 * Processes waiting to start
 ******************************************************************************/
static int processes_waiting_to_start;

/***************************************************************************//**
 * Message Command incoming message.
 ******************************************************************************/
static struct Message_Command incoming_msg;

/***************************************************************************//**
 * MPI request that the rank is chechking to see if it has a message
 ******************************************************************************/
static MPI_Request req_poll = MPI_REQUEST_NULL;

/***************************************************************************//**
 * Handler receiver of the worker.
 * @return: 0 on stop message, 1 on run message 
 ******************************************************************************/
static int recv_handler_worker();

/***************************************************************************//**
 * Send command of a control message
 * @return: Nothing 
 ******************************************************************************/
void send_command(int,int,int);

/***************************************************************************//**
 * Start the worker process. Same as in class.
 * @return:  Worker rank or process to start
 ******************************************************************************/
int startWorkerProcess();

/***************************************************************************//**
 * Receive hanlder of the Master.
 *@return: 0 on complete message, 1 on Run message 
 ******************************************************************************/
int receiving_handle();
/***************************************************************************//**
 * Create the process pool.
 * @return: 2 for Master or 1 for Worker 
 ******************************************************************************/
int create_pool();

/***************************************************************************//**
 * The worker check its message inbox to see if it can finish or not. 
 * @ask:    Ask the Master to stop 
 * @return:  0 on if there is a Stop incoming message, 1 on Run incoming message 
 ******************************************************************************/
int should_terminate_worker(int ask);

/***************************************************************************//**
 * Terminate the process pool.
 * @return: Nothing
 ******************************************************************************/
void terminate_pool();

/***************************************************************************//**
 * Initialize the Execution Units
 * @return: Nothing 
 ******************************************************************************/
void  init_UEs(int size);

/***************************************************************************//**
 * Create the process pool.
 * @processes_waiting_to_start:   Processes waiting to start
 * @UE_to_start:   Unit of execuiton to start
 * @return: The proawaitingProcessMPIRank
 ******************************************************************************/
int startAwaitingTask(int processes_waiting_to_start,int UE_to_start);

/***************************************************************************//**
 * Worker can ask to sleep by sending a sleep command to Master
 * @return: 0 on stop message, 1 on run message 
 ******************************************************************************/
int workerSleep();
#endif

