/**************************************************************/
/**************************************************************/
#include "actor.h"
#include "squirrel.h"
#include "cell.h"
#include "process_pool.h"
#include "stdio.h"
#include "mpi.h"
#include <time.h>

#define _DEBUG 1
/**************************************************************/

#define MAX_REG 10
#define _TAG_SQUIRRELS 1
#define _TAG_CELLS 2
#define _TAG_CLOCK 3
#define _DAYS_INFLUX 5
#define _DAYS_POP 6

void delay(unsigned int);

void squirrels_work(struct Squirrel *squirrel, int rank, _registry_cell *registry);

typedef struct _registry_cell
{
  int rank;
  float pos_x;
  float pos_y;
  int ID;
  _registry_cell * next;
} _registry_cell;

static void worker(int, _registry_cell *);
struct Squirrel *spawnSquirrels(int);
struct Cell *spawnCells(int);
void masterlives();

/**************************************************************/
/**************************************************************/
/**************************************************************/

void masterlives()
{
  int masterStatus = receiving_handle();
  while (masterStatus)
  {
    masterStatus = receiving_handle();
  }
}

void assign_registry( _registry_cell * r, int workers){

      _registry_cell cur = r; 
      while ()

}

int main(int argc, char *argv[])
{
  double x = 5;

  /****Initialize MPI****/
  MPI_Init(NULL, NULL);

  int rank;
  int size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int *workers = (int *)malloc(size * sizeof(int));
  int statusCode = create_pool();
  
  _registry_cell *registry = (_registry_cell *)malloc(16 * sizeof(_registry_cell));

  /*Give work to a worker*/
  if (statusCode == 1)
  {
    worker(rank, registry);
  }
  else if (statusCode == 2)
  {
    //08000468687
    /*Worker determines the jobs to give not other workers*/
    startworkers(size - 1, workers);
   

    /*While the master lives*/
    masterlives();

    /*Send back to master the data */
    //MPI_Recv(data, data_length, MPI_DOUBLE, pid, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }
  // MPI_Barrier(MPI_COMM_WORLD);
  terminate_pool();
  MPI_Finalize();

  return 0;
}

void master_send_instructions(int num_cells,int num_squirrels,int size){

    /*Master sends the number of squirrels*/
    /*The tags for exchange the messages are 0 */
  //  int number_squirrel = 4;
  //  int number_cells = 16;
    /*Send a broadcast to the workers the two parameters*/

     for (i = 0; i < size - 1; i++)
        {
            /*if UE is not active*/
            if (UEs_state[i])
            {
        MPI_Send(&num_squirrels, 1, MPI_INT,i+1,_TAG_SQUIRRELS, MPI_COMM_WORLD);

    MPI_Bcast(&number_squirrel, 1, MPI_INT, _MASTER, MPI_COMM_WORLD);
    MPI_Bcast(&number_cells, 1, MPI_INT, _MASTER, MPI_COMM_WORLD);


}


void startworkers(int num_workers, int *workers)
{
  int i = 0;
  for (i = 0; i < num_workers; i++)
  {
    workers[i] = startWorkerProcess();
    if (workers[i] == NULL)
      exit(0);
  }
}

static void worker(int rank, _registry_cell *registry)
{

  int workerStatus = 1;
  int parentID;
  int num_squirrels;
  int num_cells;
  /*Worker here must wait to receive a message*/
  while (workerStatus)
  {
    /* Say to worker process to start*/
    
    /*Send a message to master to start*/

    /* Receive message from the master to start the work*/
    MPI_Bcast(&num_squirrels, 1, MPI_INT, _MASTER, MPI_COMM_WORLD);
    MPI_Bcast(&num_cells, 1, MPI_INT, _MASTER, MPI_COMM_WORLD);

    struct Squirrel *squirrels = spawnSquirrels(num_squirrels);
    struct Cells *cells = spawnCells(num_cells);

    int i = 0;
    long seed = 0;
    /*Each process creates the squirrels and cells*/

    /* Work squirrels*/
   // for (i = 0; i < num_squirrels; i++)
   // {
   // }

    squirrels_work(squirrels + i, rank, registry);

    workerStatus = workerSleep();
  }
}

void print_pos(struct Squirrel *this)
{
  printf("\nSquirrel ID:%d, pos X:%f ,pos Y:%f ", this->actor.getID(this), this->pos_x, this->pos_y);
}

/*Return the rank of the cell*/
int access_registry(int cell_ID, _registry_cell *registry)
{
  int i, j;
  for (i = 0; i < MAX_REG; i++)
    if (registry->ID == cell_ID)
      return registry->rank;
}

int getCellFromPosition(float x, float y)
{
  return ((int)(x * 4) + 4 * (int)(y * 4));
}
/* Squirrel Routine 
      1. Move 
      2. Send Message to cell health status and wait until successfull
      3. Caclulate prop die 
      4. Caclulate prop born 
     */
void squirrels_work(struct Squirrel *squirrel, int rank, _registry_cell *registry)
{
  int die = 0;
  while (!die)
  {
    int seed = 0;
    int influx;
    int pop;

    /*If squirrel died then skip the rest and flag to 1*/
    if (!squirrel->health)
    {
      die = squirrel->willDie(seed, squirrel);
      continue;
    }

    /*Send health to the stepping cell*/
    int cell_ID = getCellFromPosition(squirrel->pos_x, squirrel->pos_y);
    access_registry(cell_ID, registry);
    send_msg_sq(registry[cell_ID].rank, _TAG_SQUIRRELS, MPI_INT, MPI_COMM_WORLD, &(squirrel->actor));

    MPI_Status s1;
    MPI_Status s2;
    MPI_Request r1;
    MPI_Request r2;

    /*Receive message from the cell*/
    MPI_IRecv(&influx, 1, MPI_INT, MPI_ANY_SOURCE, _TAG_SQUIRRELS, &s1, &r1);
    MPI_IRecv(&pop, 1, MPI_INT, MPI_ANY_SOURCE, _TAG_SQUIRRELS, &s2, &r2);

    /*Squirrel Moves*/
    (squirrel)->squirrelStep(&seed, (squirrel));

    MPI_Wait(&s1, &r1);
    MPI_Wait(&s2, &r2);

    squirrel->update_avgs(influx, pop, squirrel);
    squirrel->willCatchDisease(seed, squirrel);
    squirrel->willGiveBirth(seed, squirrel);
  }
}

void clock_work()
{
  int day = 0 ; 
  int forever = 1;
  /*Broadcast everyone except self*/
  while (forever)
  {
    delay(1);
    day++;
    /*Gather all the workers*/ 
    MPI_Bcast(&day, 1, MPI_INT, _MASTER, MPI_COMM_WORLD);
    if (day == 30)
        send_command(_COMPLETE, _MASTER, 0);
  }
}

void cells_work(struct Cell *this, int rank, _registry_cell *registry)
{
  int forever = 0;
  while (!forever)
  {
    int seed = 0;
    int influx;
    int pop;
    int f;
    MPI_Status status;
    /*Check if there is message from any other actor and act accordiclty*/
    MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &f, &status);
    int s_health;
    if (f)
    {
      if (if_squirrels_msg(status))
      {

        MPI_Request request_inflix;
        MPI_Request request_pop;
        /*Send the inlfux and population values to the incoming squirrel */
        MPI_Isend(&(this->influx), 1, MPI_FLOAT, status.MPI_SOURCE, _TAG_SQUIRRELS, MPI_COMM_WORLD, request_inflix);
        MPI_Isend(&(this->pop), 1, MPI_FLOAT, status.MPI_SOURCE, _TAG_SQUIRRELS, MPI_COMM_WORLD, request_pop);
        /*Receive health value from the squierrel*/
        MPI_Recv(&s_health, 1, MPI_INT, MPI_ANY_SOURCE, _TAG_SQUIRRELS, MPI_COMM_WORLD, &status);

        if (s_health)
          this->squirrels_day->squirrels_healthy++;
        else
          this->squirrels_day->squirrels_unhealthy++;
      }
      else if (if_clock_msg(status))
      {
        /* Perfome update of the cell influx and population values*/
        //int * health_sq =  (int *)malloc(sizeof(int));
        /*This means that a day passed and the squirrels have to be updated*/
        update_cell_day(this);
        int day;
        MPI_Recv(&day, 1, MPI_INT, MPI_ANY_SOURCE, _TAG_CLOCK, MPI_COMM_WORLD, &status);

        if (day > _DAYS_POP)
        {
          chronicle(&this->squirrels_day, 0, 0);
          erase_day(this->squirrels_day);
        }
        else
        {
          chronicle(&this->squirrels_day, 0, 0);
        }
      }
      forever = shouldworkerstop();
    }
  }
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

int Length(struct Day *head)
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

//void response_squirrel(struct Cell * this,int source,int tag){

//}
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

static void send_msg_sq(int _rank, int _tag, MPI_Datatype mpi_type, MPI_Comm comm, struct Squirrel *this)
{
  MPI_Send(this->health, 1, mpi_type, _rank, _tag, comm);
}

struct Cell *spawnCells(int num_cells)
{
  struct Cell *cells = (struct Cells *)malloc(num_cells * sizeof(struct Cell));

  int i = 0;

  /* Spawn actors*/
  for (i = 0; i < num_cells; i++)
  {
    *(cells + i) = Cell.new(5, i, 0.0, 0.0);
  }

  return cells;
}

void move(struct Squirrel *squirrels, long seed, int num_squirrels)
{
  int i = 0;
  for (i = 0; i < num_squirrels; i++)
  {
    (squirrels + i)->squirrelStep(&seed, (squirrels + i));
    if (_DEBUG)
      print_pos(squirrels + i);
  }
}

struct Squirrel *spawnSquirrels(int num_squirrels)
{

  struct Squirrel *squirrels = (struct Squirrel *)malloc(num_squirrels * sizeof(struct Squirrel));
  int i = 0;

  /* Spawn actors*/
  for (i = 0; i < num_squirrels; i++)
  {
    *(squirrels + i) = Squirrel.new(5, 10, (int)5, 5000, 0.0, 0.0);
  }

  return squirrels;
}

// struct Actor *actor1 = (struct Actor *)malloc(sizeof(Actor));
// *actor1 = Actor.new(5, 5);
void delay(unsigned int secs)
{
  unsigned int retTime = time(0) + secs/10; // Get finishing time.
  while (time(0) < retTime)
    ; // Loop until it arrives.
}

//send_msg(rank, 0, MPI_INT, MPI_COMM_WORLD, &(squirrels->actor));

//  squirrels->actor.send_msg("a",rank,0,MPI_INT,MPI_COMM_WORLD,squirrels);
//  squirrels->actor.send_msg(1,rank,0,MPI_INT,MPI_COMM_WORLD,squirrels);
