/**
 * @Author: B159973
 * @Date:	10/4/2019
 * @Course: Parallel Design Patterns - 2020
 * @University of Edinburgh
*/
/***************************ACTORS***********************************/
/********************************************************************/
#include "actor.h"
#include "registry.h"
#include "squirrel.h"
#include "cell.h"
#include "clock.h"
/*************************PROCESS************************************/
/********************************************************************/
#include "process_pool.h"
#include "master.h"
#include "worker.h"
/*************************LIBRARIES**********************************/
/********************************************************************/
#include "stdio.h"
#include "mpi.h"
#include <time.h>
#include <errno.h> 
/********************************************************************/
#include "main.h"
/********************************************************************/
void chronicle(struct month **lastmonth, int *healthy_s, int *unhealthy_s)
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

void clock_work(Registry_cell *r, int workers_size, struct Clock *clock)
{
  int i = 0, j = 0, month = 0, y = 0;
  int forever = 1;
  struct Registry_cell *current_r = r;
  int num_workers = 0, k = 0, squirrels = 0;
  int *cells;
  int data_r[_NUM_CELLS][3];
  int cell_id;
  
  /*******Send to all cell workers that month has passed******/
  MPI_Request rsr[_NUM_CELLS];
  MPI_Request rss[_NUM_CELLS];

  /*Squirrels healthy and unhealthy that passed from each cell*/
  int *squirrels_healthy = (int *)calloc(_NUM_CELLS, sizeof(int));
  int *squirrels_unhealthy = (int *)calloc(_NUM_CELLS, sizeof(int));
  int *workers = (int *)calloc(_NUM_CELLS, sizeof(int));
  while (current_r != NULL)
  {
    /*If the rank contains cells*/
    if (current_r->num_c != 0)
    {
      /* Workers  */
      for (k = 0; k < current_r->num_c; k++)
      {
        /*Send to every worker the cells*/
        if (_DEBUG)
          printf("[Worker]Clock to Cell ID %d %d \n", current_r->rank, _TAG_CLOCK + current_r->actors_ID[k]);
        MPI_Isend(&clock->timeline->ID, 1, MPI_INT, current_r->rank, _TAG_CLOCK + current_r->actors_ID[k], MPI_COMM_WORLD, &rss[y]);
        /*Receive the from the cell*/
        MPI_Irecv(&data_r[i], 3, MPI_INT, current_r->rank, _TAG_CLOCK + current_r->actors_ID[k], MPI_COMM_WORLD, &rsr[y]);
        y++;
        i++;
      }
      workers[num_workers] = current_r->rank;
      num_workers++;
    }
    else
    {
      squirrels += current_r->num_s;
    }
    current_r = current_r->next;
  }

  MPI_Waitall(_NUM_CELLS, rss, MPI_STATUSES_IGNORE);
  MPI_Waitall(_NUM_CELLS, rsr, MPI_STATUSES_IGNORE);

  /*Receive from each cell
    1) Healthy squirrels
    2) Unhealthy squirrells
    3) Cell ID
*/
  for (i = 0; i < _NUM_CELLS; i++)
  {
    squirrels_healthy[cell_id] = data_r[i][0];
    squirrels_unhealthy[cell_id] = data_r[i][1];
    cell_id = data_r[i][2];
  }

  /*Receive from every worker the IDs of healthy and unhealthy squirrels that step into that group of cells*/
  int u_stats[num_workers][_MAX_SQUIRRELS];
  int h_stats[num_workers][_MAX_SQUIRRELS];
  MPI_Request r_stats_h[num_workers];
  MPI_Request r_stats_u[num_workers];

  for (i = 0; i < num_workers; i++)
  {
    MPI_Irecv(&h_stats[i], _MAX_SQUIRRELS, MPI_INT, MPI_ANY_SOURCE, _TAG_STATS_HEALTHY, MPI_COMM_WORLD, &r_stats_h[i]);
    MPI_Irecv(&u_stats[i], _MAX_SQUIRRELS, MPI_INT, MPI_ANY_SOURCE, _TAG_STATS_UNHEALTHY, MPI_COMM_WORLD, &r_stats_u[i]);
  }

  MPI_Waitall(num_workers, r_stats_h, MPI_STATUSES_IGNORE);
  MPI_Waitall(num_workers, r_stats_u, MPI_STATUSES_IGNORE);

  int healthy_squarrels[_MAX_SQUIRRELS];
  int unhealthy_squarrels[_MAX_SQUIRRELS];
  int inside_u;
  int inside_h;
  int h_c = 0;
  int u_c = 0;

  init_squirrel_stats(healthy_squarrels,unhealthy_squarrels);

  /*Filter out the number of healhty and unhealthy squirrels*/
  for (i = 0; i < num_workers; i++)
  {
    for (j = 0; j < _MAX_SQUIRRELS; j++)
    {
      inside_u = 0;
      inside_h = 0;

      for (k = 0; k < _MAX_SQUIRRELS; k++)
      {
        if (h_stats[i][j] == healthy_squarrels[k])
        {
          inside_h = 1;
        }
        if (u_stats[i][j] == unhealthy_squarrels[k])
        {
          inside_u = 1;
        }
      }
      if (!inside_h)
      {
        healthy_squarrels[h_c] = h_stats[i][j] ;
        h_c++;
      }
      if (!inside_u)
      {
        unhealthy_squarrels[u_c] = u_stats[i][j] ;
        u_c++;
      }
    }
  }

  print_stat_squirrels(healthy_squarrels, unhealthy_squarrels, clock->timeline->ID, 0);

  /* Update the timeline of the simulation, by inserting the number of healthy and unhealthy squirrels, influx and
  population from each cell*/
  chronicle(&clock->timeline, squirrels_healthy, squirrels_unhealthy);

  printf("~~~~~~~~~~~~~~~~~~~~~~~MONTH CHANGE~~~~~~~~~~~~~~~~~~~~~~~\n");
  printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~%d~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n", clock->timeline->ID);
  printf("~~~~~~~~~~~~~~~~~~~~~~~MONTH CHANGE~~~~~~~~~~~~~~~~~~~~~~~\n");
}


static struct Clock new (int rank, int ID)
{
  struct Clock clock;

  clock.actor = Actor.new(rank, ID);
  clock.timeline = (struct month *)malloc(sizeof(struct month));
  clock.timeline->ID = 0;
  clock.timeline->squirrels_healthy = 0;
  clock.timeline->squirrels_unhealthy = 0;
  return clock;
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
const struct ClockClass Clock = {.new = &new};