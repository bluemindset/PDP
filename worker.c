
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
        //MPI_Bcast(&num_squirrels, 1, MPI_INT, _MASTER, MPI_COMM_WORLD);
        // MPI_Bcast(&num_cells, 1, MPI_INT, _MASTER, MPI_COMM_WORLD);
        int data[4];
        MPI_Recv(&data, 4, MPI_INT, _MASTER, _TAG_SQUIRRELS, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        //insert correct ids
        struct Squirrel *squirrels = spawnSquirrels(num_squirrels);
        struct Cells *cells = spawnCells(num_cells, data[2], data[3]);

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

struct Cell *spawnCells(int num_cells, int start_id, int end_id)
{
    struct Cell *cells = (struct Cells *)malloc(num_cells * sizeof(struct Cell));

    int i = 0;

    /* Spawn actors*/
    for (i = 0; i < num_cells; i++)
    {
        *(cells + i) = Cell.new(5, start_id, 0.0, 0.0);
        start_id++;
    }

    return cells;
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

  static void send_msg_sq(int _rank, int _tag, MPI_Datatype mpi_type, MPI_Comm comm, struct Squirrel *this)
  {
    MPI_Send(this->health, 1, mpi_type, _rank, _tag, comm);
  }



  void move(struct Squirrel * squirrels, long seed, int num_squirrels)
  {
    int i = 0;
    for (i = 0; i < num_squirrels; i++)
    {
      (squirrels + i)->squirrelStep(&seed, (squirrels + i));
      if (_DEBUG)
        print_pos(squirrels + i);
    }
  }
