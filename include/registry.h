/**
 * @Author: B159973
 * @Date:	10/4/2019
 * @Course: Parallel Design Patterns - 2020
 * @University of Edinburgh
*/
#ifndef _REGISTRY_H
#define _REGISTRY_H

 /***************************************************************************//**
 * Cell structure represents a single Cell Actor 
 * @rank:       The worker rank that control these IDs that are in the registry cells
 * @actors_ID:  The IDs of the actors. 
 * @type_actor: The type of the actors that the worker is controlling. 0 is squirrels, 1 is cells.
 * @num_s:      The number of squirrels the worker is controllling.
 * @num_c:	    The number of cells the worker is controllling.
 * @next:       The next registry cell.
 ****************************************************************************/
typedef struct Registry_cell
{
  int rank;
  int  * actors_ID;
  int  type_actor;
  int num_s;
  int num_c;
  struct Registry_cell *next;
} Registry_cell;

/***************************************************************************//**
 * Assign the to registy cells the actors  
 * @param r:            Start of the registry list to append.
 * @param worker_rank:  Rank of the worker that controls the actors
 * @param start_s:      Start of the squirrel ID to insert into actors_ID.
 * @param end_s:        End of the squirrel ID to insert into actors_ID.
 * @param start_c:      Start of the cell ID to insert into actors_ID.
 * @param end_c:        End of the cell ID to insert into actors_ID.
 * @param actor_type:   The type of the actors that the worker is controlling. 0 is squirrels, 1 is cells.
 * @return:		          Nothing
 ******************************************************************************/
void assign_registry(struct Registry_cell **r, int worker_rank, int start_s, int end_s,
        int start_c, int end_c,int actor_type);

/***************************************************************************//**
 * Assign the to registy cells the actors  
 * @param registry:     Start of the registry list.
 * @param actor:        Actor type to look into the registry cell
 * @param ID:           Actor ID to look into the registry cell
 * @return:		          Actor ID we are looking for
 ******************************************************************************/        
int access_registry(Registry_cell *registry, int actor,int ID);

/***************************************************************************//**
 * Print the registry cell recursively.
 * @param head:     Start of the registry list.
 * @return:		          Nothing
 ******************************************************************************/ 
void print_register(struct Registry_cell *head);

#endif