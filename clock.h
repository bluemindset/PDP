#ifndef _CLOCK_H
#define _CLOCK_H


typedef struct month {
    int * squirrels_healthy;
    int * squirrels_unhealthy;
    float avg_influx; 
    float avg_pop;
    struct month * nextmonth;
    int ID;
}month;

struct Clock {
    struct Actor actor; 
    int ID;
    int rank;
    month * timeline;
};

extern const struct ClockClass{
    struct Clock (*new)(int rank, int ID);
} Clock;



void clock_work(Registry_cell *r, int workers_size, struct Clock *clock);
void delaySquirrel(int delay);
void delayClock(unsigned int secs);

#endif