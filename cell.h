


struct Cell {
    struct Actor actor; 
    int steps;
    int seed; 
    float pos_x;
    float pos_y;
    float influx;
    float pop;
    int (*getCellIDFromPosition) (float, float, struct Cell *);

};

extern const struct CellClass{
    struct Cell (*new)(int rank, int ID,float pos_x,float pos_y);
} Cell;

//static int getCellFromPosition(float, float, struct Cell *);
