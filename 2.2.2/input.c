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
 * \file input.c
 * \brief Source file to define the input functions.
 * \authors Javier Burguete and Borja Latorre.
 * \copyright Copyright 2012-2016, all rights reserved.
 */
#define _GNU_SOURCE
#include "config.h"
#include <stdio.h>
#include <libxml/parser.h>
#include <libintl.h>
#include <glib.h>
#include <glib/gstdio.h>
#include "utils.h"
#include "experiment.h"
#include "variable.h"
#include "input.h"

#define DEBUG_INPUT 0           ///< Macro to debug input functions.

Input input[1];

const xmlChar *result_name = (xmlChar *) "result";
  ///< Name of the result file.
const xmlChar *variables_name = (xmlChar *) "variables";
  ///< Name of the variables file.

/**
 * \fn void input_new ()
 * \brief Function to create a new Input struct.
 */
void
input_new ()
{
#if DEBUG_INPUT
  fprintf (stderr, "input_new: start\n");
#endif
  input->nvariables = input->nexperiments = input->nsteps = 0;
  input->simulator = input->evaluator = input->directory = input->name = NULL;
  input->experiment = NULL;
  input->variable = NULL;
#if DEBUG_INPUT
  fprintf (stderr, "input_new: end\n");
#endif
}

/**
 * \fn void input_free ()
 * \brief Function to free the memory of the input file data.
 */
void
input_free ()
{
  unsigned int i;
#if DEBUG_INPUT
  fprintf (stderr, "input_free: start\n");
#endif
  g_free (input->name);
  g_free (input->directory);
  for (i = 0; i < input->nexperiments; ++i)
    experiment_free (input->experiment + i);
  g_free (input->experiment);
  for (i = 0; i < input->nvariables; ++i)
    variable_free (input->variable + i);
  g_free (input->variable);
  xmlFree (input->evaluator);
  xmlFree (input->simulator);
  xmlFree (input->result);
  xmlFree (input->variables);
  input->nexperiments = input->nvariables = input->nsteps = 0;
#if DEBUG_INPUT
  fprintf (stderr, "input_free: end\n");
#endif
}

/**
 * \fn void input_error (char *message)
 * \brief Function to print an error message opening an Input struct.
 * \param message
 * \brief Error message.
 */
void
input_error (char *message)
{
  char buffer[64];
  snprintf (buffer, 64, "%s: %s\n", gettext ("Input"), message);
  error_message = g_strdup (buffer);
}

/**
 * \fn int input_open (char *filename)
 * \brief Function to open the input file.
 * \param filename
 * \brief Input data file name.
 * \return 1_on_success, 0_on_error.
 */
int
input_open (char *filename)
{
  char buffer2[64];
  xmlDoc *doc;
  xmlNode *node, *child;
  xmlChar *buffer;
  int error_code;
  unsigned int i;

#if DEBUG_INPUT
  fprintf (stderr, "input_open: start\n");
#endif

  // Resetting input data
  buffer = NULL;
  input_new ();

  // Parsing the input file
#if DEBUG_INPUT
  fprintf (stderr, "input_open: parsing the input file %s\n", filename);
#endif
  doc = xmlParseFile (filename);
  if (!doc)
    {
      input_error (gettext ("Unable to parse the input file"));
      goto exit_on_error;
    }

  // Getting the root node
#if DEBUG_INPUT
  fprintf (stderr, "input_open: getting the root node\n");
#endif
  node = xmlDocGetRootElement (doc);
  if (xmlStrcmp (node->name, XML_OPTIMIZE))
    {
      input_error (gettext ("Bad root XML node"));
      goto exit_on_error;
    }

  // Getting result and variables file names
  if (!input->result)
    {
      input->result = (char *) xmlGetProp (node, XML_RESULT);
      if (!input->result)
        input->result = (char *) xmlStrdup (result_name);
    }
  if (!input->variables)
    {
      input->variables = (char *) xmlGetProp (node, XML_VARIABLES);
      if (!input->variables)
        input->variables = (char *) xmlStrdup (variables_name);
    }

  // Opening simulator program name
  input->simulator = (char *) xmlGetProp (node, XML_SIMULATOR);
  if (!input->simulator)
    {
      input_error (gettext ("Bad simulator program"));
      goto exit_on_error;
    }

  // Opening evaluator program name
  input->evaluator = (char *) xmlGetProp (node, XML_EVALUATOR);

  // Obtaining pseudo-random numbers generator seed
  input->seed
    = xml_node_get_uint_with_default (node, XML_SEED, DEFAULT_RANDOM_SEED,
                                      &error_code);
  if (error_code)
    {
      input_error (gettext ("Bad pseudo-random numbers generator seed"));
      goto exit_on_error;
    }

  // Opening algorithm
  buffer = xmlGetProp (node, XML_ALGORITHM);
  if (!xmlStrcmp (buffer, XML_MONTE_CARLO))
    {
      input->algorithm = ALGORITHM_MONTE_CARLO;

      // Obtaining simulations number
      input->nsimulations
        = xml_node_get_int (node, XML_NSIMULATIONS, &error_code);
      if (error_code)
        {
          input_error (gettext ("Bad simulations number"));
          goto exit_on_error;
        }
    }
  else if (!xmlStrcmp (buffer, XML_SWEEP))
    input->algorithm = ALGORITHM_SWEEP;
  else if (!xmlStrcmp (buffer, XML_GENETIC))
    {
      input->algorithm = ALGORITHM_GENETIC;

      // Obtaining population
      if (xmlHasProp (node, XML_NPOPULATION))
        {
          input->nsimulations
            = xml_node_get_uint (node, XML_NPOPULATION, &error_code);
          if (error_code || input->nsimulations < 3)
            {
              input_error (gettext ("Invalid population number"));
              goto exit_on_error;
            }
        }
      else
        {
          input_error (gettext ("No population number"));
          goto exit_on_error;
        }

      // Obtaining generations
      if (xmlHasProp (node, XML_NGENERATIONS))
        {
          input->niterations
            = xml_node_get_uint (node, XML_NGENERATIONS, &error_code);
          if (error_code || !input->niterations)
            {
              input_error (gettext ("Invalid generations number"));
              goto exit_on_error;
            }
        }
      else
        {
          input_error (gettext ("No generations number"));
          goto exit_on_error;
        }

      // Obtaining mutation probability
      if (xmlHasProp (node, XML_MUTATION))
        {
          input->mutation_ratio
            = xml_node_get_float (node, XML_MUTATION, &error_code);
          if (error_code || input->mutation_ratio < 0.
              || input->mutation_ratio >= 1.)
            {
              input_error (gettext ("Invalid mutation probability"));
              goto exit_on_error;
            }
        }
      else
        {
          input_error (gettext ("No mutation probability"));
          goto exit_on_error;
        }

      // Obtaining reproduction probability
      if (xmlHasProp (node, XML_REPRODUCTION))
        {
          input->reproduction_ratio
            = xml_node_get_float (node, XML_REPRODUCTION, &error_code);
          if (error_code || input->reproduction_ratio < 0.
              || input->reproduction_ratio >= 1.0)
            {
              input_error (gettext ("Invalid reproduction probability"));
              goto exit_on_error;
            }
        }
      else
        {
          input_error (gettext ("No reproduction probability"));
          goto exit_on_error;
        }

      // Obtaining adaptation probability
      if (xmlHasProp (node, XML_ADAPTATION))
        {
          input->adaptation_ratio
            = xml_node_get_float (node, XML_ADAPTATION, &error_code);
          if (error_code || input->adaptation_ratio < 0.
              || input->adaptation_ratio >= 1.)
            {
              input_error (gettext ("Invalid adaptation probability"));
              goto exit_on_error;
            }
        }
      else
        {
          input_error (gettext ("No adaptation probability"));
          goto exit_on_error;
        }

      // Checking survivals
      i = input->mutation_ratio * input->nsimulations;
      i += input->reproduction_ratio * input->nsimulations;
      i += input->adaptation_ratio * input->nsimulations;
      if (i > input->nsimulations - 2)
        {
          input_error (gettext
                       ("No enough survival entities to reproduce the population"));
          goto exit_on_error;
        }
    }
  else
    {
      input_error (gettext ("Unknown algorithm"));
      goto exit_on_error;
    }
  xmlFree (buffer);
  buffer = NULL;

  if (input->algorithm == ALGORITHM_MONTE_CARLO
      || input->algorithm == ALGORITHM_SWEEP)
    {

      // Obtaining iterations number
      input->niterations
        = xml_node_get_uint (node, XML_NITERATIONS, &error_code);
      if (error_code == 1)
        input->niterations = 1;
      else if (error_code)
        {
          input_error (gettext ("Bad iterations number"));
          goto exit_on_error;
        }

      // Obtaining best number
      input->nbest
        = xml_node_get_uint_with_default (node, XML_NBEST, 1, &error_code);
      if (error_code || !input->nbest)
        {
          input_error (gettext ("Invalid best number"));
          goto exit_on_error;
        }

      // Obtaining tolerance
      input->tolerance
        = xml_node_get_float_with_default (node, XML_TOLERANCE, 0.,
                                           &error_code);
      if (error_code || input->tolerance < 0.)
        {
          input_error (gettext ("Invalid tolerance"));
          goto exit_on_error;
        }

      // Getting direction search method parameters
      if (xmlHasProp (node, XML_NSTEPS))
        {
          input->nsteps = xml_node_get_uint (node, XML_NSTEPS, &error_code);
          if (error_code || !input->nsteps)
            {
              input_error (gettext ("Invalid steps number"));
              goto exit_on_error;
            }
          buffer = xmlGetProp (node, XML_DIRECTION);
          if (!xmlStrcmp (buffer, XML_COORDINATES))
            input->direction = DIRECTION_METHOD_COORDINATES;
          else if (!xmlStrcmp (buffer, XML_RANDOM))
            {
              input->direction = DIRECTION_METHOD_RANDOM;
              input->nestimates
                = xml_node_get_uint (node, XML_NESTIMATES, &error_code);
              if (error_code || !input->nestimates)
                {
                  input_error (gettext ("Invalid estimates number"));
                  goto exit_on_error;
                }
            }
          else
            {
              input_error
                (gettext ("Unknown method to estimate the direction search"));
              goto exit_on_error;
            }
          xmlFree (buffer);
          buffer = NULL;
          input->relaxation
            = xml_node_get_float_with_default (node, XML_RELAXATION,
                                               DEFAULT_RELAXATION, &error_code);
          if (error_code || input->relaxation < 0. || input->relaxation > 2.)
            {
              input_error (gettext ("Invalid relaxation parameter"));
              goto exit_on_error;
            }
        }
      else
        input->nsteps = 0;
    }
  // Obtaining the thresold
  input->thresold = xml_node_get_float_with_default (node, XML_THRESOLD, 0.,
                                                     &error_code);
  if (error_code)
    {
      input_error (gettext ("Invalid thresold"));
      goto exit_on_error;
    }

  // Reading the experimental data
  for (child = node->children; child; child = child->next)
    {
      if (xmlStrcmp (child->name, XML_EXPERIMENT))
        break;
#if DEBUG_INPUT
      fprintf (stderr, "input_open: nexperiments=%u\n", input->nexperiments);
#endif
      input->experiment = (Experiment *)
        g_realloc (input->experiment,
                   (1 + input->nexperiments) * sizeof (Experiment));
      if (!input->nexperiments)
        {
          if (!experiment_open (input->experiment, child, 0))
            goto exit_on_error;
        }
      else
        {
          if (!experiment_open (input->experiment + input->nexperiments, child,
                                input->experiment->ninputs))
            goto exit_on_error;
        }
      ++input->nexperiments;
#if DEBUG_INPUT
      fprintf (stderr, "input_open: nexperiments=%u\n", input->nexperiments);
#endif
    }
  if (!input->nexperiments)
    {
      input_error (gettext ("No optimization experiments"));
      goto exit_on_error;
    }
  buffer = NULL;

  // Reading the variables data
  for (; child; child = child->next)
    {
#if DEBUG_INPUT
      fprintf (stderr, "input_open: nvariables=%u\n", input->nvariables);
#endif
      if (xmlStrcmp (child->name, XML_VARIABLE))
        {
          snprintf (buffer2, 64, "%s %u: %s",
                    gettext ("Variable"),
                    input->nvariables + 1, gettext ("bad XML node"));
          input_error (buffer2);
          goto exit_on_error;
        }
      input->variable = (Variable *)
        g_realloc (input->variable,
                   (1 + input->nvariables) * sizeof (Variable));
      if (!variable_open (input->variable + input->nvariables, child,
                          input->algorithm, input->nsteps))
        goto exit_on_error;
      ++input->nvariables;
    }
  if (!input->nvariables)
    {
      input_error (gettext ("No optimization variables"));
      goto exit_on_error;
    }
  buffer = NULL;

  // Obtaining the error norm
  if (xmlHasProp (node, XML_NORM))
    {
      buffer = xmlGetProp (node, XML_NORM);
      if (!xmlStrcmp (buffer, XML_EUCLIDIAN))
        input->norm = ERROR_NORM_EUCLIDIAN;
      else if (!xmlStrcmp (buffer, XML_MAXIMUM))
        input->norm = ERROR_NORM_MAXIMUM;
      else if (!xmlStrcmp (buffer, XML_P))
        {
          input->norm = ERROR_NORM_P;
          input->p = xml_node_get_float (node, XML_P, &error_code);
          if (!error_code)
            {
              input_error (gettext ("Bad P parameter"));
              goto exit_on_error;
            }
        }
      else if (!xmlStrcmp (buffer, XML_TAXICAB))
        input->norm = ERROR_NORM_TAXICAB;
      else
        {
          input_error (gettext ("Unknown error norm"));
          goto exit_on_error;
        }
      xmlFree (buffer);
    }
  else
    input->norm = ERROR_NORM_EUCLIDIAN;

  // Getting the working directory
  input->directory = g_path_get_dirname (filename);
  input->name = g_path_get_basename (filename);

  // Closing the XML document
  xmlFreeDoc (doc);

#if DEBUG_INPUT
  fprintf (stderr, "input_open: end\n");
#endif
  return 1;

exit_on_error:
  xmlFree (buffer);
  xmlFreeDoc (doc);
  show_error (error_message);
  g_free (error_message);
  input_free ();
#if DEBUG_INPUT
  fprintf (stderr, "input_open: end\n");
#endif
  return 0;
}
