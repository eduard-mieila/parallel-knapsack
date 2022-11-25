#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "par_genetic_algorithm.h"
#include <math.h>
#include <time.h>

int min(const int a, const int b) {
	return (a < b) ? a : b;
}

int read_input(sack_object **objects, int *object_count, int *sack_capacity, int *generations_count, int *threads, int argc, char *argv[])
{
	FILE *fp;

	if (argc < 4) {
		fprintf(stderr, "Usage:\n\t./tema1 in_file generations_count threads\n");
		return 0;
	}

	fp = fopen(argv[1], "r");
	if (fp == NULL) {
		return 0;
	}

	if (fscanf(fp, "%d %d", object_count, sack_capacity) < 2) {
		fclose(fp);
		return 0;
	}

	if (*object_count % 10) {
		fclose(fp);
		return 0;
	}

	sack_object *tmp_objects = (sack_object *) calloc(*object_count, sizeof(sack_object));

	for (int i = 0; i < *object_count; ++i) {
		if (fscanf(fp, "%d %d", &tmp_objects[i].profit, &tmp_objects[i].weight) < 2) {
			free(objects);
			fclose(fp);
			return 0;
		}
	}

	fclose(fp);

	*generations_count = (int) strtol(argv[2], NULL, 10);

	*threads = (int) atoi(argv[3]);
	
	if (*generations_count == 0) {
		free(tmp_objects);

		return 0;
	}

	*objects = tmp_objects;

	return 1;
}

void print_objects(const sack_object *objects, int object_count)
{
	for (int i = 0; i < object_count; ++i) {
		printf("%d %d\n", objects[i].weight, objects[i].profit);
	}
}

void print_generation(const individual *generation, int limit)
{
	for (int i = 0; i < limit; ++i) {
		for (int j = 0; j < generation[i].chromosome_length; ++j) {
			printf("%d ", generation[i].chromosomes[j]);
		}

		printf("\n%d - %d\n", i, generation[i].fitness);
	}
}

void print_best_fitness(const individual *generation)
{
	printf("%d\n", generation[0].fitness);
}

void compute_fitness_function(const sack_object *objects, individual *generation, int start, int end, int sack_capacity)
{
	int weight;
	int profit;

	for (int i = start; i < end; ++i) {
		weight = 0;
		profit = 0;
		// Pentru a castiga timp pierdut in cmpfunc la calcularea numarului de cromozomi,
		// vom face aceasta numaratoare o singura data, acum.
		generation[i].chromosome_count = 0;

		for (int j = 0; j < generation[i].chromosome_length; ++j) {
			if (generation[i].chromosomes[j]) {
				weight += objects[j].weight;
				profit += objects[j].profit;
				generation[i].chromosome_count++;
			}
		}


		generation[i].fitness = (weight <= sack_capacity) ? profit : 0;
	}
}

int cmpfunc(const void *a, const void *b)
{
	individual *first = (individual *) a;
	individual *second = (individual *) b;

	int res = second->fitness - first->fitness; // decreasing by fitness
	if (res == 0) {
		int first_count = first->chromosome_count;
		int second_count = second->chromosome_count;


		res = first_count - second_count; // increasing by number of objects in the sack
		if (res == 0) {
			return second->index - first->index;
		}
	}

	return res;
}

void mutate_bit_string_1(const individual *ind, int generation_index)
{
	int i, mutation_size;
	int step = 1 + generation_index % (ind->chromosome_length - 2);

	if (ind->index % 2 == 0) {
		// for even-indexed individuals, mutate the first 40% chromosomes by a given step
		mutation_size = ind->chromosome_length * 4 / 10;
		for (i = 0; i < mutation_size; i += step) {
			ind->chromosomes[i] = 1 - ind->chromosomes[i];
		}
	} else {
		// for even-indexed individuals, mutate the last 80% chromosomes by a given step
		mutation_size = ind->chromosome_length * 8 / 10;
		for (i = ind->chromosome_length - mutation_size; i < ind->chromosome_length; i += step) {
			ind->chromosomes[i] = 1 - ind->chromosomes[i];
		}
	}
}

void mutate_bit_string_2(const individual *ind, int generation_index)
{
	int step = 1 + generation_index % (ind->chromosome_length - 2);

	// mutate all chromosomes by a given step
	for (int i = 0; i < ind->chromosome_length; i += step) {
		ind->chromosomes[i] = 1 - ind->chromosomes[i];
	}
}

void crossover(individual *parent1, individual *child1, int generation_index)
{
	individual *parent2 = parent1 + 1;
	individual *child2 = child1 + 1;
	int count = 1 + generation_index % parent1->chromosome_length;

	memcpy(child1->chromosomes, parent1->chromosomes, count * sizeof(int));
	memcpy(child1->chromosomes + count, parent2->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));

	memcpy(child2->chromosomes, parent2->chromosomes, count * sizeof(int));
	memcpy(child2->chromosomes + count, parent1->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));
}

void copy_individual(const individual *from, const individual *to)
{
	memcpy(to->chromosomes, from->chromosomes, from->chromosome_length * sizeof(int));
}

void free_generation(individual *generation)
{
	int i;

	for (i = 0; i < generation->chromosome_length; ++i) {
		free(generation[i].chromosomes);
		generation[i].chromosomes = NULL;
		generation[i].fitness = 0;
	}
}

// In implementarea paralela, spre deosebire de schelet, vom imparti numarul
// de operatii in mod eegal intre threaduri. Astfel, vom calcula indici diferiti
// de start si end pentru initializarea/calcularea coeficientului fitness/asignarea
// unui noou index, copierea parintilor din generatia anterioara, mutatii si
// crossover.
void *par_genetic(void *arg) {

	// set some help pointers
	thread_parameters *t_params = (thread_parameters *) arg;
	parameters *params = t_params->params;
	individual *current_generation, *next_generation;

	current_generation = params->current_generation;
	next_generation = params->next_generation;

	// range of initializations
	int start = t_params->tid * (double)params->object_count / params->threads; 
	int end = min (params->object_count, (t_params->tid + 1) * (double)params->object_count / params->threads); 

	// set initial generation (composed of object_count individuals with a single item in the sack)
	for (int i = start; i < end; ++i) {
		current_generation[i].fitness = 0;
		current_generation[i].chromosomes = (int*) calloc(params->object_count, sizeof(int));
		current_generation[i].chromosomes[i] = 1;
		current_generation[i].index = i;
		current_generation[i].chromosome_length = params->object_count;

		next_generation[i].fitness = 0;
		next_generation[i].chromosomes = (int*) calloc(params->object_count, sizeof(int));
		next_generation[i].index = i;
		next_generation[i].chromosome_length = params->object_count;
	}

	// wait for all individuals to be loaded
	pthread_barrier_wait(params->barrier);


	for (int k = 0; k < params->generations_count; ++k) {
		int cursor = 0;

		// fitness computation 
		compute_fitness_function(params->objects, current_generation, start, end, params->sack_capacity);
		
		// wait for all calculations to be done
		pthread_barrier_wait(params->barrier);

		// sort done only by thread_0
		if (t_params->tid == 0) {
			qsort(current_generation, params->object_count, sizeof(individual), cmpfunc);
		}

		// wait for fit sort
		pthread_barrier_wait(params->barrier);

		// keep first 30% children, divided per threads
		int heritage_count = params->object_count * 3 / 10;
		int heritage_start = t_params->tid * (double) heritage_count / params->threads;
		int heritage_end = min(heritage_count, (t_params->tid + 1) * (double) heritage_count / params->threads);

		for (int i = heritage_start; i < heritage_end; i++) {
			copy_individual(current_generation + i, next_generation + i);
		}

		// mutate first 20% children with mutation_1, divided per threads
		cursor = heritage_count;
		int m1_count = params->object_count * 2 / 10;
		int m1_start = t_params->tid * (double) m1_count / params->threads;
		int m1_end = min(m1_count, (t_params->tid + 1) * (double) m1_count / params->threads);
		for (int i = m1_start; i < m1_end; i++) {
			copy_individual(current_generation + i, next_generation + cursor + i);
			mutate_bit_string_1(next_generation + cursor + i, k);
		}

		// mutate next 20% children with mutation_2, divided per threads
		cursor += m1_count;
		int m2_count = params->object_count * 2 / 10;
		int m2_start = t_params->tid * (double) m2_count / params->threads;
		int m2_end = min(m2_count, (t_params->tid + 1) * (double) m2_count / params->threads);
		for (int i = m2_start; i < m2_end; i++) {
			copy_individual(current_generation + m2_count + i, next_generation + cursor + i);
			mutate_bit_string_2(next_generation + cursor + i, k);
		}

		// crossover first 30% parents with one-point crossover
		// (if there is an odd number of parents, the last one is kept as such)
		cursor += m2_count;
		int cov_count = params->object_count * 3 / 10;
		if (cov_count % 2 == 1) {	
			if (t_params->tid == 0) {
				copy_individual(current_generation + params->object_count - 1, next_generation + cursor + cov_count - 1);
			}
			// Make sure count is even
			cov_count--;
		}

		int cov_start = t_params->tid * (double) cov_count / params->threads;
		int cov_end = min(cov_count - 1, (t_params->tid + 1) * (double) cov_count / params->threads);
		for (int i = cov_start; i < cov_end; i += 2) {
			crossover(current_generation + i, next_generation + cursor + i, k);
		}

		// wait for all threads to do the mutations and crossovers
		pthread_barrier_wait(params->barrier);

		// switch to new generation
		individual *tmp = current_generation;
		current_generation = next_generation;
		next_generation = tmp;

		for (int i = start; i < end; ++i) {
			current_generation[i].index = i;
		}

		// wait for all threads to compute index
		pthread_barrier_wait(params->barrier);

		if (k % 5 == 0 && t_params->tid == 0) {
			print_best_fitness(current_generation);
		}
	}

	compute_fitness_function(params->objects, current_generation, start, end, params->sack_capacity);
	// wait for check_print
	pthread_barrier_wait(params->barrier);



	// result announcement done only by thread_0
	if (t_params->tid == 0) {
		qsort(current_generation, params->object_count, sizeof(individual), cmpfunc);
		print_best_fitness(current_generation);
	}

	pthread_exit(NULL);
}