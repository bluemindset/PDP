#ifndef CELL_H_
#define CELL_H_



struct Day_stat {
    int squirrels_healthy;
    int squirrels_unhealthy;
    struct Day_stat * nextstat;
}Day_stat;


struct Cell {
    struct Actor actor; 
    int ID;
    int seed; 
    float pos_x;
    float pos_y;
    float influx;
    float pop;
    int (*getCellIDFromPosition) (float, float, struct Cell *);
    int * squirrels_month;
    struct Day_stat * day_stats;
    int c;
};


extern const struct CellClass{
    struct Cell (*new)(int rank, int ID,float pos_x,float pos_y);
} Cell;


void update_cells(struct Cell *cell, int day);



#endif