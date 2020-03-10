#include "actor.h"
#include "squirrel.h"
#include "process_pool.h"
#include "stdio.h"
#include "mpi.h"
#include <time.h> 


static void worker();
void masterlives();

void masterlives(){
      int masterStatus = receiving_handle();
      while(masterStatus){
        masterStatus = receiving_handle();
      }
}

int main(int argc, char *argv[]){
    double x = 5;

    /****Initialize MPI****/
    MPI_Init(NULL, NULL);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int statusCode = create_pool();
  
      if (statusCode ==1 ){
      /*Give work to a worker*/  
        worker();
      }
      else if (statusCode == 2){  
        //Send package to workers 
        int workerPid = startWorkerProcess();
        /*Master sends the number of squirrels*/
        /*The tags for exchange the messages are 0 */
        int number_squirrel = 5;
        printf("PID%d",workerPid);
        MPI_Send(&number_squirrel, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
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


void delay(int number_of_seconds) 
{ 
    // Converting time into milli_seconds 
    int milli_seconds = 1000 * number_of_seconds; 
  
    // Storing start time 
    clock_t start_time = clock(); 
  
    // looping till required time is not achieved 
    while (clock() < start_time + milli_seconds); 

} 
  
static void worker(){
    
    int workerStatus = 1; 
    int parentID;
    int num_squirrels;
    /*Worker here must wait to receive a message*/
    while(workerStatus){
     // parentID = incoming_msg.context;
     // startWorkerProcess();
      
      /*Send a message to master to start*/
      
      /* Receive message from the master to start the work*/
      MPI_Recv(&num_squirrels, 1, MPI_INT, _MASTER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      printf("SQUER%d",num_squirrels);
      struct Squirrel* squirrels = (struct Squirrel*)malloc(num_squirrels * sizeof(struct Squirrel)); 
      struct Actor* actor1 = (struct Actor*)malloc(sizeof(Actor));
      *actor1 = Actor.new(5,5);
      int i ;
      for (i =0 ;i< num_squirrels;i++){     
         /* Spawn actors*/
        *(squirrels+i) = Squirrel.new(5,10,(int)5);
     }
       int attemps = 5;
       while(attemps>0){
          int sum = squirrels->actor.sum(&(squirrels->actor));
          int sum1 = actor1->sum(actor1); 
          printf("\nsum: |%d",sum);
          delay(1);
          attemps--;
       }

     workerStatus = workerSleep();

    }


}