

typedef struct Day {
    int squirrels_healthy;
    int squirrels_unhealthy;
    Day* nextday;
}Day;

struct Cell {
    struct Actor actor; 
    int steps;
    int seed; 
    float pos_x;
    float pos_y;
    int influx;
    int pop;
    int (*getCellIDFromPosition) (float, float, struct Cell *);
    int * squirrels_month;
    Day * squirrels_day;
    int c;
};

extern const struct CellClass{
    struct Cell (*new)(int rank, int ID,float pos_x,float pos_y);
} Cell;

//static int getCellFromPosition(float, float, struct Cell *);
