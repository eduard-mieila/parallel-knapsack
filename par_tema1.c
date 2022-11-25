#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "par_genetic_algorithm.h"
#include "parameters.h"

int main(int argc, char *argv[]) {
	parameters params;

	// array with all the objects that can be placed in the sack
	params.objects = NULL;

	// number of objects
	params.object_count = 0;

	// maximum weight that can be carried in the sack
	params.sack_capacity = 0;

	// number of generations
	params.generations_count = 0;

	// read input
	if (!read_input(&params.objects, &params.object_count, &params.sack_capacity, &params.generations_count, &params.threads, argc, argv)) {
		return 0;
	}

	thread_parameters tparams[params.threads];
	pthread_t tid[params.threads];
	pthread_barrier_t barrier;

	pthread_barrier_init(&barrier, NULL, params.threads);
	params.current_generation = (individual*) calloc(params.object_count, sizeof(individual));
	params.next_generation = (individual*) calloc(params.object_count, sizeof(individual));
	params.barrier = &barrier;

	for (int i = 0; i < params.threads; i++) {
		tparams[i].params = &params;
		tparams[i].tid = i;
		pthread_create(&tid[i], NULL, par_genetic, &tparams[i]);
	}


	// se asteapta thread-urile
	for (int i = 0; i < params.threads; i++) {
		pthread_join(tid[i], NULL);
	}

	pthread_barrier_destroy(&barrier);

	// free resources for old generation
	free_generation(params.current_generation);
	free_generation(params.next_generation);

	// free resources
	free(params.current_generation);
	free(params.next_generation);

	free(params.objects);

	return 0;
}
