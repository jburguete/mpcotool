/*
Calibrator: a software to make calibrations of empirical parameters.

AUTHORS: Javier Burguete and Borja Latorre.

Copyright 2012-2014, AUTHORS.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice,
		this list of conditions and the following disclaimer in the
		documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY AUTHORS ``AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
SHALL AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
*/

/**
 * \file calibrator.c
 * \brief Source file of the calibrator.
 * \authors Javier Burguete and Borja Latorre.
 * \copyright Copyright 2012-2014, all rights reserved.
 */
#define _GNU_SOURCE
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <gsl/gsl_rng.h>
#include <libxml/parser.h>
#include <glib.h>
#ifdef G_OS_WIN32
	#include <windows.h>
#elif (!defined(NetBSD))
	#include <alloca.h>
#endif
#ifdef HAVE_MPI
	#include <mpi.h>
#endif
#ifdef HAVE_GAUL
	#include "gaul.h"
#endif

/**
 * \def DEBUG
 * \brief Macro to debug.
 */
#define DEBUG 0

/**
 * \enum CalibrateAlgorithm
 * \brief Enum to define the calibration algorithm.
 */
enum CalibrateAlgorithm
{
	CALIBRATE_ALGORITHM_MONTE_CARLO = 0,
	CALIBRATE_ALGORITHM_SWEEP = 1,
	CALIBRATE_ALGORITHM_GENETIC = 2
};

/**
 * \struct Calibrate
 * \brief Struct to define the calibration data.
 */
typedef struct
{
/**
 * \var simulator
 * \brief Name of the simulator program.
 * \var evaluator
 * \brief Name of the program to evaluate the objective function.
 * \var template
 * \brief Matrix of template names of input files.
 * \var experiment
 * \brief Array experimental data file names.
 * \var label
 * \brief Array of variable names.
 * \var format
 * \brief Array of variable formats.
 * \var nvariables
 * \brief Variables number.
 * \var nexperiments
 * \brief Experiments number.
 * \var ninputs
 * \brief Number of input files to the simulator.
 * \var nsimulations
 * \brief Simulations number per experiment.
 * \var algorithm
 * \brief Algorithm number
 * \var nsweeps
 * \brief Array of sweeps of the sweep algorithm.
 * \var nstart
 * \brief Beginning simulation number of the task.
 * \var nend
 * \brief Ending simulation number of the task.
 * \var nthreads
 * \brief Number of threads.
 * \var thread
 * \brief Array of simulation numbers to calculate on the thread.
 * \var niterations
 * \brief Number of algorithm iterations
 * \var nbests
 * \brief Number of best simulations.
 * \var nsaveds
 * \brief Number of saved simulations.
 * \var simulation_best
 * \brief Array of best simulation numbers.
 * \var population
 * \brief Number of individuals.
 * \var generations
 * \brief Number of generations.
 * \var bits
 * \brief Number of bits representing each variable.
 * \var value
 * \brief Array of variable values.
 * \var rangemin
 * \brief Array of minimum variable values.
 * \var rangemax
 * \brief Array of maximum variable values.
 * \var error_best
 * \brief Array of best minimum errors.
 * \var tolerance
 * \brief Algorithm tolerance.
 * \var crossover
 * \brief Crossover probability.
 * \var mutation
 * \brief Mutation probability.
 * \var result
 * \brief Result file.
 * \var file
 * \brief Matrix of input template files.
 * \var mpi_rank
 * \brief Number of MPI task.
 * \var mpi_tasks
 * \brief Total number of MPI tasks.
 */
	char *simulator, *evaluator, **experiment, **template[4], **label, **format;
	unsigned int nvariables, nexperiments, ninputs, nsimulations, algorithm,
		*nsweeps, nstart, nend, nthreads, *thread, niterations, nbests, nsaveds,
		*simulation_best, population, generations, bits;
	double *value, *rangemin, *rangemax, *error_best, tolerance,
		crossover, mutation;
	FILE *result;
	GMappedFile **file[4];
#ifdef HAVE_MPI
	int mpi_rank, mpi_tasks;
#endif
} Calibrate;

/**
 * \struct ParallelData
 * \brief Struct to pass to the GThreads parallelized function.
 */
typedef struct
{
/**
 * \var thread
 * \brief Thread number.
 * \var calibrate
 * \brief Calibration data pointer.
 */
	unsigned int thread;
	Calibrate *calibrate;
} ParallelData;

/**
 * \var void (*calibrate_step)(Calibrate*)
 * \brief Pointer to the function to perform a calibration algorithm step.
 */
void (*calibrate_step)(Calibrate*);

/**
 * \var rng
 * \brief Pseudo-random numbers generator struct.
 */
gsl_rng *rng;

/**
 * \var mutex
 * \brief Mutex struct.
 */
GMutex mutex;

/**
 * \fn int omp_thread_count()
 * \brief Function to get the OpenMP threads.
 * \return OpenMP threads number.
 */
int omp_thread_count()
{
    int n = 0;
    #pragma omp parallel reduction(+:n)
    n += 1;
    return n;
}

#ifdef HAVE_GAUL
int ga_variables;			// Number of variables
int ga_variable_bits;		// Bits representing each variable
int ga_bits;				// Chromosome bits ( nv * bs )
double *ga_variable_max;	// Max value for each variable
double *ga_variable_min;	// Min value for each variable
Calibrate *ga_calibrate;	// Auxiliary Calibrate struct

// Get variable i from chromosome
double ga_get_variable( byte *chromosome, int i )
{
	int j;
	boolean bin;
	double var = 0.0, max = 0.0;

	if( i >= 0 && i < ga_variables )
	{
		for( j=0 ; j<ga_variable_bits ; j++ )
		{
			bin = ga_bit_get(chromosome, j + i * ga_variable_bits );
			if( bin )
			{
				var = var * 2.0 + 1.0;
				max = max * 2.0 + 1.0;
			}
			else
			{
				var *= 2.0; 
				max = max * 2.0 + 1.0;
			}
		}
		return ga_variable_min[i] +
			var / max * ( ga_variable_max[i] - ga_variable_min[i] );
	}

	return 0.0;
}

// Set lowest and highest values of variable i
int ga_set_variable_min_max( int i, double min, double max )
{
	if( i >= 0 && i < ga_variables )
	{
		ga_variable_min[i] = min;
		ga_variable_max[i] = max;
		return 0;
	}
	return 1;
}

// Set number of variables and bits representing each one
int ga_set_variables( int n, int bits )
{
	int i;
	if( n > 0 && bits > 0 )
	{
		ga_variables = n;
		ga_variable_bits = bits;
		ga_bits = n * bits;
		ga_variable_min = (double *)malloc(n * sizeof (double));
		ga_variable_max = (double *)malloc(n * sizeof (double));
		for( i=0 ; i<n ; i++ )
		{
			ga_set_variable_min_max( i, 0.0, 1.0 );
		}
		return 0;
	}
	return 1;
}
#endif


/**
 * \fn void calibrate_input(Calibrate *calibrate, unsigned int simulation, \
 *   char *input, GMappedFile *template)
 * \brief Function to write the simulation input file.
 * \param calibrate
 * \brief Calibration data.
 * \param simulation
 * \brief Simulation number.
 * \param input
 * \brief Input file name.
 * \param template
 * \brief Template of the input file name.
 */
void calibrate_input(Calibrate *calibrate, unsigned int simulation,
	char *input, GMappedFile *template)
{
	unsigned int i;
	char buffer[32], value[32], *buffer2, *buffer3, *content;
	FILE *file;
	gsize length;
	GRegex *regex;

#if DEBUG
printf("calibrate_input: start\n");
#endif

	// Opening template
	content = g_mapped_file_get_contents(template);
	length = g_mapped_file_get_length(template);
#if DEBUG
printf("calibrate_input: length=%lu\ncontent:\n%s", length, content);
#endif
	file = fopen(input, "w");

	// Parsing template
	for (i = 0; i < calibrate->nvariables; ++i)
	{
#if DEBUG
printf("calibrate_input: variable=%u\n", i);
#endif
		snprintf(buffer, 32, "@variable%u@", i + 1);
		regex = g_regex_new(buffer, 0, 0, NULL);
		if (i == 0)
		{
			buffer2 = g_regex_replace_literal(regex, content, length, 0,
				calibrate->label[i], 0, NULL);
#if DEBUG
printf("calibrate_input: buffer2\n%s", buffer2);
#endif
		}
		else
		{
			length = strlen(buffer3);
			buffer2 = g_regex_replace_literal(regex, buffer3, length, 0,
				calibrate->label[i], 0, NULL);
			g_free(buffer3);
		}
		g_regex_unref(regex);
		length = strlen(buffer2);
		snprintf(buffer, 32, "@value%u@", i + 1);
		regex = g_regex_new(buffer, 0, 0, NULL);
		snprintf(value, 32, calibrate->format[i],
		calibrate->value[simulation * calibrate->nvariables + i]);

#if DEBUG
printf("calibrate_parse: value=%s\n", value);
#endif
		buffer3 = g_regex_replace_literal(regex, buffer2, length, 0, value,
			0, NULL);
		g_free(buffer2);
		g_regex_unref(regex);
	}

	// Saving input file
	fwrite(buffer3, strlen(buffer3), sizeof(char), file);
	g_free(buffer3);
	fclose(file);

#if DEBUG
printf("calibrate_input: end\n");
#endif
}

/**
 * \fn double calibrate_parse(Calibrate *calibrate, unsigned int simulation, \
 *   unsigned int experiment)
 * \brief Function to parse input files, simulating and calculating the \
 *   objective function.
 * \param calibrate
 * \brief Calibration data.
 * \param simulation
 * \brief Simulation number.
 * \param experiment
 * \brief Experiment number.
 * \return Objective function value.
 */
double calibrate_parse(Calibrate *calibrate, unsigned int simulation,
	unsigned int experiment)
{
	unsigned int i;
	double e;
	char buffer[512], input[4][32], output[32], result[32];
	FILE *file_result;

#if DEBUG
printf("calibrate_parse: start\n");
printf("calibrate_parse: simulation=%u experiment=%u\n", simulation,
experiment);
#endif

	// Opening input files
	for (i = 0; i < calibrate->ninputs; ++i)
	{
		snprintf(&input[i][0], 32, "input-%u-%u-%u", i, simulation, experiment);
#if DEBUG
printf("calibrate_parse: i=%u input=%s\n", i, &input[i][0]);
#endif
		calibrate_input(calibrate, simulation, &input[i][0],
			calibrate->file[i][experiment]);
	}
	for (; i < 4; ++i) snprintf(&input[i][0], 32, "");
#if DEBUG
printf("calibrate_parse: parsing end\n");
#endif

	// Performing the simulation
	snprintf(output, 32, "output-%u-%u", simulation, experiment);
	snprintf(result, 32, "result-%u-%u", simulation, experiment);
	snprintf(buffer, 512, "./%s %s %s %s %s %s", calibrate->simulator,
		&input[0][0], &input[1][0], &input[2][0], &input[3][0], output);
#if DEBUG
printf("calibrate_parse: %s\n", buffer);
#endif
	system(buffer);

	// Checking the objective value function
	snprintf(buffer, 512, "./%s %s %s %s", calibrate->evaluator, output,
		calibrate->experiment[experiment], result);
#if DEBUG
printf("calibrate_parse: %s\n", buffer);
#endif
	system(buffer);
	file_result = fopen(result, "r");
	e = atof(fgets(buffer, 512, file_result));
	fclose(file_result);

	// Removing files
#if !DEBUG
	snprintf(buffer, 512, "rm %s %s %s %s %s %s", &input[0][0], &input[1][0],
		&input[2][0], &input[3][0], output, result);
	system(buffer);
#endif

#if DEBUG
printf("calibrate_parse: end\n");
#endif

	// Returning the objective function
	return e;
}

/**
 * \fn void calibrate_best_thread(Calibrate *calibrate, \
 *   unsigned int simulation, double value)
 * \brief Function to save the bests simulations of a thread.
 * \param calibrate
 * \brief Calibration data.
 * \param simulation
 * \brief Simulation number.
 * \param value
 * \brief Objective function value.
 */
void calibrate_best_thread(Calibrate *calibrate, unsigned int simulation,
	double value)
{
	unsigned int i, j;
	double e;
#if DEBUG
printf("calibrate_best_thread: start\n");
#endif
	if (calibrate->nsaveds < calibrate->nbests
		|| value < calibrate->error_best[calibrate->nsaveds - 1])
	{
		g_mutex_lock(&mutex);
		if (calibrate->nsaveds < calibrate->nbests) ++calibrate->nsaveds;
		calibrate->error_best[calibrate->nsaveds - 1] = value;
		calibrate->simulation_best[calibrate->nsaveds - 1] = simulation;
		for (i = calibrate->nsaveds; --i;)
		{
			if (calibrate->error_best[i] < calibrate->error_best[i - 1])
			{
				j = calibrate->simulation_best[i];
				e = calibrate->error_best[i];
				calibrate->simulation_best[i]
					= calibrate->simulation_best[i - 1];
				calibrate->error_best[i] = calibrate->error_best[i - 1];
				calibrate->simulation_best[i - 1] = j;
				calibrate->error_best[i - 1] = e;
			}
			else break;
		}
		g_mutex_unlock(&mutex);
	}
#if DEBUG
printf("calibrate_best_thread: end\n");
#endif
}

/**
 * \fn void calibrate_best_sequential(Calibrate *calibrate, \
 *   unsigned int simulation, double value)
 * \brief Function to save the bests simulations.
 * \param calibrate
 * \brief Calibration data.
 * \param simulation
 * \brief Simulation number.
 * \param value
 * \brief Objective function value.
 */
void calibrate_best_sequential(Calibrate *calibrate, unsigned int simulation,
	double value)
{
	unsigned int i, j;
	double e;
#if DEBUG
printf("calibrate_best_sequential: start\n");
#endif
	if (calibrate->nsaveds < calibrate->nbests
		|| value < calibrate->error_best[calibrate->nsaveds - 1])
	{
		if (calibrate->nsaveds < calibrate->nbests) ++calibrate->nsaveds;
		calibrate->error_best[calibrate->nsaveds - 1] = value;
		calibrate->simulation_best[calibrate->nsaveds - 1] = simulation;
		for (i = calibrate->nsaveds; --i;)
		{
			if (calibrate->error_best[i] < calibrate->error_best[i - 1])
			{
				j = calibrate->simulation_best[i];
				e = calibrate->error_best[i];
				calibrate->simulation_best[i]
					= calibrate->simulation_best[i - 1];
				calibrate->error_best[i] = calibrate->error_best[i - 1];
				calibrate->simulation_best[i - 1] = j;
				calibrate->error_best[i - 1] = e;
			}
			else break;
		}
	}
#if DEBUG
printf("calibrate_best_sequential: end\n");
#endif
}

/**
 * \fn void* calibrate_thread(ParallelData *data)
 * \brief Function to calibrate on a thread.
 * \param data
 * \brief Function data.
 * \return NULL
 */
void* calibrate_thread(ParallelData *data)
{
	unsigned int i, j, thread;
	double e;
	Calibrate *calibrate;
#if DEBUG
printf("calibrate_thread: start\n");
#endif
	thread = data->thread;
	calibrate = data->calibrate;
#if DEBUG
printf("calibrate_thread: thread=%u start=%u end=%u\n", thread,
calibrate->thread[thread], calibrate->thread[thread + 1]);
#endif
	for (i = calibrate->thread[thread]; i < calibrate->thread[thread + 1]; ++i)
	{
		e = 0.;
		for (j = 0; j < calibrate->nexperiments; ++j)
			e += calibrate_parse(calibrate, i, j);
		calibrate_best_thread(calibrate, i, e);
#if DEBUG
printf("calibrate_thread: i=%u e=%lg\n", i, e);
#endif
	}
#if DEBUG
printf("calibrate_thread: end\n");
#endif
	g_thread_exit(NULL);
	return NULL;
}

/**
 * \fn void calibrate_sequential(Calibrate *calibrate)
 * \brief Function to calibrate sequentially.
 * \param calibrate
 * \brief Calibration data pointer.
 */
void calibrate_sequential(Calibrate *calibrate)
{
	unsigned int i, j;
	double e;
#if DEBUG
printf("calibrate_sequential: start\n");
#endif
	for (i = calibrate->nstart; i < calibrate->nend; ++i)
	{
		e = 0.;
		for (j = 0; j < calibrate->nexperiments; ++j)
			e += calibrate_parse(calibrate, i, j);
		calibrate_best_sequential(calibrate, i, e);
#if DEBUG
printf("calibrate_sequential: i=%u e=%lg\n", i, e);
#endif
	}
#if DEBUG
printf("calibrate_sequential: end\n");
#endif
}

/**
 * \fn void calibrate_merge(Calibrate *calibrate, unsigned int nsaveds, \
 *   unsigned int *simulation_best, double *error_best)
 * \brief Function to merge the 2 calibration results.
 * \param calibrate
 * \brief Calibration data.
 * \param nsaveds
 * \brief Number of saved results.
 * \param simulation_best
 * \brief Array of bests simulation numbers.
 * \param error_best
 * \brief Array of bests objective function values.
 */
void calibrate_merge(Calibrate *calibrate, unsigned int nsaveds,
	unsigned int *simulation_best, double *error_best)
{
	unsigned int i, j, k, s[calibrate->nbests];
	double e[calibrate->nbests];
#if DEBUG
printf("calibrate_merge: start\n");
#endif
	i = j = k = 0;
	do
	{
		if (i == calibrate->nsaveds)
		{
			s[k] = simulation_best[j];
			e[k] = error_best[j];
			++j;
			++k;
			if (j == nsaveds) break;
		}
		else if (j == nsaveds)
		{
			s[k] = calibrate->simulation_best[i];
			e[k] = calibrate->error_best[i];
			++i;
			++k;
			if (i == calibrate->nsaveds) break;
		}
		else if (calibrate->error_best[i] > error_best[j])
		{
			s[k] = simulation_best[j];
			e[k] = error_best[j];
			++j;
			++k;
		}
		else
		{
			s[k] = calibrate->simulation_best[i];
			e[k] = calibrate->error_best[i];
			++i;
			++k;
		}
	}
	while (k < calibrate->nbests);
	calibrate->nsaveds = k;
	memcpy(calibrate->simulation_best, s, k * sizeof(unsigned int));
	memcpy(calibrate->error_best, e, k * sizeof(double));
#if DEBUG
printf("calibrate_merge: end\n");
#endif
}

/**
 * \fn void calibrate_synchronise(Calibrate *calibrate)
 * \brief Function to synchronise the calibration results of MPI tasks.
 * \param calibrate
 * \brief Calibration data.
 */
#ifdef HAVE_MPI
void calibrate_synchronise(Calibrate *calibrate)
{
	unsigned int i, nsaveds, simulation_best[calibrate->nbests];
	double error_best[calibrate->nbests];
	MPI_Status mpi_stat;
#if DEBUG
printf("calibrate_synchronise: start\n");
#endif
	if (calibrate->mpi_rank == 0)
	{
		for (i = 1; i < calibrate->mpi_tasks; ++i)
		{
			MPI_Recv(&nsaveds, 1, MPI_INT, i, 1, MPI_COMM_WORLD, &mpi_stat);
			MPI_Recv(simulation_best, nsaveds, MPI_INT, i, 1,
				MPI_COMM_WORLD, &mpi_stat);
			MPI_Recv(error_best, nsaveds, MPI_DOUBLE, i, 1,
				MPI_COMM_WORLD, &mpi_stat);
			calibrate_merge(calibrate, nsaveds, simulation_best, error_best);
		}
	}
	else
	{
		MPI_Send(&calibrate->nsaveds, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
		MPI_Send(calibrate->simulation_best, calibrate->nsaveds, MPI_INT, 0, 1,
			MPI_COMM_WORLD);
		MPI_Send(calibrate->error_best, calibrate->nsaveds, MPI_DOUBLE, 0, 1,
			MPI_COMM_WORLD);
	}
#if DEBUG
printf("calibrate_synchronise: end\n");
#endif
}
#endif

#ifdef HAVE_GAUL

/**
 * \fn double ga_calibrate_parse(Calibrate *calibrate, unsigned int simulation, \
 *   unsigned int experiment)
 * \brief Function to parse input files, simulating and calculating the \
 *   objective function.
 * \param calibrate
 * \brief Calibration data.
 * \param simulation
 * \brief Simulation number.
 * \param experiment
 * \brief Experiment number.
 * \return Objective function value.
 */
double ga_calibrate_parse(Calibrate *calibrate, unsigned int rank, unsigned int thread,
	unsigned int experiment)
{
	unsigned int i;
	double e;
	char buffer[512], input[4][32], output[32], result[32];
	FILE *file_result;

#if DEBUG
printf("calibrate_parse: start\n");
printf("calibrate_parse: simulation=%u-%u experiment=%u\n", rank, thread, experiment);
#endif

	// Opening input files
	for (i = 0; i < calibrate->ninputs; ++i)
	{
		snprintf(&input[i][0], 32, "input-%u-%u-%u-%u", i, rank, thread, experiment);
#if DEBUG
printf("calibrate_parse: i=%u input=%s\n", i, &input[i][0]);
#endif
		calibrate_input(calibrate, thread, &input[i][0],
			calibrate->file[i][experiment]);
	}
	for (; i < 4; ++i) snprintf(&input[i][0], 32, "");
#if DEBUG
printf("calibrate_parse: parsing end\n");
#endif

	// Performing the simulation
	snprintf(output, 32, "output-%u-%u-%u", rank, thread, experiment);
	snprintf(result, 32, "result-%u-%u-%u", rank, thread, experiment);
	snprintf(buffer, 512, "./%s %s %s %s %s %s", calibrate->simulator,
		&input[0][0], &input[1][0], &input[2][0], &input[3][0], output);
#if DEBUG
printf("calibrate_parse: %s\n", buffer);
#endif
	system(buffer);

	// Checking the objective value function
	snprintf(buffer, 512, "./%s %s %s %s", calibrate->evaluator, output,
		calibrate->experiment[experiment], result);
#if DEBUG
printf("calibrate_parse: %s\n", buffer);
#endif
	system(buffer);
	file_result = fopen(result, "r");
	e = atof(fgets(buffer, 512, file_result));
	fclose(file_result);

	// Removing files
#if !DEBUG
	snprintf(buffer, 512, "rm %s %s %s %s %s %s", &input[0][0], &input[1][0],
		&input[2][0], &input[3][0], output, result);
	system(buffer);
#endif

#if DEBUG
printf("calibrate_parse: end\n");
#endif

	// Returning the objective function
	return e;
}

boolean genetic_score(population *pop, entity *entity)
{
	int j, rank, thread;
	double score = 0.0;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	thread = omp_get_thread_num();

	for (j = 0; j < ga_calibrate->nvariables; ++j)
	{
		ga_calibrate->value[thread * ga_calibrate->nvariables + j] = 
			ga_get_variable(entity->chromosome[0], j);
	}

	score = 0.0;
	for (j = 0; j < ga_calibrate->nexperiments; ++j)
	{
		score += ga_calibrate_parse(ga_calibrate, rank, thread, j);
	}

	entity->fitness = -score;

	return TRUE;
}

#endif

/**
 * \fn void calibrate_genetic(Calibrate *calibrate)
 * \brief Function to calibrate with the genetic algorithm.
 * \param calibrate
 * \brief Calibration data pointer.
 */

void calibrate_genetic(Calibrate *calibrate)
{
#ifdef HAVE_GAUL
	int i;

	// Store calibrate
	ga_calibrate = calibrate;

	// Population of solutions
	population *pop = NULL;

	// Number of variables and bits for each one
	ga_set_variables( calibrate->nvariables, calibrate->bits );

	// Lowest and highest values
	for( i=0 ; i<calibrate->nvariables ; i++ )
	{
		ga_set_variable_min_max( i, calibrate->rangemin[i], calibrate->rangemax[i] );
	}

	// Random seed number
	random_seed(DEFAULT_RANDOM_SEED);

	if (calibrate->mpi_rank != 0)
	{
		/*
		 * A population is created so that the callbacks are defined.  Evolution doesn't
		 * occur with this population, so population_size can be zero.
		 */
		// Genesis configuration
		pop = ga_genesis_bitstring(
			0,				/* const int              population_size */
			1,				/* const int              num_chromo */
			ga_bits,		/* const int              len_chromo */
			NULL,			/* GAgeneration_hook      generation_hook */
			NULL,			/* GAiteration_hook       iteration_hook */
			NULL,			/* GAdata_destructor      data_destructor */
			NULL,			/* GAdata_ref_incrementor data_ref_incrementor */
			genetic_score,	/* GAevaluate             evaluate */
			ga_seed_bitstring_random,	/* GAseed                 seed */
			NULL,			/* GAadapt                adapt */
			ga_select_one_bestof2,	/* GAselect_one           select_one */
			ga_select_two_bestof2,	/* GAselect_two           select_two */
			ga_mutate_bitstring_singlepoint,	/* GAmutate               mutate */
			ga_crossover_bitstring_doublepoints,	/* GAcrossover            crossover */
			NULL,			/* GAreplace              replace */
			NULL			/* vpointer	User data */
		);

		// The slaves halt here until ga_detach_mpi_slaves(), below, is called
		ga_attach_mpi_slave( pop );
	}
	else
	{
		// Genesis configuration
		pop = ga_genesis_bitstring(
			calibrate->population,		/* const int              population_size */
			1,				/* const int              num_chromo */
			ga_bits,		/* const int              len_chromo */
			NULL,			/* GAgeneration_hook      generation_hook */
			NULL,			/* GAiteration_hook       iteration_hook */
			NULL,			/* GAdata_destructor      data_destructor */
			NULL,			/* GAdata_ref_incrementor data_ref_incrementor */
			genetic_score,	/* GAevaluate             evaluate */
			ga_seed_bitstring_random,	/* GAseed                 seed */
			NULL,			/* GAadapt                adapt */
			ga_select_one_bestof2,	/* GAselect_one           select_one */
			ga_select_two_bestof2,	/* GAselect_two           select_two */
			ga_mutate_bitstring_singlepoint,	/* GAmutate               mutate */
			ga_crossover_bitstring_doublepoints,	/* GAcrossover            crossover */
			NULL,			/* GAreplace              replace */
			NULL			/* vpointer	User data */
		);

		// Population configuration
		ga_population_set_parameters(
			pop,			/* population      *pop */
			GA_SCHEME_DARWIN,	/* const ga_scheme_type     scheme */
			GA_ELITISM_PARENTS_DIE,	/* const ga_elitism_type   elitism */
			calibrate->crossover,		/* double  crossover */
			calibrate->mutation,		/* double  mutation */
			0.0				/* double  migration */
		);

		if( calibrate->mpi_tasks > 1 )
		{
			// Run evolution
			ga_evolution_mpi(
				pop,					/* population              *pop */
				calibrate->generations	/* const int               max_generations */
			);
		}
		else
		{
			// Run evolution
			ga_evolution(
				pop,					/* population              *pop */
				calibrate->generations	/* const int               max_generations */
			);
		}

		printf("THE BEST IS\n");
		printf("error=%e\n", -ga_get_entity_from_rank(pop,0)->fitness);
		for( i=0 ; i<calibrate->nvariables ; i++ )
		{
			printf("var%d=%e\n", i+1,
				ga_get_variable( ga_get_entity_from_rank(pop,0)->chromosome[0], i ));
		}

		ga_extinction(pop);

		if( calibrate->mpi_tasks > 1 )
		{
			// Allow all slave processes to continue
			ga_detach_mpi_slaves();
		}
	}
#endif
}

/**
 * \fn void calibrate_sweep(Calibrate *calibrate)
 * \brief Function to calibrate with the sweep algorithm.
 * \param calibrate
 * \brief Calibration data pointer.
 */
void calibrate_sweep(Calibrate *calibrate)
{
	unsigned int i, j, k, l;
	double e;
	GThread *thread[calibrate->nthreads];
	ParallelData data[calibrate->nthreads];
#if DEBUG
printf("calibrate_sweep: start\n");
#endif
	for (i = 0; i < calibrate->nsimulations; ++i)
	{
		k = i;
		for (j = 0; j < calibrate->nvariables; ++j)
		{
			l = k % calibrate->nsweeps[j];
			k /= calibrate->nsweeps[j];
			e = calibrate->rangemin[j];
			if (calibrate->nsweeps[j] > 1)
				e += l * (calibrate->rangemax[j] - calibrate->rangemin[j])
					/ (calibrate->nsweeps[j] - 1);
			calibrate->value[i * calibrate->nvariables + j] = e;
		}
	}
	if (calibrate->nthreads <= 1)
		calibrate_sequential(calibrate);
	else
	{
		for (i = 0; i < calibrate->nthreads; ++i)
		{
			data[i].calibrate = calibrate;
			data[i].thread = i;
			thread[i] = g_thread_new(NULL, (void(*))calibrate_thread, &data[i]);
		}
		for (i = 0; i < calibrate->nthreads; ++i) g_thread_join(thread[i]);
	}
#ifdef HAVE_MPI
	// Communicating tasks results
	calibrate_synchronise(calibrate);
#endif
#if DEBUG
printf("calibrate_sweep: end\n");
#endif
}

/**
 * \fn void calibrate_MonteCarlo(Calibrate *calibrate)
 * \brief Function to calibrate with the Monte-Carlo algorithm.
 * \param calibrate
 * \brief Calibration data pointer.
 */
void calibrate_MonteCarlo(Calibrate *calibrate)
{
	unsigned int i, j;
	GThread *thread[calibrate->nthreads];
	ParallelData data[calibrate->nthreads];
#if DEBUG
printf("calibrate_MonteCarlo: start\n");
#endif
	for (i = 0; i < calibrate->nsimulations; ++i)
		for (j = 0; j < calibrate->nvariables; ++j)
			calibrate->value[i * calibrate->nvariables + j] =
				calibrate->rangemin[j] + gsl_rng_uniform(rng)
				* (calibrate->rangemax[j] - calibrate->rangemin[j]);
	if (calibrate->nthreads <= 1)
		calibrate_sequential(calibrate);
	else
	{
		for (i = 0; i < calibrate->nthreads; ++i)
		{
			data[i].calibrate = calibrate;
			data[i].thread = i;
			thread[i] = g_thread_new(NULL, (void(*))calibrate_thread, &data[i]);
		}
		for (i = 0; i < calibrate->nthreads; ++i) g_thread_join(thread[i]);
	}
#ifdef HAVE_MPI
	// Communicating tasks results
	calibrate_synchronise(calibrate);
#endif
#if DEBUG
printf("calibrate_MonteCarlo: end\n");
#endif
}

/**
 * \fn void calibrate_refine(Calibrate *calibrate)
 * \brief Function to refine the search ranges of the variables in iterative
 *   algorithms.
 * \param calibrate
 * \brief Calibration data.
 */
void calibrate_refine(Calibrate *calibrate)
{
	unsigned int i, j;
	double d;
#ifdef HAVE_MPI
	MPI_Status mpi_stat;
#endif
#if DEBUG
printf("calibrate_refine: start\n");
#endif
#ifdef HAVE_MPI
	if (!calibrate->mpi_rank)
	{
#endif
		for (j = 0; j < calibrate->nvariables; ++j)
			calibrate->rangemin[j] = calibrate->rangemax[j] = calibrate->value
				[calibrate->simulation_best[0] * calibrate->nvariables + j];
		for (i = 0; ++i < calibrate->nsaveds;)
		{
			for (j = 0; j < calibrate->nvariables; ++j)
			{
				calibrate->rangemin[j] = fmin(calibrate->rangemin[j],
					calibrate->value
						[calibrate->simulation_best[i] * calibrate->nvariables
							+ j]);
				calibrate->rangemax[j] = fmax(calibrate->rangemax[j],
					calibrate->value
						[calibrate->simulation_best[i] * calibrate->nvariables
							+ j]);
			}
		}
		for (j = 0; j < calibrate->nvariables; ++j)
		{
			d = calibrate->tolerance
				* (calibrate->rangemax[j] - calibrate->rangemin[j]);
			calibrate->rangemin[j] -= d;
			calibrate->rangemax[j] += d;
			printf("%s min=%lg max=%lg\n", calibrate->label[j],
				calibrate->rangemin[j], calibrate->rangemax[j]);
			fprintf(calibrate->result, "%s min=%lg max=%lg\n",
				calibrate->label[j], calibrate->rangemin[j],
				calibrate->rangemax[j]);
		}
#ifdef HAVE_MPI
		for (i = 1; i < calibrate->mpi_tasks; ++i)
		{
			MPI_Send(calibrate->rangemin, calibrate->nvariables, MPI_DOUBLE, i,
				1, MPI_COMM_WORLD);
			MPI_Send(calibrate->rangemax, calibrate->nvariables, MPI_DOUBLE, i,
				1, MPI_COMM_WORLD);
		}
	}
	else
	{
		MPI_Recv(calibrate->rangemin, calibrate->nvariables, MPI_DOUBLE, 0, 1,
			MPI_COMM_WORLD, &mpi_stat);
		MPI_Recv(calibrate->rangemax, calibrate->nvariables, MPI_DOUBLE, 0, 1,
			MPI_COMM_WORLD, &mpi_stat);
	}
#endif
#if DEBUG
printf("calibrate_refine: end\n");
#endif
}

/**
 * \fn void calibrate_print(Calibrate *calibrate)
 * \brief Function to print the results.
 * \param calibrate
 * \brief Calibration data.
 */
void calibrate_print(Calibrate *calibrate)
{
	unsigned int i;
	char buffer[512];
#ifdef HAVE_MPI
	if (!calibrate->mpi_rank)
	{
#endif
		printf("THE BEST IS\n");
		fprintf(calibrate->result, "THE BEST IS\n");
		printf("error=%le\n", calibrate->error_best[0]);
		fprintf(calibrate->result, "error=%le\n", calibrate->error_best[0]);
		for (i = 0; i < calibrate->nvariables; ++i)
		{
			snprintf(buffer, 512, "%s=%s\n",
				calibrate->label[i], calibrate->format[i]);
			printf(buffer, calibrate->value[calibrate->simulation_best[0]
				* calibrate->nvariables + i]);
			fprintf(calibrate->result, buffer,
				calibrate->value[calibrate->simulation_best[0]
					* calibrate->nvariables + i]);
		}
		fflush(calibrate->result);
#ifdef HAVE_MPI
	}
#endif
}

/**
 * \fn void calibrate_iterate(Calibrate *calibrate)
 * \brief Function to iterate the algorithm.
 * \param calibrate
 * \brief Calibration data.
 */
void calibrate_iterate(Calibrate *calibrate)
{
	unsigned int i;
#if DEBUG
printf("calibrate_iterate: start\n");
#endif
	for (i = 0; i < calibrate->niterations; ++i)
	{
		calibrate_step(calibrate);
		calibrate_refine(calibrate);
		calibrate_print(calibrate);
	}
#if DEBUG
printf("calibrate_iterate: end\n");
#endif
}

/**
 * \fn int calibrate_new(Calibrate *calibrate, char *filename)
 * \brief Function to open and perform a calibration.
 * \param calibrate
 * \brief Calibration data pointer.
 * \param filename
 * \brief Input data file name.
 * \return 1 on success, 0 on error.
 */
int calibrate_new(Calibrate *calibrate, char *filename)
{
	unsigned int i, j;
	xmlChar *buffer;
	xmlNode *node, *child;
	xmlDoc *doc;
	static const xmlChar *template[4]=
		{XML_TEMPLATE1, XML_TEMPLATE2, XML_TEMPLATE3, XML_TEMPLATE4};

#if DEBUG
printf("calibrate_new: start\n");
#endif

	// Parsing the XML data file
	doc = xmlParseFile(filename);
	if (!doc)
	{
		printf("Unable to parse the data file %s\n", filename);
		return 0;
	}

	// Obtaining the root XML node
	node = xmlDocGetRootElement(doc);
	if (!node)
	{
		printf("No XML nodes in the data file\n");
		return 0;
	}
	if (xmlStrcmp(node->name, XML_CALIBRATE))
	{
		printf("Bad name of the XML root node in the data file\n");
		return 0;
	}

	// Obtaining the simulator file
	if (xmlHasProp(node, XML_SIMULATOR))
	{
		calibrate->simulator = (char*)xmlGetProp(node, XML_SIMULATOR);
	}
	else
	{
		printf("No simulator in the data file\n");
		return 0;
	}

	// Obtaining the evaluator file
	if (xmlHasProp(node, XML_EVALUATOR))
	{
		calibrate->evaluator = (char*)xmlGetProp(node, XML_EVALUATOR);
	}
	else
	{
		printf("No error in the data file\n");
		return 0;
	}

	// Reading the algorithm
	if (xmlHasProp(node, XML_ALGORITHM))
	{
		buffer = xmlGetProp(node, XML_ALGORITHM);
		if (!xmlStrcmp(buffer, XML_SWEEP))
		{
			calibrate->algorithm = CALIBRATE_ALGORITHM_SWEEP;
			calibrate_step = calibrate_sweep;
			xmlFree(buffer);
		}
		else if (!xmlStrcmp(buffer, XML_GENETIC))
		{
			calibrate->algorithm = CALIBRATE_ALGORITHM_GENETIC;
			calibrate_step = calibrate_genetic;
			xmlFree(buffer);

			// Check GAUL
			#ifndef HAVE_GAUL
				printf("Calibrator was not compiled with GAUL support\n");
				return 0;
			#endif

			// Obtaining population
			if (xmlHasProp(node, XML_POPULATION))
			{
				buffer = xmlGetProp(node, XML_POPULATION);
				calibrate->population = strtoul((char*)buffer, NULL, 0);
				xmlFree(buffer);
				if( ! ( calibrate->population > 0 ) )
				{
					printf("Invalid population number\n");
					return 0;
				}
			}
			else
			{
				printf("No population number in the data file\n");
				return 0;
			}

			// Obtaining generations
			if (xmlHasProp(node, XML_GENERATIONS))
			{
				buffer = xmlGetProp(node, XML_GENERATIONS);
				calibrate->generations = strtoul((char*)buffer, NULL, 0);
				calibrate->nsimulations = calibrate->population * calibrate->generations;
				xmlFree(buffer);
				if( ! ( calibrate->generations > 0 ) )
				{
					printf("Invalid generation number\n");
					return 0;
				}
			}
			else
			{
				printf("No generation number in the data file\n");
				return 0;
			}

			// Obtaining bits representing each variable
			if (xmlHasProp(node, XML_BITS))
			{
				buffer = xmlGetProp(node, XML_BITS);
				calibrate->bits = strtoul((char*)buffer, NULL, 0);
				xmlFree(buffer);
				if( ! ( calibrate->bits > 0 ) )
				{
					printf("Invalid bit number\n");
					return 0;
				}
			}
			else
			{
				printf("No generation number in the data file\n");
				return 0;
			}

			// Obtaining crossover probability
			if (xmlHasProp(node, XML_CROSSOVER))
			{
				buffer = xmlGetProp(node, XML_CROSSOVER);
				calibrate->crossover = atof((char*)buffer);
				xmlFree(buffer);
				if( ! ( calibrate->crossover >= 0.0 && calibrate->crossover <= 1.0 ) )
				{
					printf("Invalid crossover probability\n");
					return 0;
				}
			}
			else
			{
				printf("No crossover probability in the data file\n");
				return 0;
			}

			// Obtaining mutation probability
			if (xmlHasProp(node, XML_MUTATION))
			{
				buffer = xmlGetProp(node, XML_MUTATION);
				calibrate->mutation = atof((char*)buffer);
				xmlFree(buffer);
				if( ! ( calibrate->mutation >= 0.0 && calibrate->mutation <= 1.0 ) )
				{
					printf("Invalid mutation probability\n");
					return 0;
				}
			}
			else
			{
				printf("No mutation probability in the data file\n");
				return 0;
			}

		}
		else if (!xmlStrcmp(buffer, XML_MONTE_CARLO))
		{
			calibrate->algorithm = CALIBRATE_ALGORITHM_MONTE_CARLO;
			calibrate_step = calibrate_MonteCarlo;

			// Obtaining the simulations number
			if (xmlHasProp(node, XML_SIMULATIONS))
			{
				buffer = xmlGetProp(node, XML_SIMULATIONS);
				calibrate->nsimulations = strtoul((char*)buffer, NULL, 0);
				xmlFree(buffer);
			}
			else
			{
				printf("No simulations number in the data file\n");
				return 0;
			}
		}
		else
		{
			printf("Unknown algorithm %s\n", buffer);
			return 0;
		}
	}
	else
	{
		calibrate->algorithm = CALIBRATE_ALGORITHM_MONTE_CARLO;
		calibrate_step = calibrate_MonteCarlo;

		// Obtaining the simulations number
		if (xmlHasProp(node, XML_SIMULATIONS))
		{
			buffer = xmlGetProp(node, XML_SIMULATIONS);
			calibrate->nsimulations = strtoul((char*)buffer, NULL, 0);
			xmlFree(buffer);
		}
		else
		{
			printf("No simulations number in the data file\n");
			return 0;
		}
	}

	// Reading the iterations number
	if (xmlHasProp(node, XML_ITERATIONS))
	{
		buffer = xmlGetProp(node, XML_ITERATIONS);
		calibrate->niterations = strtoul((char*)buffer, NULL, 0);
		xmlFree(buffer);
		if (!calibrate->niterations)
		{
			printf("Null iterations number in the data file\n");
			return 0;
		}
	}
	else calibrate->niterations = 1;

	// Reading the best simulations number
	if (xmlHasProp(node, XML_BESTS))
	{
		buffer = xmlGetProp(node, XML_BESTS);
		calibrate->nbests = strtoul((char*)buffer, NULL, 0);
		xmlFree(buffer);
		if (!calibrate->nbests)
		{
			printf("Null bests number in the data file\n");
			return 0;
		}
	}
	else calibrate->nbests = 1;
	calibrate->simulation_best
		= (unsigned int*)alloca(calibrate->nbests * sizeof(unsigned int));
	calibrate->error_best = (double*)alloca(calibrate->nbests * sizeof(double));
	calibrate->nsaveds = 0;

	// Reading the algorithm tolerance
	if (xmlHasProp(node, XML_TOLERANCE))
	{
		buffer = xmlGetProp(node, XML_TOLERANCE);
		calibrate->tolerance = atof((char*)buffer);
		xmlFree(buffer);
		if (calibrate->tolerance < 0.)
		{
			printf("Negative tolerance\n");
			return 0;
		}
	}
	else calibrate->tolerance = 0.;
	// Reading the experimental data file names
	calibrate->nexperiments = 0;
	calibrate->experiment = NULL;
	for (i = 0; i < 4; ++i)
	{
		calibrate->template[i] = NULL;
		calibrate->file[i] = NULL;
	}
	for (child = node->children; child; child = child->next)
	{
		if (xmlStrcmp(child->name, XML_EXPERIMENT)) break;
#if DEBUG
printf("calibrate_new: nexperiments=%u\n", calibrate->nexperiments);
#endif
		if (xmlHasProp(child, XML_NAME))
		{
			calibrate->experiment = realloc(calibrate->experiment,
				(1 + calibrate->nexperiments) * sizeof(char*));
			calibrate->experiment[calibrate->nexperiments] =
				(char*)xmlGetProp(child, XML_NAME);
		}
		else
		{
			printf("No experiment %u file name\n", calibrate->nexperiments + 1);
			return 0;
		}
		if (!calibrate->nexperiments) calibrate->ninputs = 0;
#if DEBUG
printf("calibrate_new: template[0]\n");
#endif
		if (xmlHasProp(child, XML_TEMPLATE1))
		{
			calibrate->template[0] = realloc(calibrate->template[0],
				(1 + calibrate->nexperiments) * sizeof(char*));
			calibrate->template[0][calibrate->nexperiments] =
				(char*)xmlGetProp(child, template[0]);
			calibrate->file[0] = realloc(calibrate->file[0],
				(1 + calibrate->nexperiments) * sizeof(GMappedFile*));
#if DEBUG
printf("calibrate_new: experiment=%u template1=%s\n", calibrate->nexperiments,
calibrate->template[0][calibrate->nexperiments]);
#endif
			calibrate->file[0][calibrate->nexperiments] =
				g_mapped_file_new
					(calibrate->template[0][calibrate->nexperiments], 0, NULL);
			if (!calibrate->nexperiments) ++calibrate->ninputs;
#if DEBUG
printf("calibrate_new: ninputs=%u\n", calibrate->ninputs);
#endif
		}
		else
		{
			printf("No experiment %u template1\n", calibrate->nexperiments + 1);
			return 0;
		}
		for (j = 1; j < 4; ++j)
		{
#if DEBUG
printf("calibrate_new: template%u\n", j + 1);
#endif
			if (xmlHasProp(child, template[j]))
			{
				if (calibrate->nexperiments && calibrate->ninputs < 2)
				{
					printf("Experiment %u: bad templates number\n",
						calibrate->nexperiments + 1);
					return 0;
				}
				calibrate->template[j] = realloc(calibrate->template[j],
					(1 + calibrate->nexperiments) * sizeof(char*));
				calibrate->template[j][calibrate->nexperiments] =
					(char*)xmlGetProp(child, template[j]);
				calibrate->file[j] = realloc(calibrate->file[j],
					(1 + calibrate->nexperiments) * sizeof(GMappedFile*));
#if DEBUG
printf("calibrate_new: experiment=%u template%u=%s\n", calibrate->nexperiments,
j + 1, calibrate->template[j][calibrate->nexperiments]);
#endif
				calibrate->file[j][calibrate->nexperiments] =
					g_mapped_file_new
						(calibrate->template[j][calibrate->nexperiments], 0,
						 	NULL);
				if (!calibrate->nexperiments) ++calibrate->ninputs;
#if DEBUG
printf("calibrate_new: ninputs=%u\n", calibrate->ninputs);
#endif
			}
			else if (calibrate->nexperiments && calibrate->ninputs > 1)
			{
				printf("No experiment %u template%u\n",
					calibrate->nexperiments + 1, j + 1);
				return 0;
			}
			else break;
		}
		++calibrate->nexperiments;
#if DEBUG
printf("calibrate_new: nexperiments=%u\n", calibrate->nexperiments);
#endif
	}
	if (!calibrate->nexperiments)
	{
		printf("No calibration experiments\n");
		return 0;
	}

	// Reading the variables data
	calibrate->nvariables = 0;
	calibrate->label = NULL;
	calibrate->rangemin = NULL;
	calibrate->rangemax = NULL;
	calibrate->format = NULL;
	calibrate->nsweeps = NULL;
	if (calibrate->algorithm == CALIBRATE_ALGORITHM_SWEEP)
		calibrate->nsimulations = 1;
	for (; child; child = child->next)
	{
		if (xmlStrcmp(child->name, XML_VARIABLE))
		{
			printf("Bad XML node\n");
			return 0;
		}
		if (xmlHasProp(child, XML_NAME))
		{
			calibrate->label = realloc(calibrate->label,
				(1 + calibrate->nvariables) * sizeof(char*));
			calibrate->label[calibrate->nvariables] =
				(char*)xmlGetProp(child, XML_NAME);
		}
		else
		{
			printf("No variable %u name\n", calibrate->nvariables + 1);
			return 0;
		}
		if (xmlHasProp(child, XML_MINIMUM))
		{
			calibrate->rangemin = realloc(calibrate->rangemin,
				(1 + calibrate->nvariables) * sizeof(double));
			buffer = xmlGetProp(child, XML_MINIMUM);
			calibrate->rangemin[calibrate->nvariables] = atof((char*)buffer);
			xmlFree(buffer);
		}
		else
		{
			printf("No variable %u minimum range\n", calibrate->nvariables + 1);
			return 0;
		}
		if (xmlHasProp(child, XML_MAXIMUM))
		{
			calibrate->rangemax = realloc(calibrate->rangemax,
				(1 + calibrate->nvariables) * sizeof(double));
			buffer = xmlGetProp(child, XML_MAXIMUM);
			calibrate->rangemax[calibrate->nvariables] = atof((char*)buffer);
			xmlFree(buffer);
		}
		else
		{
			printf("No variable %u maximum range\n", calibrate->nvariables + 1);
			return 0;
		}
		calibrate->format = realloc(calibrate->format,
			(1 + calibrate->nvariables) * sizeof(char*));
		if (xmlHasProp(child, XML_FORMAT))
		{
			calibrate->format[calibrate->nvariables] =
				(char*)xmlGetProp(child, XML_FORMAT);
		}
		else
		{
			calibrate->format[calibrate->nvariables] =
				(char*)xmlStrdup(DEFAULT_FORMAT);
		}
		if (calibrate->algorithm == CALIBRATE_ALGORITHM_SWEEP)
		{
			if (xmlHasProp(child, XML_SWEEPS))
			{
				calibrate->nsweeps = realloc(calibrate->nsweeps,
					(1 + calibrate->nvariables) * sizeof(unsigned int));
				buffer = xmlGetProp(child, XML_SWEEPS);
				calibrate->nsweeps[calibrate->nvariables] =
					strtoul((char*)buffer, NULL, 0);
				xmlFree(buffer);
			}
			else
			{
				printf("No variable %u sweeps number\n",
					calibrate->nvariables + 1);
				return 0;
			}
			calibrate->nsimulations *=
				calibrate->nsweeps[calibrate->nvariables];
#if DEBUG
printf("calibrate_new: nsweeps=%u nsimulations=%u\n",
calibrate->nsweeps[calibrate->nvariables], calibrate->nsimulations);
#endif
		}
		++calibrate->nvariables;
	}
	if (!calibrate->nvariables)
	{
		printf("No calibration variables\n");
		return 0;
	}
#if DEBUG
printf("calibrate_new: nvariables=%u\n", calibrate->nvariables);
#endif

	// Allocating values
	calibrate->value = (double*)malloc(calibrate->nsimulations *
		calibrate->nvariables * sizeof(double));

	// Calculating simulations to perform on each task
#ifdef HAVE_MPI
	calibrate->nstart = calibrate->mpi_rank * calibrate->nsimulations
		/ calibrate->mpi_tasks;
	calibrate->nend = (1 + calibrate->mpi_rank) * calibrate->nsimulations
		/ calibrate->mpi_tasks;
#else
	calibrate->nstart = 0;
	calibrate->nend = calibrate->nsimulations;
#endif
#if DEBUG
printf("calibrate_new: nstart=%u nend=%u\n", calibrate->nstart,
calibrate->nend);
#endif

	// Calculating simulations to perform on each thread
	calibrate->thread =
		(unsigned int*)alloca((1 + calibrate->nthreads) * sizeof(unsigned int));
	for (i = 0; i <= calibrate->nthreads; ++i)
	{
	   calibrate->thread[i] = calibrate->nstart
		   + i * (calibrate->nend - calibrate->nstart) / calibrate->nthreads;
#if DEBUG
printf("calibrate_new: i=%u thread=%u\n", i, calibrate->thread[i]);
#endif
	}

	// Opening result file
	calibrate->result = fopen("result", "w");

	// Performing the algorithm
	switch (calibrate->algorithm)
	{
		// Genetic algorithm
		case CALIBRATE_ALGORITHM_GENETIC:
			calibrate_genetic(calibrate);
			break;

		// Iterative algorithm
		default:
			calibrate_iterate(calibrate);
	}

	// Closing the XML document
	xmlFreeDoc(doc);

	// Closing result file
	fclose(calibrate->result);

	// Freeing memory
	xmlFree(calibrate->simulator);
	xmlFree(calibrate->evaluator);
	for (i = 0; i < calibrate->nexperiments; ++i)
	{
		xmlFree(calibrate->experiment[i]);
		for (j = 0; j < calibrate->ninputs; ++j)
		{
			xmlFree(calibrate->template[j][i]);
			g_mapped_file_unref(calibrate->file[j][i]);
		}
	}
	free(calibrate->experiment);
	for (i = 0; i < calibrate->ninputs; ++i)
	{
		free(calibrate->template[i]);
		free(calibrate->file[i]);
	}
	for (i = 0; i < calibrate->nvariables; ++i)
	{
		xmlFree(calibrate->label[i]);
		xmlFree(calibrate->format[i]);
	}
	free(calibrate->label);
	free(calibrate->rangemin);
	free(calibrate->rangemax);
	free(calibrate->format);
	free(calibrate->nsweeps);
	free(calibrate->value);

#if DEBUG
printf("calibrate_new: end\n");
#endif

	return 1;
}

/**
 * \fn int cores_number()
 * \brief Function to obtain the cores number.
 * \return Cores number.
 */
int cores_number()
{
#ifdef G_OS_WIN32
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
#else
    return (int)sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

/**
 * \fn int main(int argn, char **argc)
 * \brief Main function.
 * \param argn
 * \brief Arguments number.
 * \param argc
 * \brief Arguments pointer.
 * \return 0 on success, >0 on error.
 */
int main(int argn, char **argc)
{
	Calibrate calibrate[1];

#ifdef HAVE_MPI
	// Starting MPI
	MPI_Init(&argn, &argc);
	MPI_Comm_size(MPI_COMM_WORLD, &calibrate->mpi_tasks);
	MPI_Comm_rank(MPI_COMM_WORLD, &calibrate->mpi_rank);
	printf("rank=%d tasks=%d\n", calibrate->mpi_rank, calibrate->mpi_tasks);
#endif

	// Checking sintaxis
	if (!(argn == 2 || (argn == 4 && !strcmp(argc[1], "-nthreads"))))
	{
		printf("The sintaxis is:\ncalibrator [-nthreads x] data_file\n");
#ifdef HAVE_MPI
		// Closing MPI
		MPI_Finalize();
#endif
		return 1;
	}

	// Starting GThreads
	if (argn == 2) calibrate->nthreads = cores_number();
	else calibrate->nthreads = atoi(argc[2]);
	printf("nthreads=%u\n", calibrate->nthreads);

#ifdef HAVE_GAUL
	printf("ompthreads=%u\n", omp_thread_count());
#endif

	// Starting pseudo-random numbers generator
	rng = gsl_rng_alloc(gsl_rng_taus2);
	gsl_rng_set(rng, DEFAULT_RANDOM_SEED);

	// Allowing spaces in the XML data file
	xmlKeepBlanksDefault(0);

	// Making calibration
	calibrate_new(calibrate, argc[argn - 1]);

	// Freeing memory
	gsl_rng_free(rng);

#ifdef HAVE_MPI
	// Closing MPI
	MPI_Finalize();
#endif

	return 0;
}
