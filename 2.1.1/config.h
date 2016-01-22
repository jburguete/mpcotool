/* config.h.  Generated from config.h.in by configure.  */
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
 * \file config.h
 * \brief Configuration header file.
 * \authors Javier Burguete and Borja Latorre.
 * \copyright Copyright 2012-2016, all rights reserved.
 */
#ifndef CONFIG__H
#define CONFIG__H 1

// Array sizes

#define MAX_NINPUTS 8
  ///< Maximum number of input files in the simulator program.
#define NALGORITHMS 3           ///< Number of stochastic algorithms.
#define NDIRECTIONS 2            ///< Number of direction estimate methods.
#define NNORMS 4                ///< Number of error norms.
#define NPRECISIONS 15          ///< Number of precisions.

// Default choices

#define DEFAULT_PRECISION (NPRECISIONS - 1)     ///< Default precision digits.
#define DEFAULT_RANDOM_SEED 7007        ///< Default pseudo-random numbers seed.
#define DEFAULT_RELAXATION 1.   ///< Default relaxation parameter.

// Interface labels

#define LOCALE_DIR "locales"    ///< Locales directory.
#define PROGRAM_INTERFACE "mpcotool"    ///< Name of the interface program.

// XML labels

#define XML_ABSOLUTE_MINIMUM (const xmlChar*)"absolute_minimum"
  ///< absolute minimum XML label.
#define XML_ABSOLUTE_MAXIMUM (const xmlChar*)"absolute_maximum"
  ///< absolute maximum XML label.
#define XML_ADAPTATION (const xmlChar*)"adaptation"
  ///< adaption XML label.
#define XML_ALGORITHM (const xmlChar*)"algorithm"
  ///< algoritm XML label.
#define XML_OPTIMIZE (const xmlChar*)"optimize"
  ///< optimize XML label.
#define XML_COORDINATES (const xmlChar*)"coordinates"
  ///< coordinates XML label.
#define XML_DIRECTION (const xmlChar*)"direction"
  ///< direction XML label.
#define XML_EUCLIDIAN (const xmlChar*)"euclidian"
  ///< euclidian XML label.
#define XML_EVALUATOR (const xmlChar*)"evaluator"
  ///< evaluator XML label.
#define XML_EXPERIMENT (const xmlChar*)"experiment"
  ///< experiment XML label.
#define XML_GENETIC (const xmlChar*)"genetic"   ///< genetic XML label.
#define XML_MINIMUM (const xmlChar*)"minimum"   ///< minimum XML label.
#define XML_MAXIMUM (const xmlChar*)"maximum"   ///< maximum XML label.
#define XML_MONTE_CARLO (const xmlChar*)"Monte-Carlo"
  ///< Monte-Carlo XML label.
#define XML_MUTATION (const xmlChar*)"mutation" ///< mutation XML label.
#define XML_NAME (const xmlChar*)"name" ///< name XML label.
#define XML_NBEST (const xmlChar*)"nbest"       ///< nbest XML label.
#define XML_NBITS (const xmlChar*)"nbits"       ///< nbits XML label.
#define XML_NESTIMATES (const xmlChar*)"nestimates"
  ///< nestimates XML label.
#define XML_NGENERATIONS (const xmlChar*)"ngenerations"
  ///< ngenerations XML label.
#define XML_NITERATIONS (const xmlChar*)"niterations"
  ///< niterations XML label.
#define XML_NORM (const xmlChar*)"norm" ///< norm XML label.
#define XML_NPOPULATION (const xmlChar*)"npopulation"
  ///< npopulation XML label.
#define XML_NSIMULATIONS (const xmlChar*)"nsimulations"
  ///< nsimulations XML label.
#define XML_NSTEPS (const xmlChar*)"nsteps"     ///< nsteps XML label.
#define XML_NSWEEPS (const xmlChar*)"nsweeps"   ///< nsweeps XML label.
#define XML_P (const xmlChar*)"p"       ///< p XML label.
#define XML_PRECISION (const xmlChar*)"precision"
  ///< precision XML label.
#define XML_RANDOM (const xmlChar*)"random"     ///< random XML label.
#define XML_RELAXATION (const xmlChar*)"relaxation"
  ///< relaxation XML label.
#define XML_REPRODUCTION (const xmlChar*)"reproduction"
  ///< reproduction XML label.
#define XML_RESULT (const xmlChar*)"result"     ///< result XML label.
#define XML_SIMULATOR (const xmlChar*)"simulator"
  ///< simulator XML label.
#define XML_SEED (const xmlChar*)"seed" ///< seed XML label.
#define XML_STEP (const xmlChar*)"step" ///< step XML label.
#define XML_SWEEP (const xmlChar*)"sweep"       ///< sweep XML label.
#define XML_TAXICAB (const xmlChar*)"taxicab"   ///< taxicab XML label.
#define XML_TEMPLATE1 (const xmlChar*)"template1"
  ///< template1 XML label.
#define XML_TEMPLATE2 (const xmlChar*)"template2"
  ///< template2 XML label.
#define XML_TEMPLATE3 (const xmlChar*)"template3"
  ///< template3 XML label.
#define XML_TEMPLATE4 (const xmlChar*)"template4"
  ///< template4 XML label.
#define XML_TEMPLATE5 (const xmlChar*)"template5"
  ///< template5 XML label.
#define XML_TEMPLATE6 (const xmlChar*)"template6"
  ///< template6 XML label.
#define XML_TEMPLATE7 (const xmlChar*)"template7"
  ///< template7 XML label.
#define XML_TEMPLATE8 (const xmlChar*)"template8"
  ///< template8 XML label.
#define XML_THRESOLD (const xmlChar*)"thresold"
  ///< thresold XML label.
#define XML_TOLERANCE (const xmlChar*)"tolerance"
  ///< tolerance XML label.
#define XML_VARIABLE (const xmlChar*)"variable" ///< variable XML label.
#define XML_VARIABLES (const xmlChar*)"variables"
  ///< variables XML label.
#define XML_WEIGHT (const xmlChar*)"weight"     ///< weight XML label.

#endif
