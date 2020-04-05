#ifndef CELL_H_
#define CELL_H_



struct month_stat {
    int squirrels_healthy;
    int squirrels_unhealthy;
    struct month_stat * nextstat;
}month_stat;


struct Cell {
    struct Actor actor; 
    int ID;
    int seed; 
    float pos_x;
    float pos_y;
    int influx;
    int pop;
    int (*getCellIDFromPosition) (float, float, struct Cell *);
    int  squirrels_healthy[24];
    int  squirrels_unhealthy[24];
    struct month_stat * month_stats;
    int c;
};


extern const struct CellClass{
    struct Cell (*new)(int rank, int ID,float pos_x,float pos_y);
} Cell;


void update_cell(struct Cell *cell, int month, int rank,int * stats,int cellID);



#endif