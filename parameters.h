#ifndef PARAMETERS_H
#define PARAMETERS_H

#include "sack_object.h"

// structure for an individual in the genetic algorithm; the chromosomes are an array corresponding to each sack
// object, in order, where 1 means that the object is in the sack, 0 that it is not
typedef struct _parameters {
	sack_object *objects;
    int object_count;
    int sack_capacity;
    int generations_count;
    int threads;
    individual *current_generation;
    individual *next_generation;
    pthread_barrier_t *barrier;
} parameters;

typedef struct _thread_parameters {
	parameters *params;
    int tid;
} thread_parameters;


#endif