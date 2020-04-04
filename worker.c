
/*************************LIBRARIES**********************************/
/********************************************************************/
#include "mpi.h"
#include "omp.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
/********************************************************************/
/***************************ACTORS***********************************/
/********************************************************************/
#include "actor.h"
#include "cell.h"
#include "registry.h"
#include "clock.h"
#include "squirrel.h"
/*************************PROCESS************************************/
/********************************************************************/
#include "process_pool.h"
#include "master.h"
#include "worker.h"

#include "main.h"
#include "ran2.h"
/********************************************************************/

/*Return the rank of the cell*/

int getCellFromPosition(float x, float y)
{
    return ((int)(x * 4) + 4 * (int)(y * 4));
}

void print_pos(struct Squirrel *this)
{
    printf("\nSquirrel ID:%d, pos X:%f ,pos Y:%f ", this->actor.getID(&this->actor), this->pos_x, this->pos_y);
}

int if_squirrels_msg(MPI_Status status)
{
    int tag = status.MPI_TAG;
    if (tag % _TAG_SQUIRRELS == 0)
    {
        return 1;
    }
    return 0;
}

int if_clock_msg(MPI_Status status, int cellID)
{

    if (status.MPI_TAG == _TAG_CLOCK + cellID)
    {
        return 1;
    }
    return 0;
}

struct Cell *spawnCells(int startID, int endID, int rank)
{
    struct Cell *cells = (struct Cell *)malloc((endID - startID) * sizeof(struct Cell));
    int i, k = 0;
    /* Spawn actors*/
    for (i = 0; i < (endID - startID); i++)
    {
        *(cells + i) = Cell.new(rank, i, 0.0, 0.0);
        (cells + i)->influx = 0;
        (cells + i)->pop = 0;
        // k++;
    }

    // printf("Squirrels healthy %d \n", (cells + 8)->day_stats->squirrels_healthy);
    return cells;
}

struct Squirrel *spawnSquirrels(int startID, int endID, int rank)
{
    struct Squirrel *squirrels = (struct Squirrel *)malloc((endID - startID) * sizeof(struct Squirrel));
    int i;
    int k = 0;
    /* Spawn actors*/
    for (i = startID; i < endID; i++)
    {
        *(squirrels + k) = Squirrel.new(rank, i, 0, 5000, 0.0, 0.0);
        k++;
    }

    return squirrels;
}

void erase_day(struct Day *lastday)
{
    Day *d = lastday;
    while (d->nextday->nextday != NULL)
    {
        d = d->nextday;
    }
    Day *d_last = d->nextday;
    free(d_last);
    d->nextday = NULL;
}

int length(struct Day *head)
{
    struct Day *current = head;
    int count = 0;
    while (current != NULL)
    {
        count++;
        current = current->nextday;
    }
    return count;
}

void chronicle(struct Day **lastday, int *healthy_s, int *unhealthy_s, float avg_influx, float avg_pop, int cells)
{
    struct Day *midnight = malloc(sizeof(struct Day));
    midnight->squirrels_healthy = (int *)calloc(cells, sizeof(int));
    midnight->squirrels_unhealthy = (int *)calloc(cells, sizeof(int));
    int i;
    for (i = 0; i < cells; i++)
    {
        midnight->squirrels_healthy[i] = healthy_s[i];
        midnight->squirrels_unhealthy[i] = unhealthy_s[i];
    }

    midnight->ID = (*lastday)->ID + 1;
    midnight->nextday = *lastday;
    *lastday = midnight;
}

void worker(int rank, struct Registry_cell *registry, int size)
{
    /*Worker will either control cells or squirrels*/
    int alive = 1;

    /* Receive message from the master to start the work*/
    int data[3];
    MPI_Recv(&data, 3, MPI_INT, _MASTER, _TAG_INITIAL, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    /*
            Data package 
            [0] = start ID of squirrel
            [1] = end ID of squirrel
            OR
            [0] = start ID of cell
            [1] = end ID of cell
            AND
            [2] = instatiate squirrels(0) or cells(1) 
        */
    /*Instanitate with correct IDs*/
    int success_assign = 1, success_all_assign = 0;
    long seed = 1;
    int i = 0;

    /* If the worker handles squirrels*/
    if (data[2] == 0)
    { /*Initally create the squirrels and assign them* their identities*/

        int num_squirrels = (data[1] - data[0]);
        int *influx_all[num_squirrels];
        int *pop_all[num_squirrels];
        int data_cell[num_squirrels][2];

        /* Spawn the squirrels on the worker*/
        struct Squirrel *squirrels = spawnSquirrels(data[0], data[1], rank);
        MPI_Request *rs = (MPI_Request *)malloc(sizeof(MPI_Request) * num_squirrels);

        /*Reduce to master for a sychronization, for a succesful spawning*/
        MPI_Reduce(&success_assign, &success_all_assign, 1, MPI_FLOAT, MPI_SUM, _MASTER,
                   MPI_COMM_WORLD);

        while (alive)
        {
            alive = should_terminate_worker(0);
            /*Start the squirrels work*/
            if (alive)
            {
                for (i = 0; i < num_squirrels; i++)
                {
                    /*If the squirrel is not dead*/
                    if ((squirrels + i)->health != -1)
                    {
                        /*Drop the squirrel somewhere inside the map*/
                        float new_x, new_y;
                        srand(time(NULL));
                        seed = rand() % __INT_MAX__;
                        squirrelStep(0.0, 0.0, &new_x, &new_y, &seed);
                        (squirrels + i)->pos_x = new_x;
                        (squirrels + i)->pos_y = new_y;

                        /* How active is this group of squirrels? 
                        Each month the activity of the squirrels changes.
                        Some become more active while other rest more!*/
                        delay(10);
                        /*Make squirrel work and collect the two references values
                        The function here issues an Ireceive for gathering information for
                        every cell.
                    */
                        rs[i] = squirrels_work(squirrels + i, rank, registry, data_cell[i], &alive);
                    }
                }
            }
            /* This is a deadlock point. We do not use wait_all here because the cells might be shutdown 
                at the time they wait for and answer!!! This is happening because the cells send a complete 
                message to the master. Therefore after the complete message the master will also send to the worker
                of the squirrels to shutdown. To avoid getting into a deadlock, we keep testing if the receives are complete
                while also checking if the squirrels workers must shutdown !*/

            int ready;
            /*Test for all the values to be received from the cell */
            MPI_Testall(num_squirrels, rs, &ready, MPI_STATUSES_IGNORE);
            while (!ready && alive != 0)
            {
                MPI_Testall(num_squirrels, rs, &ready, MPI_STATUSES_IGNORE);
                if (should_terminate_worker(0) == 0)
                {
                    alive = 0;
                    ready = 1;
                }
            }

            if (1)
                for (i = 0; i < num_squirrels; i++)
                {
                    printf("Population %d\n", data_cell[i][0]);
                    printf("Influx %d\n", data_cell[i][1]);
                }
            if (alive)
                /*Squirrels do their routine of life (death, born, catch disease)*/
                for (i = 0; i < num_squirrels; i++)
                {
                    squirrel_life(squirrels + i, data_cell[i][0], data_cell[i][1], &num_squirrels);
                }
        }
    }

    /* The worker handles cells*/
    else if (data[2] == 1)
    {
        int day = 0, ask = 0;
        int clock_msg = 1;
        int work = 0;
        int cellID;
        int startID = data[0], endID = data[1];
        /*Initally create the cells and assign them* their identities*/
        struct Cell *cells = spawnCells(startID, endID, rank);
        int num_cells = endID - startID;
        int flag_last_day = 1;
        int stats[_MAX_DAYS_SIMULATION * 16 * 2] = {0};

        /*Reduce to Master for a sychronization*/
        MPI_Reduce(&success_assign, &success_all_assign, 1, MPI_FLOAT, MPI_SUM, _MASTER,
                   MPI_COMM_WORLD);
        while (alive)
        {
            alive = should_terminate_worker(ask);
            MPI_Status status;
            MPI_Request rs[num_cells];
            int move_on = 0, clock_messages = 0, f, ready;
            /* If the worker haven't ask to finish*/
            int cells_receive = 1;
            int recv_squirrels = 0;

            if (!ask)
            {
                /*This while loop lasts until a month changes. When a  month changes, the clock sends
                to all the cells a message. Every worker must exit the loop until all these messages are gathered
                for each cell (which are 8 messages) and the cells then stop receiving messages from the squirrels
                and update themselves as well as they send how many healthy and unhealthy squirrels step into them to Master
                in order for the master to send a report*/
                while (cells_receive && should_terminate_worker(ask))
                {
                    MPI_Status status;
                    /* The worker receives a message and then it checks if its from a 
                            squirrel or a clock */
                    MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                    int recv_data[2];
                    // printf("[Worker] TAG %d RANK %d \n",status.MPI_TAG,status.MPI_SOURCE);

                    if (status.MPI_TAG < _TAG_CLOCK)
                    {
                        work = 1;
                        MPI_Recv(&recv_data, 2, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
                        cellID = recv_data[1];
                    }
                    else if (status.MPI_TAG >= _TAG_CLOCK)
                    {
                        //printf("[Worker] TAG %d RANK %d \n", status.MPI_TAG, status.MPI_SOURCE);
                        work = 2;
                        cellID = status.MPI_TAG - _TAG_CLOCK;
                    }

                    /* If this is from a squirrel*/
                    if (work == 1)
                    {
                       
                        /*Send the inlfux and population values to the incoming squirrel */
                        int send_data[2] = {(cells + (cellID-startID))->influx, (cells + (cellID-startID))->pop};
                        MPI_Send(&send_data, 2, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD);
                       // if (_DEBUG)
                        printf("[Worker]Cell ID %d Sending Data to %d  %d \n", (cells + cellID-startID)->actor.ID, send_data[0], send_data[1]);

                        /*Receive health value from the squierrel*/
                        recv_squirrels++;

                        if (recv_data[0] == 1)
                            stats[(1 * (num_cells * 2) * _MAX_DAYS_SIMULATION) + (cellID * _MAX_DAYS_SIMULATION) + day]++;
                        //z * ySize * xSize + y * xSize + x
                        else if (recv_data[0] == 0)
                            stats[(0 * (num_cells * 2) * _MAX_DAYS_SIMULATION) + (cellID * _MAX_DAYS_SIMULATION) + day]++;
                        //  printf( "%d   ddd\n",  stats[(1 * (num_cells*2) * _MAX_DAYS_SIMULATION )+(cellID * _MAX_DAYS_SIMULATION )+ day]);
                    }
                    /*If this is from a clock*/
                    else if (work == 2)
                    {
                        /* Receive the message from the clock, if there are as many messages from the clock as
                        the number of cells this worker is handling then exit the loop*/
                        int d=0 ;
                        MPI_Recv(&d, 1, MPI_INT, _MASTER, _TAG_CLOCK + cellID, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        day = d;
                        clock_messages++;
                        if (clock_messages >= num_cells)
                            cells_receive = 0;
                    }
                }
                clock_messages = 0;
                cells_receive = 1;

                /* Procceding to the stage of sending the stats of each cell to MASTER (reporting state)*/
                int k = 0;
                for (i = 0; i < num_cells; i++)
                {
                    int data_send[3];
                    int data_recv[2];
                    data_send[0] = stats[0 * num_cells * _MAX_DAYS_SIMULATION + (i + startID) * _MAX_DAYS_SIMULATION + day];
                    data_send[1] = stats[1 * num_cells * _MAX_DAYS_SIMULATION + (i + startID) * _MAX_DAYS_SIMULATION + day];
                    data_send[2] = i;
                    MPI_Isend(&data_send, 3, MPI_INT, _MASTER, _TAG_CLOCK + i + startID, MPI_COMM_WORLD, &rs[k]);
                    k++;
                    update_cell(cells + i, day, rank, stats, i+startID, num_cells);
                }
                int flag;

                int ready;
                /*Test for all the values to be received from the cell */

                MPI_Testall(num_cells, rs, &ready, MPI_STATUSES_IGNORE);
                while (!ready && (alive) != 0)
                {
                    MPI_Testall(2, rs, &ready, MPI_STATUSES_IGNORE);
                    if (should_terminate_worker(0) == 0)
                    {
                        alive = 0;
                        ready = 1;
                    }
                }
                printf("Worker that handles cells received  %d squirrels\n", recv_squirrels);
                recv_squirrels = 0;

                if (_DEBUG)
                    printf("[Worker] Day is %d rank is %d \n", day, rank);
            }
            /* Update the value of the cells*/

            /* If this is the final month of the simulation then terminate the worker and its cells*/
            if (day >= _MAX_DAYS_SIMULATION && flag_last_day)
            {
                ask = 1; /*Ask master to temrinate*/
                flag_last_day = 0;
                send_command(_COMPLETE, _MASTER, 0);
            }
        }
    }
}
//  static void send_msg_sq(int _rank, int _tag, MPI_Datatype mpi_type, MPI_Comm comm, struct Squirrel *this)
//   {
//     MPI_Send(this->health, 1, mpi_type, _rank, _tag, comm);
//   }
