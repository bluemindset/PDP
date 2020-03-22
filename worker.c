
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
#include "clock.h"
#include "squirrel.h"
#include "registry.h"
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

int update_cell_day(struct Cell *this)
{
    int i, sum = 0;
    this->influx = this->pop = 0;
    Day *d = this->squirrels_day;
    int c = 0;

    while (d != NULL)
    {
        this->pop += d->squirrels_unhealthy + d->squirrels_healthy;
        if (c++ > _DAYS_INFLUX)
            this->influx += d->squirrels_unhealthy;
        d = d->nextday;
    }
}

int if_squirrels_msg(MPI_Status status)
{
    int tag = status.MPI_TAG;

    if (tag == _TAG_SQUIRRELS)
    {
        return 1;
    }
    return 0;
}

int if_clock_msg(MPI_Status status)
{
    int tag = status.MPI_TAG;

    if (tag == _TAG_CLOCK)
    {
        return 1;
    }
    return 0;
}

struct Cell *spawnCells(int startID, int endID, int rank)
{
    struct Cell *cells = (struct Cell *)malloc((endID - startID) * sizeof(struct Cell));

    int i;
    int k = 0;
    /* Spawn actors*/
    for (i = startID; i < endID; i++)
    {
        *(cells + k++) = Cell.new(rank, i, 0.0, 0.0);
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
        *(squirrels + k++) = Squirrel.new(rank, i, 0, 5000, 0.0, 0.0);
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

void chronicle(struct Day **lastday, int healthy_s, int unhealthy_s)
{
    struct Day *midnight = malloc(sizeof(struct Day));

    midnight->squirrels_healthy = healthy_s;
    midnight->squirrels_unhealthy = unhealthy_s;

    midnight->nextday = *lastday;
    *lastday = midnight;
}

void worker(int rank, struct Registry_cell *registry, int size)
{

    int workerStatus = 1;
    /*Worker here must wait to receive a message*/
    while (workerStatus)
    {
        /*startWorkerProcess -> call it only if worker needs to rise another one*/
        /* Say to worker process to start*/
        //should_terminate_worker();
        /*Send a message to master to start*/

        /* Receive message from the master to start the work*/
        int data[4];
        MPI_Recv(&data, 4, MPI_INT, _MASTER, _TAG_INITIAL, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Received Data from Master\n");
        /*
        Data package 
        [0] = number of squirrels to instantiate
        [1] = number of cells to instantiate
        [2] = start ID of cell
        */
        /*Instanitate with correct IDs*/
        int success_assign = 1, success_all_assign = 0;
        
        struct Squirrel *squirrels = spawnSquirrels(data[0], data[1], rank);
        struct Cell *cells = spawnCells(data[2], data[3], rank);
        /* Assign into the registry the cells and the squirrels */
        MPI_Reduce(&success_assign, &success_all_assign, 1, MPI_FLOAT, MPI_SUM, _MASTER,
                   MPI_COMM_WORLD);
        // print_register(registry);
        long seed = 0;

        /*Each process creates the squirrels and cells*/
        int num_squirrels = (data[1] - data[0]);
        int num_cells = (data[3] - data[2]);
        /* Work squirrels*/
        int i;

        
#pragma omp parallel num_threads(num_squirrels + num_cells)
        {
            int tid = omp_get_thread_num();
            int numthreads = omp_get_num_threads();
           // printf("num squirels%d",num_squirrels);
            if (tid < num_squirrels)
            {   
                int low = num_squirrels * tid / num_squirrels;
                int high = num_cells * (tid + 1) / num_squirrels;
                if (low == high) high++;
                for (i = low; i < high; i++)
                {
                    squirrels_work(squirrels + i, rank, registry, tid);
                }
            }
            else{
                int low = num_cells * tid / numthreads;
                int high = num_cells * (tid + 1) / numthreads;
                //int high = ceil(high_f);
                //printf("%d  %d\n",low,high);
                if (low == high) high++;
                for (i = low; i < high; i++)
                {
                    cells_work(cells + i, rank, registry, tid);
                }
            }
        }

        workerStatus = workerSleep();
    }
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
