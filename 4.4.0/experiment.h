/*
MPCOTool:
The Multi-Purposes Calibration and Optimization Tool. A software to perform
calibrations or optimizations of empirical parameters.

AUTHORS: Javier Burguete and Borja Latorre.

Copyright 2012-2023, AUTHORS.

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
 * \file experiment.h
 * \brief Header file to define the experiment data.
 * \authors Javier Burguete.
 * \copyright Copyright 2012-2023, all rights reserved.
 */
#ifndef EXPERIMENT__H
#define EXPERIMENT__H 1

/**
 * \struct Experiment
 * \brief Struct to define the experiment data.
 */
typedef struct
{
  char *name;                   ///< File name.
  char *stencil[MAX_NINPUTS];   ///< Array of template names of input files.
  double weight;                ///< Objective function weight.
  unsigned int ninputs;         ///< Number of input files to the simulator.
} Experiment;

extern const char *stencil[MAX_NINPUTS];

// Public functions
void experiment_free (Experiment * experiment, unsigned int type);
void experiment_error (Experiment * experiment, char *message);
int experiment_open_xml (Experiment * experiment, xmlNode * node,
                         unsigned int ninputs);
int experiment_open_json (Experiment * experiment, JsonNode * node,
                          unsigned int ninputs);

#endif
