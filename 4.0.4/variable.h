/*
MPCOTool:
The Multi-Purposes Calibration and Optimization Tool. A software to perform
calibrations or optimizations of empirical parameters.

AUTHORS: Javier Burguete and Borja Latorre.

Copyright 2012-2018, AUTHORS.

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
 * \file variable.h
 * \brief Header file to define the variable data.
 * \authors Javier Burguete.
 * \copyright Copyright 2012-2018, all rights reserved.
 */
#ifndef VARIABLE__H
#define VARIABLE__H 1

///> Enum to define the algorithms.
enum Algorithm
{
  ALGORITHM_MONTE_CARLO = 0,    ///< Monte-Carlo algorithm.
  ALGORITHM_SWEEP = 1,          ///< Sweep algorithm.
  ALGORITHM_GENETIC = 2,        ///< Genetic algorithm.
  ALGORITHM_ORTHOGONAL = 3      ///< Orthogonal sampling algorithm.
};

/**
 * \struct Variable
 * \brief Struct to define the variable data.
 */
typedef struct
{
  char *name;                   ///< Variable name.
  double rangemin;              ///< Minimum variable value.
  double rangemax;              ///< Maximum variable value.
  double rangeminabs;           ///< Absolute minimum variable value.
  double rangemaxabs;           ///< Absolute maximum variable value.
  double step;                  ///< Hill climbing method step size.
  unsigned int precision;       ///< Variable precision.
  unsigned int nsweeps;         ///< Sweeps of the sweep algorithm.
  unsigned int nbits;           ///< Bits number of the genetic algorithm.
} Variable;

extern const char *format[NPRECISIONS];
extern const double precision[NPRECISIONS];

// Public functions
void variable_free (Variable * variable, unsigned int type);
void variable_error (Variable * variable, char *message);
int variable_open_xml (Variable * variable, xmlNode * node,
                       unsigned int algorithm, unsigned int nsteps);
int variable_open_json (Variable * variable, JsonNode * node,
                        unsigned int algorithm, unsigned int nsteps);

#endif
