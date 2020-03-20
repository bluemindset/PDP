
struct Squirrel {
    /*Declaring actor fields*/
    struct Actor actor; 
    int steps;
    int seed; 
    int health; 
    float pos_x;
    float pos_y;
    double influx[50];
    double pop[50];
    double avg_influx;
    double avg_pop;
    /*Declaring functions*/
    void (*squirrelStep)(long *,struct Squirrel*);
    int (*willGiveBirth)( long *,struct Squirrel*);
    int (*willCatchDisease)( long *,struct Squirrel*);
    int (*willDie)(long *,struct Squirrel*);
    void (*update_avgs)(int influx, int pop, struct Squirrel *this);

};

extern const struct SquirrelClass{
    struct Squirrel (*new)(int rank, int ID, int steps,int seed,float pos_x,float pos_y);
} Squirrel;

//static void initialiseRNG(long *,struct Squirrel *);
