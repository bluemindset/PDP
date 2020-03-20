
  void clock_work()
  {
    int day = 0;
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