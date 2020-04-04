
int  cells_work(struct Cell *cell, int rank, struct Registry_cell *registry,int * month, MPI_Request * r,int work)
{
  int forever = 1;
  int seed = 0;
  int influx, pop, f = 0, s_health;

  MPI_Status status;
  /*Check if there is message from any other actor and act accordiclty*/

  /*Check the TAG*/
    if (work == 1)
    {
      int tag = cell->actor.ID * _TAG_SQUIRRELS;
      MPI_Request request_inflix;
      MPI_Request request_pop;
      //IMPLEMENT EVERYHTING WITH A TAG
      int recv_data[2];
      int send_data[2] = {(cell->influx), (cell->pop)};
      
      /*Send the inlfux and population values to the incoming squirrel */
      MPI_Recv(&recv_data, 2, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
      // printf("[Worker]Cell ID %d Received Data from %d  \n",cell->actor.ID,status.MPI_TAG);

      MPI_Send(&send_data, 2, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD);
      // printf("[Worker]%d %d \n",send_data[0],send_data[1]);
      if(_DEBUG)
        printf("[Worker]Cell ID %d Sending Data to %d  \n", cell->actor.ID, status.MPI_TAG);

      /*Receive health value from the squierrel*/
       if (recv_data[0])
         cell->month_stats->squirrels_healthy++;
       else
         cell->month_stats->squirrels_unhealthy++;

      if(_DEBUG)   
        printf("[Worker]Cell ID %d Sending Data to %d  \n", cell->actor.ID, status.MPI_TAG);

    }
    if (if_clock_msg(status, cell->actor.ID))
    {
      /* Perfome update of the cell influx and population values*/
      /*This means that a month passed and the squirrels have to be updated*/
     
     // delay(3);
     
    //  if(_DEBUG)
    //  printf("[Worker]Cell ID %d issued receive %d  \n", rank, status.MPI_TAG);
      MPI_Irecv(month, 1, MPI_INT, _MASTER, _TAG_CLOCK+cell->actor.ID, MPI_COMM_WORLD,r );


    
      return 1;
      //MPI_Irecv(&data_recv,2,MPI_INT, _MASTER, _TAG_CLOCK+cell->actor.ID,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
      /* Update the cells by sending them back their influx they need to store*/
      //update_cells(cell, month);
      /*Update individual cell data*/
      /*implement return */
    }
    return 0;
    //forever = 0; //should_terminate_worker();
  }
}


void update_cell(){

}
