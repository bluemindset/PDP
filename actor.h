 struct Actor{
    double address;
    double value;
    double (*getAddress)(struct Actor *this);
    double (*sum)(struct Actor *this);
};

/*Constructor*/
extern const struct ActorClass{
    struct Actor (*new)(double addr,double val);
} Actor;

//static double sum(struct  Actor *this);

