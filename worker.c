
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

int if_clock_msg(MPI_Status status,int cellID)
{

    if (status.MPI_TAG == _TAG_CLOCK+cellID)
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
    for (i = startID; i < endID; i++)
    {
        *(cells + k) = Cell.new(rank, i, 0.0, 0.0);
       
        k++;
    }
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
    midnight->avg_influx = avg_influx;
    midnight->avg_pop = avg_pop;
    midnight->ID = (*lastday)->ID+1;
    midnight->nextday = *lastday;
    *lastday = midnight;
}

void worker(int rank, struct Registry_cell *registry, int size)
{
    /*Worker will either control cells or squirrels*/
    int workerStatus = 1;
    // while (workerStatus)
    // {
    /*startWorkerProcess -> call it only if worker needs to rise another one*/
    /* Receive message from the master to start the work*/
    int data[3];
    MPI_Recv(&data, 3, MPI_INT, _MASTER, _TAG_INITIAL, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("Received Data from Master\n");
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
    int i, it = 0;

    if (data[2] == 0)
    { /*Initally create the squirrels and assign them* their identities*/

        int num_squirrels = (data[1] - data[0]);
        int *influx_all[num_squirrels];
        int *pop_all[num_squirrels];
        int data_cell[num_squirrels][2];

        struct Squirrel *squirrels = spawnSquirrels(data[0], data[1], rank);
        MPI_Request *rs = (MPI_Request *)malloc(sizeof(MPI_Request) * num_squirrels);

        /*Reduce to Master for a sychronization*/
        MPI_Reduce(&success_assign, &success_all_assign, 1, MPI_FLOAT, MPI_SUM, _MASTER,
                   MPI_COMM_WORLD);

        while (workerStatus)
        {
            /*Start the squirrels work*/
            for (i = 0; i < num_squirrels; i++)
            {
                /*Drop the squirrel somewhere inside the map*/
                float new_x, new_y;
                squirrelStep(0.0, 0.0, &new_x, &new_y, &seed);
                (squirrels + i)->pos_x = new_x;
                (squirrels + i)->pos_y = new_y;

                /*Make squirrels work and collect the two references values*/
                rs[i] = squirrels_work(squirrels + i, rank, registry, data_cell[i]);
            }

            /*Wait for all the values to be received*/
            MPI_Waitall(num_squirrels, rs, MPI_STATUSES_IGNORE);

            if (_DEBUG)
                for (i = 0; i < num_squirrels; i++)
                {
                    printf("Population %d\n", data_cell[i][0]);
                    printf("Influx %d\n", data_cell[i][1]);
                }
            /*Squirrels do their routine of life*/
            for (i = 0; i < num_squirrels; i++)
            {
                squirrel_life(squirrels + i, data_cell[i][0], data_cell[i][1]);
            }
            workerStatus = !should_terminate_worker(0);
        }
    }
    else if (data[2] == 1)
    {
        /*Initally create the cells and assign them* their identities*/
        struct Cell *cells = spawnCells(data[0], data[1], rank);
        int num_cells = (data[1] - data[0]);

        /*Reduce to Master for a sychronization*/
        MPI_Reduce(&success_assign, &success_all_assign, 1, MPI_FLOAT, MPI_SUM, _MASTER,
                   MPI_COMM_WORLD);

        /*Worker life is determined throught here*/
        while (workerStatus)
        {
            for (i = 0; i < num_cells; i++)
            {
                cells_work(cells + i, rank, registry);
            }
            workerStatus = !should_terminate_worker(0);
        }
    }
    // }
}
//  static void send_msg_sq(int _rank, int _tag, MPI_Datatype mpi_type, MPI_Comm comm, struct Squirrel *this)
//   {
//     MPI_Send(this->health, 1, mpi_type, _rank, _tag, comm);
//   }

// void worker_receive_instructions(int num_cells, int num_squirrels, int size)
// {

//   int i;
//   /*Number of squirrels*/
//   /*Number of cells*/
//   /*Number of cell IDs*/
//   int c = 0;
//   int data[4];
//   for (i = 0; i < size - 1; i++)
//   {
//     /*if UE is not active*/
//     if (UEs_state[i])
//     {
//       data[0] = num_squirrels;
//       data[1] = num_cells;
//       data[3] = c;
//       data[4] = c + size;
//       MPI_Send(&data, 4, MPI_INT, i + 1, _TAG_SQUIRRELS, MPI_COMM_WORLD);
//     }
//     c += size;
//   }
// }

//   void move(struct Squirrel * squirrels, long seed, int num_squirrels)
//   {
//     int i = 0;
//     for (i = 0; i < num_squirrels; i++)
//     {
//       (squirrels + i)->squirrelStep(&seed, (squirrels + i));
//       if (_DEBUG)
//         print_pos(squirrels + i);
//     }
//   }

/* Say to worker process to start*/
//should_terminate_worker();
/*Send a message to master to start*/

/* Work squirrels*/
// #pragma omp parallel num_threads(2)
//         {

//             int tid = omp_get_thread_num();
//             int numthreads = omp_get_num_threads();

//             if (tid == 0)
//             {
//                 for (i = 0; i < num_squirrels; i++)
//                 {
//                     if (squ_id[i] != -1)
//                     {
//                         /*Drop the squirrel somewhere inside the map*/
//                         float new_x, new_y;
//                         squirrelStep(0, 0, &new_x, &new_y, &seed);
//                         (squirrels + squ_id[i])->pos_x = new_x;
//                         (squirrels + squ_id[i])->pos_y = new_y;
//                         squirrels_work(squirrels + squ_id[i], rank, registry, squ_id, i);
//                     }
//                 }
//             }
//             else
//             {
//                 {
//                     for (i = 0; i < num_cells; i++)
//                     {
//                         cells_work(cells + cells_id[i], rank, registry, tid);
//                     }
//                 }
//             }
//         }
