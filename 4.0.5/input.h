/*
MPCOTool:
The Multi-Purposes Calibration and Optimization Tool. A software to perform
calibrations or optimizations of empirical parameters.

AUTHORS: Javier Burguete and Borja Latorre.

Copyright 2012-2019, AUTHORS.

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
 * \file input.h
 * \brief Header file to define the input functions.
 * \authors Javier Burguete.
 * \copyright Copyright 2012-2019, all rights reserved.
 */
#ifndef INPUT__H
#define INPUT__H 1

///> Enum to define the methods to estimate the hill climbing.
enum ClimbingMethod
{
  CLIMBING_METHOD_COORDINATES = 0,      ///< Coordinates hill climbing method.
  CLIMBING_METHOD_RANDOM = 1,   ///< Random hill climbing method.
};

///> Enum to define the error norm.
enum ErrorNorm
{
  ERROR_NORM_EUCLIDIAN = 0,
  ///< Euclidian norm: \f$\sqrt{\sum_i\left(w_i\,x_i\right)^2}\f$.
  ERROR_NORM_MAXIMUM = 1,
  ///< Maximum norm: \f$\max_i\left|w_i\,x_i\right|\f$.
  ERROR_NORM_P = 2,
  ///< P-norm \f$\sqrt[p]{\sum_i\left|w_i\,x_i\right|^p}\f$.
  ERROR_NORM_TAXICAB = 3
    ///< Taxicab norm \f$\sum_i\left|w_i\,x_i\right|\f$.
};

/**
 * \struct Input
 * \brief Struct to define the optimization input file.
 */
typedef struct
{
  Experiment *experiment;       ///< Array or experiments.
  Variable *variable;           ///< Array of variables.
  char *result;                 ///< Name of the result file.
  char *variables;              ///< Name of the variables file.
  char *simulator;              ///< Name of the simulator program.
  char *evaluator;
  ///< Name of the program to evaluate the objective function.
  char *directory;              ///< Working directory.
  char *name;                   ///< Input data file name.
  double tolerance;             ///< Algorithm tolerance.
  double mutation_ratio;        ///< Mutation probability.
  double reproduction_ratio;    ///< Reproduction probability.
  double adaptation_ratio;      ///< Adaptation probability.
  double relaxation;            ///< Relaxation parameter.
  double p;                     ///< Exponent of the P error norm.
  double threshold;             ///< Threshold to finish the optimization.
  unsigned long int seed;
  ///< Seed of the pseudo-random numbers generator.
  unsigned int nvariables;      ///< Variables number.
  unsigned int nexperiments;    ///< Experiments number.
  unsigned int nsimulations;    ///< Simulations number per experiment.
  unsigned int algorithm;       ///< Algorithm type.
  unsigned int nsteps;
  ///< Number of steps to do the hill climbing method.
  unsigned int climbing;        ///< Method to estimate the hill climbing.
  unsigned int nestimates;
  ///< Number of simulations to estimate the hill climbing.
  unsigned int niterations;     ///< Number of algorithm iterations
  unsigned int nbest;           ///< Number of best simulations.
  unsigned int norm;            ///< Error norm type.
  unsigned int type;            ///< Type of input file.
} Input;

extern Input input[1];
extern const char *result_name;
extern const char *variables_name;

// Public functions
void input_new ();
void input_free ();
int input_open (char *filename);

#endif
