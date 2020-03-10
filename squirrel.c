#include "actor.h"
#include "squirrel.h"

static struct Squirrel new(double addr,double val,int steps) {
	struct Squirrel squirrel={.steps = steps };
      squirrel.actor = Actor.new(addr, val);
      return squirrel; 
}

const struct SquirrelClass Squirrel={.new=&new};
