/*
Calibrator: a software to make calibrations of empirical parameters.

AUTHORS: Javier Burguete and Borja Latorre.

Copyright 2012-2015, AUTHORS.

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
 * \file calibrator.h
 * \brief Header file of the calibrator.
 * \authors Javier Burguete.
 * \copyright Copyright 2012-2015, all rights reserved.
 */
#ifndef CALIBRATOR__H
#define CALIBRATOR__H 1

/**
 * \enum Algorithm
 * \brief Enum to define the algorithms.
 * \var ALGORITHM_MONTE_CARLO
 * \brief Monte-Carlo algorithm.
 * \var ALGORITHM_SWEEP
 * \brief Sweep algorithm.
 * \var ALGORITHM_GENETIC
 * \brief Genetic algorithm.
 */
enum Algorithm
{
  ALGORITHM_MONTE_CARLO = 0,
  ALGORITHM_SWEEP = 1,
  ALGORITHM_GENETIC = 2
};

/**
 * \struct Input
 * \brief Struct to define the calibration input file.
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
     * \brief Array of experimental data file names.
     * \var label
     * \brief Array of variable names.
     * \var format
     * \brief Array of variable formats.
	 * \var directory
	 * \brief Working directory.
     * \var rangemin
     * \brief Array of minimum variable values.
     * \var rangemax
     * \brief Array of maximum variable values.
     * \var rangeminabs
     * \brief Array of absolute minimum variable values.
     * \var rangemaxabs
     * \brief Array of absolute maximum variable values.
     * \var weight
     * \brief Array of the experiment weights.
     * \var tolerance
     * \brief Algorithm tolerance.
     * \var mutation_ratio
     * \brief Mutation probability.
     * \var reproduction_ratio
     * \brief Reproduction probability.
     * \var adaptation_ratio
     * \brief Adaptation probability.
     * \var nvariables
     * \brief Variables number.
     * \var nexperiments
     * \brief Experiments number.
     * \var ninputs
     * \brief Number of input files to the simulator.
     * \var nsimulations
     * \brief Simulations number per experiment.
     * \var algorithm
     * \brief Algorithm type.
     * \var nsweeps
     * \brief Array of sweeps of the sweep algorithm.
	 * \var nbits
	 * \param Array of bits numbers of the genetic algorithm.
     * \var niterations
     * \brief Number of algorithm iterations
     * \var nbest
     * \brief Number of best simulations.
     */
  char *simulator, *evaluator, **experiment, **template[MAX_NINPUTS], **label,
    **format, *directory;
  double *rangemin, *rangemax, *rangeminabs, *rangemaxabs, *weight, tolerance,
    mutation_ratio, reproduction_ratio, adaptation_ratio;
  unsigned int nvariables, nexperiments, ninputs, nsimulations, algorithm,
    *nsweeps, *nbits, niterations, nbest;
} Input;

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
     * \brief Array of experimental data file names.
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
     * \brief Algorithm type.
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
     * \var nbest
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
     * \var rangeminabs
     * \brief Array of absolute minimum variable values.
     * \var rangemaxabs
     * \brief Array of absolute maximum variable values.
     * \var error_best
     * \brief Array of the best minimum errors.
     * \var weight
     * \brief Array of the experiment weights.
     * \var value_old
     * \brief Array of the best variable values on the previous step.
     * \var error_old
     * \brief Array of the best minimum errors on the previous step.
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
  char *simulator, *evaluator, **experiment, **template[MAX_NINPUTS], **label,
    **format;
  unsigned int nvariables, nexperiments, ninputs, nsimulations, algorithm,
    *nsweeps, nstart, nend, *thread, niterations, nbest, nsaveds,
    *simulation_best;
  double *value, *rangemin, *rangemax, *rangeminabs, *rangemaxabs, *error_best,
    *weight, *value_old, *error_old, tolerance, mutation_ratio,
    reproduction_ratio, adaptation_ratio;
  FILE *result;
  gsl_rng *rng;
  GMappedFile **file[MAX_NINPUTS];
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
     */
  unsigned int thread;
} ParallelData;

// Public functions
void show_error (char *msg);
int xml_node_get_int (xmlNode * node, const xmlChar * prop, int *error_code);
unsigned int xml_node_get_uint (xmlNode * node, const xmlChar * prop,
                                int *error_code);
double xml_node_get_float (xmlNode * node, const xmlChar * prop,
                           int *error_code);
void input_new ();
int input_open (char *filename);

#endif
