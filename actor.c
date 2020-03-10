
#include "actor.h"
#include "stdio.h"

static double getAddress(struct Actor *this){
      return (this->address); 
}

static double sum(struct  Actor *this)
{
      return (this->value+this->value);
}
      
/*Create the new Actor*/
static struct Actor new(double addr,double val) {
	return (struct Actor){.address=addr ,.sum = &sum, .value=val, .getAddress= &getAddress};
}

const struct ActorClass Actor={.new=&new};

/***********************************************************************************************/
