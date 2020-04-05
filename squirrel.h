#ifndef _SQUIRREL_H
#define _SQUIRREL_H

struct Squirrel {
    /*Declaring actor fields*/
    struct Actor actor; 
    int steps;
    int seed; 
    int health; 
    float pos_x;
    float pos_y;
    int influx[50];
    int pop[50];
    double avg_influx;
    double avg_pop;
    /*Declaring functions*/
    // void (*squirrelStep)(float x, float y, float *x_new, float *y_new, long *state);

    // int (*willGiveBirth)
    // int (*willCatchDisease)( long *,struct Squirrel*);
    // int (*willDie)(long *,struct Squirrel*);
    void (*update_avgs)(int influx, int pop, struct Squirrel *this);

};

extern const struct SquirrelClass{
    struct Squirrel (*new)(int rank, int ID, int steps,int seed,float pos_x,float pos_y);
} Squirrel;

void squirrels_work(struct Squirrel * squirrel, int rank, struct Registry_cell *registry, int  data[2],int * alive,MPI_Request * r);
int squirrel_life(struct Squirrel *squirrel, int influx, int pop, int *num_squirrels,int rank);

/*Update the averages of the squirrels*/
static void update_avgs(int influx, int pop, struct Squirrel *this);

/* Squirrel Routine 
      1. Move 
      2. Send Message to cell health status and wait until successfull
      3. Caclulate prop die 
      4. Caclulate prop born 
     */
    
void store_squirrel(int recvID, int *squirrels_IDs_healthy, int *squirrels_IDs_unhealthy,int health);
void init_squirrel_stats (int *squirrels_IDs_healthy, int *squirrels_IDs_unhealthy);
void print_stat_squirrels(int *squirrels_IDs_healthy, int *squirrels_IDs_unhealthy, int month, int rank);
void selection_sort(int * a);
int  find_max(int * a, int high);
void swap(int * a, int p1, int p2);


/**
 * Simulates the step of a squirrel. You can call this with the arguments (0,0,&x,&y,&state)
 * to determine a random initial starting point.
 * x_new and y_new are the new x and y coordinates, state is used in the random number
 * generator and is also modified.
 * x_new can point to x, and y_new can point to y
 */
void squirrelStep(float, float, float *, float *, long *);

/**
 * Determines whether a squirrel will give birth or not based upon the average population and a random seed
 * which is modified. You can enclose this function call in an if statement if that is useful.
 */
int willGiveBirth(float, long *);

/**
 * Determines whether a squirrel will catch the disease or not based upon the average infection level
 * and a random seed which is modified. You can enclose this function call in an if statement if that is useful.
 */
int willCatchDisease(float, long *);

/**
 * Determines if a squirrel will die or not. The state is used in the random number generation and
 * is modified. You can enclose this function call in an if statement if that is useful.
 */
int willDie(long *);


#endif