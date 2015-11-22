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
 */
enum Algorithm
{
  ALGORITHM_MONTE_CARLO = 0,    ///< Monte-Carlo algorithm.
  ALGORITHM_SWEEP = 1,          ///< Sweep algorithm.
  ALGORITHM_GENETIC = 2         ///< Genetic algorithm.
};

/**
 * \struct Input
 * \brief Struct to define the calibration input file.
 */
typedef struct
{
  char **template[MAX_NINPUTS]; ///< Matrix of template names of input files.
  char **experiment;            ///< Array of experimental data file names.
  char **label;                 ///< Array of variable names.
  char *result;                 ///< Name of the result file.
  char *variables;              ///< Name of the variables file.
  char *simulator;              ///< Name of the simulator program.
  char *evaluator;
  ///< Name of the program to evaluate the objective function.
  char *directory;              ///< Working directory.
  char *name;                   ///< Input data file name.
  double *rangemin;             ///< Array of minimum variable values.
  double *rangemax;             ///< Array of maximum variable values.
  double *rangeminabs;          ///< Array of absolute minimum variable values.
  double *rangemaxabs;          ///< Array of absolute maximum variable values.
  double *weight;               ///< Array of the experiment weights.
  double *step;                 ///< Array of gradient based method step sizes.
  unsigned int *precision;      ///< Array of variable precisions.
  unsigned int *nsweeps;        ///< Array of sweeps of the sweep algorithm.
  unsigned int *nbits;
  ///< Array of bits numbers of the genetic algorithm.
  double tolerance;             ///< Algorithm tolerance.
  double mutation_ratio;        ///< Mutation probability.
  double reproduction_ratio;    ///< Reproduction probability.
  double adaptation_ratio;      ///< Adaptation probability.
  unsigned long int seed;
  ///< Seed of the pseudo-random numbers generator.
  unsigned int nvariables;      ///< Variables number.
  unsigned int nexperiments;    ///< Experiments number.
  unsigned int ninputs;         ///< Number of input files to the simulator.
  unsigned int nsimulations;    ///< Simulations number per experiment.
  unsigned int algorithm;       ///< Algorithm type.
  unsigned int nestimates;
  ///< Number of simulations to estimate the gradient.
  unsigned int nsteps;
  ///< Number of steps to do the gradient based method.
  unsigned int niterations;     ///< Number of algorithm iterations
  unsigned int nbest;           ///< Number of best simulations.
} Input;

/**
 * \struct Calibrate
 * \brief Struct to define the calibration data.
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
  double *step;                 ///< Array of gradient based method step sizes.
  double *value_old;
  ///< Array of the best variable values on the previous step.
  double *error_old;
  ///< Array of the best minimum errors on the previous step.
  unsigned int *precision;      ///< Array of variable precisions.
  unsigned int *nsweeps;        ///< Array of sweeps of the sweep algorithm.
  unsigned int *thread;
  ///< Array of simulation numbers to calculate on the thread.
  unsigned int *thread_gradient;
  ///< Array of simulation numbers to calculate on the thread for the gradient
  ///< based method.
  unsigned int *simulation_best;        ///< Array of best simulation numbers.
  double tolerance;             ///< Algorithm tolerance.
  double mutation_ratio;        ///< Mutation probability.
  double reproduction_ratio;    ///< Reproduction probability.
  double adaptation_ratio;      ///< Adaptation probability.
  double calculation_time;      ///< Calculation time.
  unsigned long int seed;
  ///< Seed of the pseudo-random numbers generator.
  unsigned int nvariables;      ///< Variables number.
  unsigned int nexperiments;    ///< Experiments number.
  unsigned int ninputs;         ///< Number of input files to the simulator.
  unsigned int nsimulations;    ///< Simulations number per experiment.
  unsigned int nestimates;
  ///< Number of simulations to estimate the gradient.
  unsigned int nsteps;
  ///< Number of steps for the gradient based method.
  unsigned int algorithm;       ///< Algorithm type.
  unsigned int nstart;          ///< Beginning simulation number of the task.
  unsigned int nend;            ///< Ending simulation number of the task.
  unsigned int nstart_gradient;
  ///< Beginning simulation number of the task for the gradient based method.
  unsigned int nend_gradient;
  ///< Ending simulation number of the task for the gradient based method.
  unsigned int niterations;     ///< Number of algorithm iterations
  unsigned int nbest;           ///< Number of best simulations.
  unsigned int nsaveds;         ///< Number of saved simulations.
#if HAVE_MPI
  int mpi_rank;                 ///< Number of MPI task.
#endif
} Calibrate;

/**
 * \struct ParallelData
 * \brief Struct to pass to the GThreads parallelized function.
 */
typedef struct
{
  unsigned int thread;          ///< Thread number.
} ParallelData;

// Public functions
void show_message (char *title, char *msg, int type);
void show_error (char *msg);
int xml_node_get_int (xmlNode * node, const xmlChar * prop, int *error_code);
unsigned int xml_node_get_uint (xmlNode * node, const xmlChar * prop,
                                int *error_code);
double xml_node_get_float (xmlNode * node, const xmlChar * prop,
                           int *error_code);
void xml_node_set_int (xmlNode * node, const xmlChar * prop, int value);
void xml_node_set_uint (xmlNode * node, const xmlChar * prop,
                        unsigned int value);
void xml_node_set_float (xmlNode * node, const xmlChar * prop, double value);
void input_new ();
void input_free ();
int input_open (char *filename);
void calibrate_input (unsigned int simulation, char *input,
                      GMappedFile * template);
double calibrate_parse (unsigned int simulation, unsigned int experiment);
void calibrate_print ();
void calibrate_save_variables (unsigned int simulation, double error);
void calibrate_best (unsigned int simulation, double value);
void calibrate_best_sequential (unsigned int simulation, double value);
void calibrate_best_thread (unsigned int simulation, double value);
void calibrate_sequential ();
void *calibrate_thread (ParallelData * data);
void calibrate_merge (unsigned int nsaveds, unsigned int *simulation_best,
                      double *error_best);
#if HAVE_MPI
void calibrate_synchronise ();
#endif
void calibrate_sweep ();
void calibrate_MonteCarlo ();
double calibrate_genetic_objective (Entity * entity);
void calibrate_genetic ();
void calibrate_save_old ();
void calibrate_merge_old ();
void calibrate_refine ();
void calibrate_iterate ();
void calibrate_new ();

#endif
