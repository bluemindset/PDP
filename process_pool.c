#include <stdlib.h>
#include <stdio.h>
#include "process_pool.h"
#include "mpi.h"

static int rank;
static int UEs;

static void handling_exit(char *message)
{
    fprintf(stderr, "%4d: [ProcessPool] %s\n", rank, message);
    MPI_Abort(MPI_COMM_WORLD, 1);
}

void create_type()
{
    struct Message_Command control_message;
    MPI_Aint startAddress, rankAddress;

    MPI_Address(&control_message, &startAddress);
    MPI_Address(&control_message.rank, &rankAddress);

    MPI_Datatype types[3] = {MPI_CHAR, MPI_INT, MPI_INT};
    const MPI_Aint offsets[MSG_ITEMS] = {0, rankAddress - startAddress, sizeof(int)};
    int blocks[MSG_ITEMS] = {1, 1, 1};

    MPI_Type_create_struct(MSG_ITEMS, blocks, offsets, types, &Message_Control_DataType);

    if (MPI_Type_commit(&Message_Control_DataType) != MPI_SUCCESS)
        handling_exit("Not succesuful on creating data type for handling messages!");
}

void send_command(int code, int receiver, int context)
{
    Message_Command command;
    command.com = code;
    command.rank = _MASTER;
    command.context = context;
    MPI_Send(&command, 1, Message_Control_DataType, receiver, CONTROL_TAG, MPI_COMM_WORLD);
}
// void heartbeat()
// {
//     // /* Master periodically sent messages to workers to check if they are alive*/

//     // int MPI_Isend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag,
//     //           MPI_Comm comm, MPI_Request * request)
//     int alive = 1;
//     MPI_ISend(&alive, 1, MPI_INT, worker, PP);
// }

int check_resources()
{
    if (UEs < 2)
        handling_exit("Not enought resources");
}

void init_UEs(int size)
{
    int i;
    for (i = 0; i < size - 1; i++)
        UEs_state[i] = 0;
}

/* Initialize the process pool*/
int create_pool()
{
    create_type();
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &UEs);

    /*Master has to create the units of executions*/
    if (rank == _MASTER)
    {
        check_resources();

        /*Initialize them and allocate them*/
        UEs_state = (int *)malloc(UEs * sizeof(int));
        init_UEs(UEs);
        /*Set processes to zero*/
        processes_waiting_to_start = 0;
        return 2;
    }
    else
    { /*If the rank is worker then wait for a message
        * Master will have to issue an IDLE command in order to wake up the worker in the future
        */
        MPI_Recv(&incoming_msg, 1, Message_Control_DataType, 0, CONTROL_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        return recv_handler_worker();
    }
}

void terminate_pool()
{
    if (rank == _MASTER)
    {
        /* If there is not UEs then terminate the state*/

        if (UEs_state != NULL)
            free(UEs_state);
        int i;
        for (i = 0; i < UEs - 1; i++)
        {
            send_command(_STOP, i + 1, 0);
        }
        /* Barrier the termination and free the datatype*/
    }
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Type_free(&Message_Control_DataType);
}

/*Wait forever and handle the receives  (Master Poll)*/
int receiving_handle()
{
    MPI_Status status;
    MPI_Recv(&incoming_msg, 1, Message_Control_DataType, MPI_ANY_SOURCE, CONTROL_TAG, MPI_COMM_WORLD, &status);
    int com = incoming_msg.com;
    int incoming_rank = status.MPI_SOURCE;
    int waiting_processes;
    /*Get the incoming message command*/
    switch (com)
    {
    case _KILL:
        /*Set the string of the specific position of the MPI rank state to 0(dead).*/
        UEs_state[incoming_rank - 1] = 0;
        break;
    case _STOP:
        /*TO DO:*/
        break;
    case _START:
        /*Increament for waiting processes.*/
        processes_waiting_to_start++; /*Parent is the incoming rank*/
        int r = startAwaitingTask(processes_waiting_to_start, incoming_rank);
        MPI_Send(&r, 1, MPI_INT, incoming_rank, PID_TAG, MPI_COMM_WORLD);
        /*If sleeping set the state to 0 */
        break;
    case _SLEEP:
        UEs_state[incoming_rank - 1] = 0;
        /*If run is completed then increament */
        break;
    case _COMPLETE:
        return 0;
        //AwaitProcess
        break;
    case _IDLE:
        //handling_exit("1111Worker process fault");
        break;
    default:
        handling_exit("Worker process fault");
    }
    return 1;
}

int startWorkerProcess()
{
    if (rank == 0)
    {
        processes_waiting_to_start++;
        return startAwaitingTask(processes_waiting_to_start, 0);
    }
    else
    {
        /*Worker must send command to the master in order to start the process*/
        int workerRank;
        /*Send a command to the worker*/
        send_command(_START, 0, 0);
        /*Reveive the rank of this worker*/
        MPI_Recv(&workerRank, 1, MPI_INT, 0, PID_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        return workerRank;
    }
}

int startAwaitingTask(int awaiting_id, int UE_to_start)
{
    int awaitingProcessMPIRank = -1;
    if (processes_waiting_to_start)
    {
        int i;
        for (i = 0; i < UEs - 1; i++)
        {
            /*if UE is not active*/
            if (!UEs_state[i])
            {
                UEs_state[i] = 1;
                if (processes_waiting_to_start == awaiting_id)
                {
                    awaitingProcessMPIRank = i + 1;
                    printf("[Master] Starting process %d\n", i + 1);
                    send_command(_IDLE, awaitingProcessMPIRank, UE_to_start);
                }
                else
                {
                    printf("[Master] Starting process with -1 %d\n", i + 1);
                    send_command(_IDLE, awaitingProcessMPIRank, -1);
                }
                processes_waiting_to_start--;
                if (processes_waiting_to_start == 0)
                    break;
            }
            if (i == UEs - 2)
            {
                handling_exit("no more process available");
            }
        }
    }
    return awaitingProcessMPIRank;
}

void close_pool()
{
    if (rank == _MASTER)
    {
        send_command(_COMPLETE, 0, 0);
    }
}

// int workerSleep()
// {

//         if (incoming_msg.com == _IDLE)
//         {
//             send_command(_SLEEP, _MASTER, 0);
//             if (req_poll != MPI_REQUEST_NULL)
//             {
//                 //printf("ms%d",msg);
//                 printf("heeee\n");
//                 MPI_Wait(&req_poll, MPI_STATUS_IGNORE);
//                 printf("Message%d", incoming_msg.com);
//             }
//         }
//         return recv_handler_worker();

// }

/*Ask gets the values of zero or one*/
int should_terminate_worker(int ask)
{
    /* If the worker wants to stop */
    if (ask)
    {
        /* If the incoming message from the master is IDLE */
        if (incoming_msg.com == _IDLE)
        {
            send_command(_SLEEP, _MASTER, 0);
            if (req_poll != MPI_REQUEST_NULL)
            {
                MPI_Wait(&req_poll, MPI_STATUS_IGNORE);
                printf("[Worker] Message from [Master]%d", incoming_msg.com);
            }
        }
        /*Act after receiving*/
        return recv_handler_worker();
    }
    else 
    {
        /*Check if there is a request from the Master to stop*/
        if (req_poll != MPI_REQUEST_NULL)
        {
            int flag;
            /* Check if request is completed */
            MPI_Test(&req_poll, &flag, MPI_STATUS_IGNORE);
            /*If there is a request that carries a stop message then stop*/
            if (flag && incoming_msg.com == _STOP)
            {
                return 1;
            }
        }
    }
    return 0;
}

/*Handling a workers receiving command*/
static int recv_handler_worker()
{
    if (incoming_msg.com == _IDLE)
    {
        printf("[Worker] Soldier %d is ready  \n", rank);
        /*Issue a non blocking receive so that it starts work when master orders*/
        MPI_Irecv(&incoming_msg, 1, Message_Control_DataType, _MASTER, CONTROL_TAG, MPI_COMM_WORLD, &req_poll);
        printf("[Worker] Heart-Beat ready!  \n", rank);
        return 1;
    }
    /* Stop the worker if possible */
    else if (incoming_msg.com == _STOP)
    {
        printf("[Worker] Process %d commanded to stop\n", rank);
        return 0;
    }
    else
    {
        handling_exit("Uknown command received");
        return 0;
    }
}