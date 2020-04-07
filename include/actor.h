/**
 * @Author: B159973
 * @Date:	10/4/2019
 * @Course: Parallel Design Patterns - 2020
 * @University of Edinburgh
*/
#ifndef _ACTOR_H
#define _ACTOR_H

 /***************************************************************************//**
 * Actor structure is the basic abstract class for every actor. 
 * 
 * @rank:   The rank of the worker the structure is handled by.
 * @ID:		The identification number of the actor.
 * ****************************************************************************/
 struct Actor{
    int rank;
    int ID;
    double (*getRank)(struct Actor *this);
    int (*getID)(struct Actor *this);
};
/***************************************************************************//**
 * Constructor of the Actor Class. 
 ******************************************************************************/
extern const struct ActorClass{
    struct Actor (*new)(int rank,int ID);
} Actor;

/***************************************************************************//**
 * Get's the Rank of the worker the actor is hanlded by
 * @param this: A pointer to the Actor structure. 
 * @return:		The rank of the actor structure
 ******************************************************************************/
static double getRank(struct Actor *this);

/***************************************************************************//**
 * Get's the ID of the worker the actor is hanlded by
 * @param this: A pointer to the Actor structure. 
 * @return:		The ID of the actor structure
 ******************************************************************************/
static int getID(struct  Actor *this);

#endif

