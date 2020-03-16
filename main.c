
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
#define S_TAG 1


void squirrels_work( struct Squirrel *squirrels, int num_squirrels, int rank, _registry_cell * registry );
typedef struct _registry_cell{

int rank;
float pos_x; 
float pos_y;
int ID; 

}_registry_cell;


static void worker(int, _registry_cell * );
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



int main(int argc, char *argv[])
{
  double x = 5;
  
  /****Initialize MPI****/
  MPI_Init(NULL, NULL);

  int rank;
  int size; 
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int * workers = (int *)malloc(size * sizeof(int));
  int statusCode = create_pool();

  _registry_cell * registry = (_registry_cell *)malloc(size * sizeof(_registry_cell));

  if (statusCode == 1)
  {
    /*Give work to a worker*/
    worker(rank,registry);
  }
  else if (statusCode == 2)
  {

    /*Worker determines the jobs to give not other workers*/
    startworkers(size-1,workers);
    /*Master sends the number of squirrels*/
    /*The tags for exchange the messages are 0 */
    int number_squirrel = 5;
    int number_cells = 5;

    MPI_Bcast(&number_squirrel, 1, MPI_INT, _MASTER, MPI_COMM_WORLD);
    MPI_Bcast(&number_cells, 1, MPI_INT, _MASTER, MPI_COMM_WORLD);

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

void startworkers(int num_workers, int* workers){
  int i = 0 ; 
  for (i =0 ; i < num_workers ; i++){
    workers[i] = startWorkerProcess();
    if (workers[i]== NULL)
      exit(0);
  }

}





static void worker(int rank, _registry_cell * registry )
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

    
    while (i < 10) 
    {
      squirrels_work(squirrels, num_squirrels, rank, registry);


      send_msg(rank,0,MPI_INT,MPI_COMM_WORLD,&(squirrels->actor));


    //  squirrels->actor.send_msg("a",rank,0,MPI_INT,MPI_COMM_WORLD,squirrels);
    //  squirrels->actor.send_msg(1,rank,0,MPI_INT,MPI_COMM_WORLD,squirrels);
      ++i;
    }
    send_command(_COMPLETE, _MASTER, 0);
    workerStatus = workerSleep();
  }
}

void print_pos(struct Squirrel *this)
{
  printf("\nSquirrel ID:%d, pos X:%f ,pos Y:%f ", this->actor.getID(this), this->pos_x, this->pos_y);
}

int access_registry(float pos_x,float pos_y, _registry_cell * registry ){
int i ,j;
      for(i = 0 ; i <MAX_REG;i++)
        if (registry->pos_x == pos_x && registry->pos_y == pos_y ) 
            return registry->ID;
}

/* Squirrel Routine 
    
      1. Move 
      2. Send Message to cell health status and wait until successfull
      3. Caclulate prop die 
      4. Caclulate prop born 
     */
void squirrels_work( struct Squirrel *squirrel, int rank, _registry_cell * registry ){
      int seed = 0 ;
      int influx ;
      int pop;  
      //move(squirrels);
      (squirrel)->squirrelStep(&seed, (squirrel));
      int cell_ID = access_registry(squirrel->pos_x,squirrel->pos_y,registry);  
      
      /*Send health*/
      send_msg_sq(registry[cell_ID].rank,S_TAG,MPI_INT,MPI_COMM_WORLD,&(squirrel->actor));

      
      MPI_Recv(&influx, 1,MPI_INT, MPI_ANY_SOURCE,S_TAG,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      MPI_Recv(&pop, 1,MPI_INT, MPI_ANY_SOURCE,S_TAG,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      
      squirrel->update_avgs(influx,pop);
      squirrel->willCatchDisease(cell_ID, seed, squirrel);
      squirrel->willGiveBirth(seed, squirrel);
      squirrel->willDie(seed, squirrel);
      

}


static void send_msg_sq (int _rank, int _tag, MPI_Datatype mpi_type, MPI_Comm comm,struct Squirrel * this)
{
    MPI_Send(this->health, 1, mpi_type,_rank,_tag,comm);  
}



struct Cell *spawnCells(int num_cells)
{
  struct Cell *cells = (struct Cells *)malloc(num_cells * sizeof(struct Cell));
  
  int i = 0;

  /* Spawn actors*/
  for (i = 0; i < num_cells; i++)
  {
    *(cells + i) = Cell.new(5, i,0.0,0.0);
  }

  return cells;
}

void move(struct Squirrel * squirrels, long seed, int num_squirrels)
{
  int i = 0;
  for (i = 0; i < num_squirrels; i++){
    (squirrels + i)->squirrelStep(&seed, (squirrels + i));
  if(_DEBUG) 
    print_pos(squirrels+i);
  }
}

struct Squirrel * spawnSquirrels(int num_squirrels)
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
 void delay(int number_of_seconds)
{
  // Converting time into milli_seconds
  int milli_seconds = 10000000 * number_of_seconds;

  // Storing start time
  clock_t start_time = clock();

  // looping till required time is not achieved
  while (clock() < start_time + milli_seconds)
    ;
}