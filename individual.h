#ifndef INDIVIDUAL_H
#define INDIVIDUAL_H

// structure for an individual in the genetic algorithm; the chromosomes are an array corresponding to each sack
// object, in order, where 1 means that the object is in the sack, 0 that it is not
typedef struct _individual {
	int fitness;
	int *chromosomes;
    int chromosome_length;
	int index;
	// Introducem o variabila noua in aceasta structura pentru a castiga timp pierdut pentru
	// calcularea numarului de cromozomi in timpul sortarii. In implementarea paralele vom
	// calcula acest numar *O SINGURA DATA* per generatie, per element, si nu de fiecare data
	// cand de apeleaza cmpfunc.
	int chromosome_count;
} individual;

#endif