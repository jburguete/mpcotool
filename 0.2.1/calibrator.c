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
#elif (!__BSD_VISIBLE)
	#include <alloca.h>
#endif
#if HAVE_MPI
	#include <mpi.h>
#endif
#include "genetic/genetic.h"

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
 * \var mutation_ratio
 * \brief Mutation probability.
 * \var reproduction_ratio
 * \brief Reproduction probability.
 * \var adaptation_ratio
 * \brief Adaptation probability.
 * \var result
 * \brief Result file.
 * \var rng
 * \brief GSL random number generator.
 * \var file
 * \brief Matrix of input template files.
 * \var genetic_variable
 * \brief array of variables for the genetic algorithm.
 * \var mpi_rank
 * \brief Number of MPI task.
 */
	char *simulator, *evaluator, **experiment, **template[4], **label, **format;
	unsigned int nvariables, nexperiments, ninputs, nsimulations, algorithm,
		*nsweeps, nstart, nend, *thread, niterations, nbests, nsaveds,
		*simulation_best;
	double *value, *rangemin, *rangemax, *error_best, tolerance,
		mutation_ratio, reproduction_ratio, adaptation_ratio;
	FILE *result;
	gsl_rng *rng;
	GMappedFile **file[4];
	GeneticVariable *genetic_variable;
#if HAVE_MPI
	int mpi_rank;
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
 * \var ntasks
 * \brief Number of tasks.
 * \var nthreads
 * \brief Number of threads.
 * \var mutex
 * \brief Mutex struct.
 * \var void (*calibrate_step)(Calibrate*)
 * \brief Pointer to the function to perform a calibration algorithm step.
 * \var calibrate
 * \brief Calibration data.
 */
int ntasks;
unsigned int nthreads;
GMutex mutex;
void (*calibrate_step)(Calibrate*);
Calibrate calibrate[1];

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
printf("calibrate_input: value=%s\n", value);
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
#if HAVE_MPI
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
		for (i = 1; i < ntasks; ++i)
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
	GThread *thread[nthreads];
	ParallelData data[nthreads];
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
	if (nthreads <= 1)
		calibrate_sequential(calibrate);
	else
	{
		for (i = 0; i < nthreads; ++i)
		{
			data[i].calibrate = calibrate;
			data[i].thread = i;
			thread[i] = g_thread_new(NULL, (void(*))calibrate_thread, &data[i]);
		}
		for (i = 0; i < nthreads; ++i) g_thread_join(thread[i]);
	}
#if HAVE_MPI
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
	GThread *thread[nthreads];
	ParallelData data[nthreads];
#if DEBUG
printf("calibrate_MonteCarlo: start\n");
#endif
	for (i = 0; i < calibrate->nsimulations; ++i)
		for (j = 0; j < calibrate->nvariables; ++j)
			calibrate->value[i * calibrate->nvariables + j] =
				calibrate->rangemin[j] + gsl_rng_uniform(calibrate->rng)
				* (calibrate->rangemax[j] - calibrate->rangemin[j]);
	if (nthreads <= 1) calibrate_sequential(calibrate);
	else
	{
		for (i = 0; i < nthreads; ++i)
		{
			data[i].calibrate = calibrate;
			data[i].thread = i;
			thread[i] = g_thread_new(NULL, (void(*))calibrate_thread, &data[i]);
		}
		for (i = 0; i < nthreads; ++i) g_thread_join(thread[i]);
	}
#if HAVE_MPI
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
#if HAVE_MPI
	MPI_Status mpi_stat;
#endif
#if DEBUG
printf("calibrate_refine: start\n");
#endif
#if HAVE_MPI
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
#if HAVE_MPI
		for (i = 1; i < ntasks; ++i)
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

double calibrate_genetic_objective(Entity *entity)
{
	unsigned int j;
	double objective;
	for (j = 0; j < calibrate->nvariables; ++j)
		calibrate->value[entity->id * calibrate->nvariables + j]
			= genetic_get_variable(entity, calibrate->genetic_variable + j);
	for (j = 0, objective = 0.; j < calibrate->nexperiments; ++j)
		objective += calibrate_parse(calibrate, entity->id, j);
	return objective;
}

/**
 * \fn void calibrate_genetic(Calibrate *calibrate)
 * \brief Function to calibrate with the genetic algorithm.
 * \param calibrate
 * \brief Calibration data pointer.
 */
void calibrate_genetic(Calibrate *calibrate)
{
	unsigned int i;
	char buffer[512], *best_genome;
	double best_objective, *best_variable;
#if DEBUG
printf("calibrate_genetic: start\n");
printf("calibrate_genetic: ntasks=%u nthreads=%u\n", ntasks, nthreads);
printf("calibrate_genetic: nvariables=%u population=%u generations=%u\n",
calibrate->nvariables, calibrate->nsimulations, calibrate->niterations);
printf("calibrate_genetic: mutation=%lg reproduction=%lg adaptation=%lg\n",
calibrate->mutation_ratio, calibrate->reproduction_ratio,
calibrate->adaptation_ratio);
#endif
	genetic_algorithm_default(
		calibrate->nvariables,
		calibrate->genetic_variable,
		calibrate->nsimulations,
		calibrate->niterations,
		calibrate->mutation_ratio,
		calibrate->reproduction_ratio,
		calibrate->adaptation_ratio,
		&calibrate_genetic_objective,
		&best_genome,
		&best_variable,
		&best_objective);
#if DEBUG
printf("calibrate_genetic: the best\n");
#endif
	printf("THE BEST IS\n");
	fprintf(calibrate->result, "THE BEST IS\n");
	printf("error=%le\n", best_objective);
	fprintf(calibrate->result, "error=%le\n", best_objective);
	for (i = 0; i < calibrate->nvariables; ++i)
	{
		snprintf(buffer, 512, "%s=%s\n",
			calibrate->label[i], calibrate->format[i]);
		printf(buffer, best_variable[i]);
		fprintf(calibrate->result, buffer, best_variable[i]);
	}
	fflush(calibrate->result);
	g_free(best_genome);
	g_free(best_variable);
#if DEBUG
printf("calibrate_genetic: start\n");
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
#if HAVE_MPI
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
#if HAVE_MPI
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
	unsigned int i, j, *nbits;
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
			nbits = NULL;
			calibrate->algorithm = CALIBRATE_ALGORITHM_GENETIC;
			calibrate_step = calibrate_genetic;
			xmlFree(buffer);

			// Obtaining population
			if (xmlHasProp(node, XML_POPULATION))
			{
				buffer = xmlGetProp(node, XML_POPULATION);
				calibrate->nsimulations = strtoul((char*)buffer, NULL, 0);
				xmlFree(buffer);
				if (calibrate->nsimulations < 3)
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
				calibrate->niterations = strtoul((char*)buffer, NULL, 0);
				calibrate->nsimulations
					= calibrate->nsimulations * calibrate->niterations;
				xmlFree(buffer);
				if (!calibrate->niterations)
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

			// Obtaining mutation probability
			if (xmlHasProp(node, XML_MUTATION))
			{
				buffer = xmlGetProp(node, XML_MUTATION);
				calibrate->mutation_ratio = atof((char*)buffer);
				xmlFree(buffer);
				if (calibrate->mutation_ratio < 0.
					|| calibrate->mutation_ratio >= 1.)
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

			// Obtaining reproduction probability
			if (xmlHasProp(node, XML_REPRODUCTION))
			{
				buffer = xmlGetProp(node, XML_REPRODUCTION);
				calibrate->reproduction_ratio = atof((char*)buffer);
				xmlFree(buffer);
				if (calibrate->reproduction_ratio < 0.
					|| calibrate->reproduction_ratio >= 1.0)
				{
					printf("Invalid reproduction probability\n");
					return 0;
				}
			}
			else
			{
				printf("No reproduction probability in the data file\n");
				return 0;
			}

			// Obtaining adaptation probability
			if (xmlHasProp(node, XML_ADAPTATION))
			{
				buffer = xmlGetProp(node, XML_ADAPTATION);
				calibrate->adaptation_ratio = atof((char*)buffer);
				xmlFree(buffer);
				if (calibrate->adaptation_ratio < 0.
					|| calibrate->adaptation_ratio >= 1.)
				{
					printf("Invalid adaptation probability\n");
					return 0;
				}
			}
			else
			{
				printf("No adaptation probability in the data file\n");
				return 0;
			}

			// Checking survivals
			i = calibrate->mutation_ratio * calibrate->nsimulations;
			i += calibrate->reproduction_ratio * calibrate->nsimulations;
			i += calibrate->adaptation_ratio * calibrate->nsimulations;
			if (i > calibrate->nsimulations - 2)
			{
				printf("No enough survival entities to reproduce the "
					"population\n");
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
			calibrate->experiment = g_realloc(calibrate->experiment,
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
			calibrate->template[0] = g_realloc(calibrate->template[0],
				(1 + calibrate->nexperiments) * sizeof(char*));
			calibrate->template[0][calibrate->nexperiments] =
				(char*)xmlGetProp(child, template[0]);
			calibrate->file[0] = g_realloc(calibrate->file[0],
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
				calibrate->template[j] = g_realloc(calibrate->template[j],
					(1 + calibrate->nexperiments) * sizeof(char*));
				calibrate->template[j][calibrate->nexperiments] =
					(char*)xmlGetProp(child, template[j]);
				calibrate->file[j] = g_realloc(calibrate->file[j],
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
			calibrate->label = g_realloc(calibrate->label,
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
			calibrate->rangemin = g_realloc(calibrate->rangemin,
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
			calibrate->rangemax = g_realloc(calibrate->rangemax,
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
		calibrate->format = g_realloc(calibrate->format,
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
				calibrate->nsweeps = g_realloc(calibrate->nsweeps,
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
		if (calibrate->algorithm == CALIBRATE_ALGORITHM_GENETIC)
		{
			// Obtaining bits representing each variable
			if (xmlHasProp(child, XML_BITS))
			{
				nbits = g_realloc(nbits,
					(1 + calibrate->nvariables) * sizeof(unsigned int));
				buffer = xmlGetProp(child, XML_BITS);
				i = strtoul((char*)buffer, NULL, 0);
				xmlFree(buffer);
				if (!i)
				{
					printf("Invalid bit number\n");
					return 0;
				}
				nbits[calibrate->nvariables] = i;
			}
			else
			{
				printf("No variable %u bits number\n",
					calibrate->nvariables + 1);
				return 0;
			}
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
	calibrate->genetic_variable = NULL;
	if (calibrate->algorithm == CALIBRATE_ALGORITHM_GENETIC)
	{
		calibrate->genetic_variable = (GeneticVariable*)
			g_malloc(calibrate->nvariables * sizeof(GeneticVariable));
		for (i = 0; i < calibrate->nvariables; ++i)
		{
			calibrate->genetic_variable[i].maximum = calibrate->rangemax[i];
			calibrate->genetic_variable[i].minimum = calibrate->rangemin[i];
			calibrate->genetic_variable[i].nbits = nbits[i];
		}
		g_free(nbits);
	}
	calibrate->value = (double*)g_malloc(calibrate->nsimulations *
		calibrate->nvariables * sizeof(double));

	// Calculating simulations to perform on each task
#if HAVE_MPI
	calibrate->nstart = calibrate->mpi_rank * calibrate->nsimulations / ntasks;
	calibrate->nend = (1 + calibrate->mpi_rank) * calibrate->nsimulations
		/ ntasks;
#else
	calibrate->nstart = 0;
	calibrate->nend = calibrate->nsimulations;
#endif
#if DEBUG
printf("calibrate_new: nstart=%u nend=%u\n", calibrate->nstart,
calibrate->nend);
#endif

	// Calculating simulations to perform on each thread
	calibrate->thread
		= (unsigned int*)alloca((1 + nthreads) * sizeof(unsigned int));
	for (i = 0; i <= nthreads; ++i)
	{
	   calibrate->thread[i] = calibrate->nstart
		   + i * (calibrate->nend - calibrate->nstart) / nthreads;
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
	g_free(calibrate->experiment);
	for (i = 0; i < calibrate->ninputs; ++i)
	{
		g_free(calibrate->template[i]);
		g_free(calibrate->file[i]);
	}
	for (i = 0; i < calibrate->nvariables; ++i)
	{
		xmlFree(calibrate->label[i]);
		xmlFree(calibrate->format[i]);
	}
	g_free(calibrate->label);
	g_free(calibrate->rangemin);
	g_free(calibrate->rangemax);
	g_free(calibrate->format);
	g_free(calibrate->nsweeps);
	g_free(calibrate->value);
	g_free(calibrate->genetic_variable);

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

#if HAVE_MPI
	// Starting MPI
	MPI_Init(&argn, &argc);
	MPI_Comm_size(MPI_COMM_WORLD, &ntasks);
	MPI_Comm_rank(MPI_COMM_WORLD, &calibrate->mpi_rank);
	printf("rank=%d tasks=%d\n", calibrate->mpi_rank, ntasks);
#else
	ntasks = 1;
#endif

	// Checking sintaxis
	if (!(argn == 2 || (argn == 4 && !strcmp(argc[1], "-nthreads"))))
	{
		printf("The sintaxis is:\ncalibrator [-nthreads x] data_file\n");
#if HAVE_MPI
		// Closing MPI
		MPI_Finalize();
#endif
		return 1;
	}

	// Starting GThreads
	if (argn == 2) nthreads = cores_number();
	else nthreads = atoi(argc[2]);
	printf("nthreads=%u\n", nthreads);

	// Starting pseudo-random numbers generator
	calibrate->rng = gsl_rng_alloc(gsl_rng_taus2);
	gsl_rng_set(calibrate->rng, DEFAULT_RANDOM_SEED);

	// Allowing spaces in the XML data file
	xmlKeepBlanksDefault(0);

	// Making calibration
	calibrate_new(calibrate, argc[argn - 1]);

	// Freeing memory
	gsl_rng_free(calibrate->rng);

#if HAVE_MPI
	// Closing MPI
	MPI_Finalize();
#endif

	return 0;
}
