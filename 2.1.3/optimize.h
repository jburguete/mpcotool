/*
MPCOTool:
The Multi-Purposes Calibration and Optimization Tool. A software to perform
calibrations or optimizations of empirical parameters.

AUTHORS: Javier Burguete and Borja Latorre.

Copyright 2012-2016, AUTHORS.

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
 * \file optimize.h
 * \brief Header file to define the optimization functions.
 * \authors Javier Burguete.
 * \copyright Copyright 2012-2016, all rights reserved.
 */
#ifndef OPTIMIZE__H
#define OPTIMIZE__H 1

/**
 * \struct Optimize
 * \brief Struct to define the optimization ation data.
 */
typedef struct
{
  GMappedFile **file[MAX_NINPUTS];      ///< Matrix of input template files.
  char **template[MAX_NINPUTS]; ///< Matrix of template names of input files.
  char **experiment;            ///< Array of experimental data file names.
  char **label;                 ///< Array of variable names.
  gsl_rng *rng;                 ///< GSL random number generator.
  GeneticVariable *genetic_variable;
  ///< Array of variables for the genetic algorithm.
  FILE *file_result;            ///< Result file.
  FILE *file_variables;         ///< Variables file.
  char *result;                 ///< Name of the result file.
  char *variables;              ///< Name of the variables file.
  char *simulator;              ///< Name of the simulator program.
  char *evaluator;
  ///< Name of the program to evaluate the objective function.
  double *value;                ///< Array of variable values.
  double *rangemin;             ///< Array of minimum variable values.
  double *rangemax;             ///< Array of maximum variable values.
  double *rangeminabs;          ///< Array of absolute minimum variable values.
  double *rangemaxabs;          ///< Array of absolute maximum variable values.
  double *error_best;           ///< Array of the best minimum errors.
  double *weight;               ///< Array of the experiment weights.
  double *step;
  ///< Array of direction search method step sizes.
  double *direction;            ///< Vector of direction search estimation.
  double *value_old;
  ///< Array of the best variable values on the previous step.
  double *error_old;
  ///< Array of the best minimum errors on the previous step.
  unsigned int *precision;      ///< Array of variable precisions.
  unsigned int *nsweeps;        ///< Array of sweeps of the sweep algorithm.
  unsigned int *thread;
  ///< Array of simulation numbers to calculate on the thread.
  unsigned int *thread_direction;
  ///< Array of simulation numbers to calculate on the thread for the direction
  ///< search method.
  unsigned int *simulation_best;        ///< Array of best simulation numbers.
  double tolerance;             ///< Algorithm tolerance.
  double mutation_ratio;        ///< Mutation probability.
  double reproduction_ratio;    ///< Reproduction probability.
  double adaptation_ratio;      ///< Adaptation probability.
  double relaxation;            ///< Relaxation parameter.
  double calculation_time;      ///< Calculation time.
  double p;                     ///< Exponent of the P error norm.
  double thresold;              ///< Thresold to finish the optimization.
  unsigned long int seed;
  ///< Seed of the pseudo-random numbers generator.
  unsigned int nvariables;      ///< Variables number.
  unsigned int nexperiments;    ///< Experiments number.
  unsigned int ninputs;         ///< Number of input files to the simulator.
  unsigned int nsimulations;    ///< Simulations number per experiment.
  unsigned int nsteps;
  ///< Number of steps for the direction search method.
  unsigned int nestimates;
  ///< Number of simulations to estimate the direction.
  unsigned int algorithm;       ///< Algorithm type.
  unsigned int nstart;          ///< Beginning simulation number of the task.
  unsigned int nend;            ///< Ending simulation number of the task.
  unsigned int nstart_direction;
  ///< Beginning simulation number of the task for the direction search method.
  unsigned int nend_direction;
  ///< Ending simulation number of the task for the direction search method.
  unsigned int niterations;     ///< Number of algorithm iterations
  unsigned int nbest;           ///< Number of best simulations.
  unsigned int nsaveds;         ///< Number of saved simulations.
  unsigned int stop;            ///< To stop the simulations.
#if HAVE_MPI
  int mpi_rank;                 ///< Number of MPI task.
#endif
} Optimize;

/**
 * \struct ParallelData
 * \brief Struct to pass to the GThreads parallelized function.
 */
typedef struct
{
  unsigned int thread;          ///< Thread number.
} ParallelData;

// Global variables
extern int ntasks;
extern unsigned int nthreads;
extern unsigned int nthreads_direction;
extern GMutex mutex[1];
extern void (*optimize_algorithm) ();
extern double (*optimize_estimate_direction) (unsigned int variable,
                                              unsigned int estimate);
extern double (*optimize_norm) (unsigned int simulation);
extern Optimize optimize[1];
extern const xmlChar *result_name;
extern const xmlChar *variables_name;

// Public functions
void optimize_input (unsigned int simulation, char *input,
                     GMappedFile * template);
double optimize_parse (unsigned int simulation, unsigned int experiment);
double optimize_norm_euclidian (unsigned int simulation);
double optimize_norm_maximum (unsigned int simulation);
double optimize_norm_p (unsigned int simulation);
double optimize_norm_taxicab (unsigned int simulation);
void optimize_print ();
void optimize_save_variables (unsigned int simulation, double error);
void optimize_best (unsigned int simulation, double value);
void optimize_sequential ();
void *optimize_thread (ParallelData * data);
void optimize_merge (unsigned int nsaveds, unsigned int *simulation_best,
                     double *error_best);
#if HAVE_MPI
void optimize_synchronise ();
#endif
void optimize_sweep ();
void optimize_MonteCarlo ();
void optimize_best_direction (unsigned int simulation, double value);
void optimize_direction_sequential ();
void *optimize_direction_thread (ParallelData * data);
double optimize_estimate_direction_random (unsigned int variable,
                                           unsigned int estimate);
double optimize_estimate_direction_coordinates (unsigned int variable,
                                                unsigned int estimate);
void optimize_step_direction (unsigned int simulation);
void optimize_direction ();
double optimize_genetic_objective (Entity * entity);
void optimize_genetic ();
void optimize_save_old ();
void optimize_merge_old ();
void optimize_refine ();
void optimize_step ();
void optimize_iterate ();
void optimize_free ();
void optimize_open ();

#endif
