#ifndef KITCHEN_H
#define KITCHEN_H

#include "order_sem.h"
#include "queue.h"
#include <pthread.h>

//Station enumeration
typedef enum {PREP, OVEN, STOVE, SINK, N_STATIONS, OUTSIDE} station_id;

//Recipe Step
typedef struct {
	station_id station;
	int duration;
} recipe_step_t;

//Construct a Recipe Step
recipe_step_t recipe_step_init(station_id station, unsigned int time);

//Sequence of recipe steps. Behaves like a bag data type
typedef struct {
	int id;
	int nsteps;
	recipe_step_t *steps;
} recipe_t;


//Initialize empty recipe_t
recipe_t recipe_init();

//Initialize empty recipe_t
recipe_t recipe_init2(int id);

//Add step to the specified recipe (in order)
int recipe_add_step(recipe_t *recipe, recipe_step_t step);

//Free recipe content
void recipe_free(recipe_t *recipe);

//Print recipe content
void print_recipe(recipe_t recipe);

int load_recipes(recipe_t recipes[], unsigned int size, const char* filename);

typedef struct{
	int recipe_id;
	int number;
} order_t;
define_queue(order_t);//queue_order_t

typedef struct {
	int id;
	order_t order;//current order
	station_id station;//current station
} chef_t;

//one per chef. Store the movement intetions
typedef struct {
	int link [N_STATIONS][N_STATIONS];//row: from; col: to
} intention_t;

void add_intention(intention_t* intention, station_id from, station_id to);

void rem_intention(intention_t* intention, station_id from, station_id to);

void print_intention(intention_t intent, int chef);

//Store information about the stations (implemented with semaphores)
typedef struct  {
	order_sem_t station_sem[N_STATIONS];
	order_sem_t sleep_sem[N_STATIONS];
	chef_t chef[N_STATIONS];
} kitchen_t;


void init_kitchen(kitchen_t *kitchen);

void free_kitchen(kitchen_t *kitchen);

void print_kitchen(kitchen_t kitchen);

//Store information about the stations (implemented with mutexes/condition variables)
typedef struct  {
	pthread_mutex_t sleep_mtx[N_STATIONS];
	pthread_cond_t sleep_cv[N_STATIONS];
	chef_t chef[N_STATIONS];
} kitchen2_t;

void init_kitchen2(kitchen2_t *kitchen);

void free_kitchen2(kitchen2_t *kitchen);

void print_kitchen2(kitchen2_t kitchen);

#endif
