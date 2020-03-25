#ifndef _CLOCK_H
#define _CLOCK_H


typedef struct Day {
    int squirrels_healthy;
    int squirrels_unhealthy;
    float avg_influx; 
    float avg_pop;
    struct Day * nextday;
}Day;

struct Clock {
    struct Actor actor; 
    int ID;
    int rank;
    Day * timeline;
};

extern const struct ClockClass{
    struct Clock (*new)(int rank, int ID);
} Clock;



void clock_work();
void delay(unsigned int secs);

#endif