/*************************LIBRARIES**********************************/
/********************************************************************/
#include "mpi.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h> 
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

struct Cell *spawnCells(int startID, int endID, int rank)
{
    struct Cell *cells = (struct Cell *)malloc((endID - startID) * sizeof(struct Cell));
    int i, k = 0;
    /* Spawn actors*/
    for (i = 0; i < (endID - startID); i++)
    {
        *(cells + i) = Cell.new(rank, i);
        (cells + i)->influx = 0;
        (cells + i)->pop = 0;
    }

    return cells;
}

struct Squirrel *spawnSquirrels(int startID, int endID, int rank, int unhealthy)
{
    struct Squirrel *squirrels = (struct Squirrel *)malloc((endID - startID) * sizeof(struct Squirrel));
    int i;
    int k = 0;
    /* Spawn actors*/
    for (i = 0; i < (endID - startID); i++)
    {
        *(squirrels + k) = Squirrel.new(rank, i + startID, 0, 5000, 0.0, 0.0);

        /*Drop the squirrel somewhere inside the map*/
        float new_x, new_y;
        srand(time(NULL));
        long seed = rand() % __INT_MAX__;
        squirrelStep(0.0, 0.0, &new_x, &new_y, &seed);
        (squirrels + k)->pos_x = new_x;
        (squirrels + k)->pos_y = new_y;

        k++;
    }
    //if (rank ==)
    for (i = 0; i < unhealthy; i++)
    {
        (squirrels + i)->health = 0;
    }

    return squirrels;
}

void erase_month(struct month *lastmonth)
{
    month *d = lastmonth;
    while (d->nextmonth->nextmonth != NULL)
    {
        d = d->nextmonth;
    }
    month *d_last = d->nextmonth;
    free(d_last);
    d->nextmonth = NULL;
}

int length_month(struct month *head)
{
    struct month *current = head;
    int count = 0;
    while (current != NULL)
    {
        count++;
        current = current->nextmonth;
    }
    return count;
}

int max_threshold(int num_squirrels)
{
    if (num_squirrels > _MAX_SQUIRRELS)
    {
        send_command(_COMPLETE, _MASTER, 0);
    }
    return 1;
}

void chronicle(struct month **lastmonth, int *healthy_s, int *unhealthy_s, float avg_influx, float avg_pop)
{
    struct month *midnight = malloc(sizeof(struct month));
    midnight->squirrels_healthy = (int *)calloc(_NUM_CELLS, sizeof(int));
    midnight->squirrels_unhealthy = (int *)calloc(_NUM_CELLS, sizeof(int));
    int i;
    for (i = 0; i < _NUM_CELLS; i++)
    {
        midnight->squirrels_healthy[i] = healthy_s[i];
        midnight->squirrels_unhealthy[i] = unhealthy_s[i];
    }

    midnight->ID = (*lastmonth)->ID + 1;
    midnight->nextmonth = *lastmonth;
    *lastmonth = midnight;
}
int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
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

        int born=0;
        int dead=0;
        int num_squirrels = (data[1] - data[0]);
        int data_cell[_MAX_SQUIRRELS][2];
        int healthy = 0;
        /* Spawn the squirrels on the worker*/
        struct Squirrel *squirrels = spawnSquirrels(data[0], data[1], rank, UNHEALTHY_SQUIRRELS);

        /*Reduce to master for a sychronization, for a succesful spawning*/
        MPI_Reduce(&success_assign, &success_all_assign, 1, MPI_FLOAT, MPI_SUM, _MASTER,
                   MPI_COMM_WORLD);

        while (alive)
        {
            MPI_Request *rs = (MPI_Request *)malloc(sizeof(MPI_Request) * _MAX_SQUIRRELS);
            alive = should_terminate_worker(0);
            /*Start the squirrels work*/
            if (alive)
            {
                for (i = 0; i < num_squirrels; i++)
                {
                    /*If the squirrel is not dead*/
                    if ((squirrels + i)->health != -1)
                    {
                        /* How active is this group of squirrels? 
                        Each month the activity of the squirrels changes.
                        Some become more active while other rest more!*/
                        int lazyness = rand() % 20;
                        msleep(lazyness);

                        /*Make squirrel work and collect the two references values
                        The function here issues an Ireceive for gathering information for
                        every cell.
                        */
                        if (squirrels + i != NULL)
                        {
                            squirrels_comm(squirrels + i, rank, data_cell[i], &alive, rs + healthy);
                            healthy++;
                        }
                    }
                }
            }
            /* 
                This is a deadlock point. We do not use wait_all here because the cells might be shutdown 
                at the time they wait for and answer!!! This is happening because the cells send a complete 
                message to the master. Therefore after the complete message the master will also send to the worker
                of the squirrels to shutdown. To avoid getting into a deadlock, we keep testing if the receives are complete
                while also checking if the squirrels workers must shutdown !
            */
            int ready = 0;
            /*Test for all the values to be received from the cell */
            MPI_Testall(healthy, rs, &ready, MPI_STATUSES_IGNORE);
            while (!ready && alive != 0)
            {
                MPI_Testall(healthy, rs, &ready, MPI_STATUSES_IGNORE);
                if (should_terminate_worker(0) == 0)
                {
                    alive = 0;
                    ready = 1;
                }
            }

            if (0)
                for (i = 0; i < num_squirrels; i++)
                {
                    printf("Influx %d\n", data_cell[i][0]);
                    printf("Population %d\n", data_cell[i][1]);
                }

            /* If the squirrel life returns 1 , a newborn is in the game!!*/
            /*Squirrels do their routine of life (death, born, catch disease)*/
            
            for (i = 0; i < num_squirrels; i++)
            {
                /* If the squirrel life returns 1 , a newborn is in the game!!*/
                if (squirrel_life(squirrels + i, data_cell[i][0], data_cell[i][1], &num_squirrels, rank,&dead))
                {
                    if (max_threshold(num_squirrels))
                    {
                         if (!should_terminate_worker(0)){ //This is added for minimizing the error value on born squirrels
                        born++;
                        /*Carefully, it needs a realloc to work correctly*/
                        squirrels = (struct Squirrel *)realloc(squirrels, (num_squirrels + 1) * sizeof(struct Squirrel));
                        *(squirrels + num_squirrels) = Squirrel.new(rank, num_squirrels, 0, 0, 0.0, 0.0);
                        /* Drop the squirrel somewhere inside the same Cell position as the squirrel that was born */
                        (squirrels + num_squirrels)->pos_x = (squirrels + i)->pos_x;
                        (squirrels + num_squirrels)->pos_y = (squirrels + i)->pos_y;
                        /* Increase the squirrels the cell is handling */
                        num_squirrels++;
                         }
                    }
                
                }
            }
            healthy = 0;
        }
        free(squirrels);
        printf("[Worker %d]Born squirrels: %d  ~~~ Dead Squirrels %d\n",rank, born,dead);
    }
    /* The worker handles cells*/
    if (data[2] == 1)
    {
        int month = 0, ask = 0;
        int clock_msg = 1;
        int work = 0;
        int cellID;
        int startID = data[0], endID = data[1];
        /*Initally create the cells and assign them* their identities*/
        struct Cell *cells = spawnCells(startID, endID, rank);
        int num_cells = endID - startID;
        int flag_last_month = 1;
        /* These are the statistics for holding information about each day and
            for each cell how many healthy and infected squirrels have stepped into*/
        int stats[_MAX_MONTHS_SIMULATION * _NUM_CELLS * 2] = {0};

        /*Reduce to Master for a sychronization*/
        MPI_Reduce(&success_assign, &success_all_assign, 1, MPI_FLOAT, MPI_SUM, _MASTER,
                   MPI_COMM_WORLD);
        while (alive)
        {
            alive = should_terminate_worker(ask);
            MPI_Status status;
            MPI_Request rs[num_cells];
            int clock_messages = 0;
            /* If the worker haven't ask to finish*/
            int cells_receive = 1;
            int recv_squirrels = 0;

            int *squirrels_IDs_healthy = (int *)malloc(_MAX_SQUIRRELS * sizeof(int));
            int *squirrels_IDs_unhealthy = (int *)malloc(_MAX_SQUIRRELS * sizeof(int));
            init_squirrel_stats(squirrels_IDs_healthy, squirrels_IDs_unhealthy);

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
                    int recv_data[3];

                    if (status.MPI_TAG < _TAG_CLOCK)
                    {
                        work = 1;
                        MPI_Recv(&recv_data, 3, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
                        cellID = recv_data[1];
                        /*Store the ID of the squirrel according with its health*/
                        store_squirrel(recv_data[2], squirrels_IDs_healthy, squirrels_IDs_unhealthy, recv_data[0]);
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
                        int send_data[2] = {(cells + (cellID - startID))->influx, (cells + (cellID - startID))->pop};
                        MPI_Send(&send_data, 2, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD);
                        if (_DEBUG)
                            printf("[Worker]Cell ID %d Sending Data to %d  %d \n", recv_data[2], send_data[0], send_data[1]);

                        /*Receive health value from the squierrel*/
                        recv_squirrels++;

                        if (recv_data[0] == 1)
                            stats[(1 * (_NUM_CELLS)*_MAX_MONTHS_SIMULATION) + (cellID * _MAX_MONTHS_SIMULATION) + month]++;
                        else if (recv_data[0] == 0)
                            stats[(0 * (_NUM_CELLS)*_MAX_MONTHS_SIMULATION) + (cellID * _MAX_MONTHS_SIMULATION) + month]++;
                    }
                    /*If this is from a clock*/
                    else if (work == 2)
                    {
                        /* Receive the message from the clock, if there are as many messages from the clock as
                        the number of cells this worker is handling then exit the loop*/
                        int d = 0;
                        MPI_Recv(&d, 1, MPI_INT, _MASTER, _TAG_CLOCK + cellID, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                        month = d;
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
                    data_send[0] = stats[0 * _NUM_CELLS * _MAX_MONTHS_SIMULATION + (i + startID) * _MAX_MONTHS_SIMULATION + month];
                    data_send[1] = stats[1 * _NUM_CELLS * _MAX_MONTHS_SIMULATION + (i + startID) * _MAX_MONTHS_SIMULATION + month];
                    data_send[2] = i;
                    MPI_Isend(&data_send, 3, MPI_INT, _MASTER, _TAG_CLOCK + i + startID, MPI_COMM_WORLD, &rs[k]);
                    k++;
                    update_cell(cells + i, month, rank, stats, i + startID);
                }

                int ready;
                /*Test for all the values to be received from the cell */
                MPI_Testall(num_cells, rs, &ready, MPI_STATUSES_IGNORE);
                while (!ready && (alive) != 0)
                {
                    MPI_Testall(num_cells, rs, &ready, MPI_STATUSES_IGNORE);
                    if (should_terminate_worker(0) == 0)
                    {
                        alive = 0;
                        ready = 1;
                    }
                }
                MPI_Request rstat[2];
                MPI_Isend(squirrels_IDs_healthy, _MAX_SQUIRRELS, MPI_INT, _MASTER, _TAG_STATS_HEALTHY, MPI_COMM_WORLD, &rstat[0]);
                MPI_Isend(squirrels_IDs_unhealthy, _MAX_SQUIRRELS, MPI_INT, _MASTER, _TAG_STATS_UNHEALTHY, MPI_COMM_WORLD, &rstat[1]);

                MPI_Testall(2, rstat, &ready, MPI_STATUSES_IGNORE);
                while (!ready && (alive) != 0)
                {
                    MPI_Testall(2, rstat, &ready, MPI_STATUSES_IGNORE);
                    if (should_terminate_worker(0) == 0)
                    {
                        alive = 0;
                        ready = 1;
                    }
                }

                if (1)
                    printf("Worker that handles cells received  %d squirrels messages\n", recv_squirrels);
                recv_squirrels = 0;

                free(squirrels_IDs_healthy);
                free(squirrels_IDs_unhealthy);
            }
            /* Update the value of the cells*/

            /* If this is the final month of the simulation then terminate the worker and its cells*/
            if (month >= _MAX_MONTHS_SIMULATION && flag_last_month)
            {
                ask = 1; /*Ask master to temrinate*/
                flag_last_month = 0;
                send_command(_COMPLETE, _MASTER, 0);
            }
        }
    }
}
