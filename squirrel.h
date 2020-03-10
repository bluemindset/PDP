
struct Squirrel {
    struct Actor actor; 
    int steps;
};

extern const struct SquirrelClass{
    struct Squirrel (*new)(double address, double value, int steps);
} Squirrel;
