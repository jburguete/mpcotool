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
 * \file input.c
 * \brief Source file to define the input functions.
 * \authors Javier Burguete and Borja Latorre.
 * \copyright Copyright 2012-2023, all rights reserved.
 */
#define _GNU_SOURCE
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>
#include <libintl.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <json-glib/json-glib.h>
#include "jb/src/jb_xml.h"
#include "jb/src/jb_json.h"
#include "jb/src/jb_win.h"
#include "tools.h"
#include "experiment.h"
#include "variable.h"
#include "input.h"

#define DEBUG_INPUT 0           ///< Macro to debug input functions.

Input input[1];                 ///< Global Input struct to set the input data.

const char *result_name = "result";     ///< Name of the result file.
const char *variables_name = "variables";       ///< Name of the variables file.

/**
 * Function to create a new Input struct.
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
 * Function to free the memory of the input file data.
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
    experiment_free (input->experiment + i, input->type);
  for (i = 0; i < input->nvariables; ++i)
    variable_free (input->variable + i, input->type);
  g_free (input->experiment);
  g_free (input->variable);
  if (input->type == INPUT_TYPE_XML)
    {
      xmlFree (input->evaluator);
      xmlFree (input->simulator);
      xmlFree (input->result);
      xmlFree (input->variables);
    }
  else
    {
      g_free (input->evaluator);
      g_free (input->simulator);
      g_free (input->result);
      g_free (input->variables);
    }
  input->nexperiments = input->nvariables = input->nsteps = 0;
#if DEBUG_INPUT
  fprintf (stderr, "input_free: end\n");
#endif
}

/**
 * Function to print an error message opening an Input struct.
 */
static void
input_error (char *message)     ///< Error message.
{
  error_message = g_strconcat (_("Input"), ": ", message, "\n", NULL);
}

/**
 * Function to open the input file in XML format.
 *
 * \return 1_on_success, 0_on_error.
 */
static inline int
input_open_xml (xmlDoc * doc)   ///< xmlDoc struct.
{
  char buffer2[64];
  Experiment *experiment;
  xmlNode *node, *child;
  xmlChar *buffer;
  int error_code;
  unsigned int i;

#if DEBUG_INPUT
  fprintf (stderr, "input_open_xml: start\n");
#endif

  // Resetting input data
  buffer = NULL;
  input->type = INPUT_TYPE_XML;

  // Getting the root node
#if DEBUG_INPUT
  fprintf (stderr, "input_open_xml: getting the root node\n");
#endif
  node = xmlDocGetRootElement (doc);
  if (xmlStrcmp (node->name, (const xmlChar *) LABEL_OPTIMIZE))
    {
      input_error (_("Bad root XML node"));
      goto exit_on_error;
    }

  // Getting result and variables file names
  if (!input->result)
    {
      input->result =
        (char *) xmlGetProp (node, (const xmlChar *) LABEL_RESULT_FILE);
      if (!input->result)
        input->result = (char *) xmlStrdup ((const xmlChar *) result_name);
    }
#if DEBUG_INPUT
  fprintf (stderr, "input_open_xml: result file=%s\n", input->result);
#endif
  if (!input->variables)
    {
      input->variables =
        (char *) xmlGetProp (node, (const xmlChar *) LABEL_VARIABLES_FILE);
      if (!input->variables)
        input->variables =
          (char *) xmlStrdup ((const xmlChar *) variables_name);
    }
#if DEBUG_INPUT
  fprintf (stderr, "input_open_xml: variables file=%s\n", input->variables);
#endif

  // Opening simulator program name
  input->simulator =
    (char *) xmlGetProp (node, (const xmlChar *) LABEL_SIMULATOR);
  if (!input->simulator)
    {
      input_error (_("Bad simulator program"));
      goto exit_on_error;
    }

  // Opening evaluator program name
  input->evaluator =
    (char *) xmlGetProp (node, (const xmlChar *) LABEL_EVALUATOR);

  // Obtaining pseudo-random numbers generator seed
  input->seed
    = jb_xml_node_get_uint_with_default (node, (const xmlChar *) LABEL_SEED,
                                         &error_code, DEFAULT_RANDOM_SEED);
  if (!error_code)
    {
      input_error (_("Bad pseudo-random numbers generator seed"));
      goto exit_on_error;
    }

  // Opening algorithm
  buffer = xmlGetProp (node, (const xmlChar *) LABEL_ALGORITHM);
  if (!xmlStrcmp (buffer, (const xmlChar *) LABEL_MONTE_CARLO))
    {
      input->algorithm = ALGORITHM_MONTE_CARLO;

      // Obtaining simulations number
      input->nsimulations
        = jb_xml_node_get_uint (node, (const xmlChar *) LABEL_NSIMULATIONS,
                                &error_code);
      if (!error_code)
        {
          input_error (_("Bad simulations number"));
          goto exit_on_error;
        }
    }
  else if (!xmlStrcmp (buffer, (const xmlChar *) LABEL_SWEEP))
    input->algorithm = ALGORITHM_SWEEP;
  else if (!xmlStrcmp (buffer, (const xmlChar *) LABEL_ORTHOGONAL))
    input->algorithm = ALGORITHM_ORTHOGONAL;
  else if (!xmlStrcmp (buffer, (const xmlChar *) LABEL_GENETIC))
    {
      input->algorithm = ALGORITHM_GENETIC;

      // Obtaining population
      if (xmlHasProp (node, (const xmlChar *) LABEL_NPOPULATION))
        {
          input->nsimulations
            = jb_xml_node_get_uint (node, (const xmlChar *) LABEL_NPOPULATION,
                                    &error_code);
          if (!error_code || input->nsimulations < 3)
            {
              input_error (_("Invalid population number"));
              goto exit_on_error;
            }
        }
      else
        {
          input_error (_("No population number"));
          goto exit_on_error;
        }

      // Obtaining generations
      if (xmlHasProp (node, (const xmlChar *) LABEL_NGENERATIONS))
        {
          input->niterations
            = jb_xml_node_get_uint (node, (const xmlChar *) LABEL_NGENERATIONS,
                                    &error_code);
          if (!error_code || !input->niterations)
            {
              input_error (_("Invalid generations number"));
              goto exit_on_error;
            }
        }
      else
        {
          input_error (_("No generations number"));
          goto exit_on_error;
        }

      // Obtaining mutation probability
      if (xmlHasProp (node, (const xmlChar *) LABEL_MUTATION))
        {
          input->mutation_ratio
            = jb_xml_node_get_float (node, (const xmlChar *) LABEL_MUTATION,
                                     &error_code);
          if (!error_code || input->mutation_ratio < 0.
              || input->mutation_ratio >= 1.)
            {
              input_error (_("Invalid mutation probability"));
              goto exit_on_error;
            }
        }
      else
        {
          input_error (_("No mutation probability"));
          goto exit_on_error;
        }

      // Obtaining reproduction probability
      if (xmlHasProp (node, (const xmlChar *) LABEL_REPRODUCTION))
        {
          input->reproduction_ratio
            = jb_xml_node_get_float (node, (const xmlChar *) LABEL_REPRODUCTION,
                                     &error_code);
          if (!error_code || input->reproduction_ratio < 0.
              || input->reproduction_ratio >= 1.0)
            {
              input_error (_("Invalid reproduction probability"));
              goto exit_on_error;
            }
        }
      else
        {
          input_error (_("No reproduction probability"));
          goto exit_on_error;
        }

      // Obtaining adaptation probability
      if (xmlHasProp (node, (const xmlChar *) LABEL_ADAPTATION))
        {
          input->adaptation_ratio
            = jb_xml_node_get_float (node, (const xmlChar *) LABEL_ADAPTATION,
                                     &error_code);
          if (!error_code || input->adaptation_ratio < 0.
              || input->adaptation_ratio >= 1.)
            {
              input_error (_("Invalid adaptation probability"));
              goto exit_on_error;
            }
        }
      else
        {
          input_error (_("No adaptation probability"));
          goto exit_on_error;
        }

      // Checking survivals
      i = input->mutation_ratio * input->nsimulations;
      i += input->reproduction_ratio * input->nsimulations;
      i += input->adaptation_ratio * input->nsimulations;
      if (i > input->nsimulations - 2)
        {
          input_error
            (_("No enough survival entities to reproduce the population"));
          goto exit_on_error;
        }
    }
  else
    {
      input_error (_("Unknown algorithm"));
      goto exit_on_error;
    }
  xmlFree (buffer);
  buffer = NULL;

  if (input->algorithm == ALGORITHM_MONTE_CARLO
      || input->algorithm == ALGORITHM_SWEEP
      || input->algorithm == ALGORITHM_ORTHOGONAL)
    {

      // Obtaining iterations number
      input->niterations = jb_xml_node_get_uint_with_default
        (node, (const xmlChar *) LABEL_NITERATIONS, &error_code, 1);
      if (!error_code || !input->niterations)
        {
          input_error (_("Bad iterations number"));
          goto exit_on_error;
        }

      // Obtaining best number
      input->nbest
        = jb_xml_node_get_uint_with_default (node,
                                             (const xmlChar *) LABEL_NBEST,
                                             &error_code, 1);
      if (!error_code || !input->nbest)
        {
          input_error (_("Invalid best number"));
          goto exit_on_error;
        }

      // Obtaining tolerance
      input->tolerance
        = jb_xml_node_get_float_with_default (node,
                                              (const xmlChar *) LABEL_TOLERANCE,
                                              &error_code, 0.);
      if (!error_code || input->tolerance < 0.)
        {
          input_error (_("Invalid tolerance"));
          goto exit_on_error;
        }

      // Getting hill climbing method parameters
      if (xmlHasProp (node, (const xmlChar *) LABEL_NSTEPS))
        {
          input->nsteps =
            jb_xml_node_get_uint (node, (const xmlChar *) LABEL_NSTEPS,
                                  &error_code);
          if (!error_code)
            {
              input_error (_("Invalid steps number"));
              goto exit_on_error;
            }
#if DEBUG_INPUT
          fprintf (stderr, "input_open_xml: nsteps=%u\n", input->nsteps);
#endif
          buffer = xmlGetProp (node, (const xmlChar *) LABEL_CLIMBING);
          if (!xmlStrcmp (buffer, (const xmlChar *) LABEL_COORDINATES))
            input->climbing = CLIMBING_METHOD_COORDINATES;
          else if (!xmlStrcmp (buffer, (const xmlChar *) LABEL_RANDOM))
            {
              input->climbing = CLIMBING_METHOD_RANDOM;
              input->nestimates
                = jb_xml_node_get_uint (node,
                                        (const xmlChar *) LABEL_NESTIMATES,
                                        &error_code);
              if (!error_code || !input->nestimates)
                {
                  input_error (_("Invalid estimates number"));
                  goto exit_on_error;
                }
            }
          else
            {
              input_error (_("Unknown method to estimate the hill climbing"));
              goto exit_on_error;
            }
          xmlFree (buffer);
          buffer = NULL;
          input->relaxation
            = jb_xml_node_get_float_with_default (node,
                                                  (const xmlChar *)
                                                  LABEL_RELAXATION,
                                                  &error_code,
                                                  DEFAULT_RELAXATION);
          if (!error_code || input->relaxation < 0. || input->relaxation > 2.)
            {
              input_error (_("Invalid relaxation parameter"));
              goto exit_on_error;
            }
        }
      else
        input->nsteps = 0;
    }
  // Obtaining the threshold
  input->threshold =
    jb_xml_node_get_float_with_default (node, (const xmlChar *) LABEL_THRESHOLD,
                                        &error_code, 0.);
  if (!error_code)
    {
      input_error (_("Invalid threshold"));
      goto exit_on_error;
    }

  // Reading the experimental data
  for (child = node->children; child; child = child->next)
    {
      if (xmlStrcmp (child->name, (const xmlChar *) LABEL_EXPERIMENT))
        break;
#if DEBUG_INPUT
      fprintf (stderr, "input_open_xml: nexperiments=%u\n",
               input->nexperiments);
#endif
      input->experiment = experiment = (Experiment *)
        g_realloc (input->experiment,
                   (1 + input->nexperiments) * sizeof (Experiment));
      if (!input->nexperiments)
        {
          if (!experiment_open_xml (experiment, child, 0))
            goto exit_on_error;
        }
      else
        {
          if (!experiment_open_xml (experiment + input->nexperiments,
                                    child, experiment->ninputs))
            goto exit_on_error;
          if (experiment[experiment->ninputs].template_flags
              != experiment->template_flags)
            {
              input_error ("bad template inputs");
              goto exit_on_error;
            }
        }
      ++input->nexperiments;
#if DEBUG_INPUT
      fprintf (stderr, "input_open_xml: nexperiments=%u\n",
               input->nexperiments);
#endif
    }
  if (!input->nexperiments)
    {
      input_error (_("No optimization experiments"));
      goto exit_on_error;
    }
  input->template_flags = experiment->template_flags;
  buffer = NULL;

  // Reading the variables data
  if (input->algorithm == ALGORITHM_SWEEP
      || input->algorithm == ALGORITHM_ORTHOGONAL)
    input->nsimulations = 1;
  for (; child; child = child->next)
    {
#if DEBUG_INPUT
      fprintf (stderr, "input_open_xml: nvariables=%u\n", input->nvariables);
#endif
      if (xmlStrcmp (child->name, (const xmlChar *) LABEL_VARIABLE))
        {
          snprintf (buffer2, 64, "%s %u: %s",
                    _("Variable"), input->nvariables + 1, _("bad XML node"));
          input_error (buffer2);
          goto exit_on_error;
        }
      input->variable = (Variable *)
        g_realloc (input->variable,
                   (1 + input->nvariables) * sizeof (Variable));
      if (!variable_open_xml (input->variable + input->nvariables, child,
                              input->algorithm, input->nsteps))
        goto exit_on_error;
      if (input->algorithm == ALGORITHM_SWEEP
          || input->algorithm == ALGORITHM_ORTHOGONAL)
        input->nsimulations *= input->variable[input->nvariables].nsweeps;
      ++input->nvariables;
    }
  if (!input->nvariables)
    {
      input_error (_("No optimization variables"));
      goto exit_on_error;
    }
  if (input->nbest > input->nsimulations)
    {
      input_error (_("Best number higher than simulations number"));
      goto exit_on_error;
    }
  buffer = NULL;

  // Obtaining the error norm
  if (xmlHasProp (node, (const xmlChar *) LABEL_NORM))
    {
      buffer = xmlGetProp (node, (const xmlChar *) LABEL_NORM);
      if (!xmlStrcmp (buffer, (const xmlChar *) LABEL_EUCLIDIAN))
        input->norm = ERROR_NORM_EUCLIDIAN;
      else if (!xmlStrcmp (buffer, (const xmlChar *) LABEL_MAXIMUM))
        input->norm = ERROR_NORM_MAXIMUM;
      else if (!xmlStrcmp (buffer, (const xmlChar *) LABEL_P))
        {
          input->norm = ERROR_NORM_P;
          input->p
            = jb_xml_node_get_float (node, (const xmlChar *) LABEL_P,
                                     &error_code);
          if (!error_code)
            {
              input_error (_("Bad P parameter"));
              goto exit_on_error;
            }
        }
      else if (!xmlStrcmp (buffer, (const xmlChar *) LABEL_TAXICAB))
        input->norm = ERROR_NORM_TAXICAB;
      else
        {
          input_error (_("Unknown error norm"));
          goto exit_on_error;
        }
      xmlFree (buffer);
    }
  else
    input->norm = ERROR_NORM_EUCLIDIAN;

  // Closing the XML document
  xmlFreeDoc (doc);

#if DEBUG_INPUT
  fprintf (stderr, "input_open_xml: end\n");
#endif
  return 1;

exit_on_error:
  xmlFree (buffer);
  xmlFreeDoc (doc);
#if DEBUG_INPUT
  fprintf (stderr, "input_open_xml: end\n");
#endif
  return 0;
}

/**
 * Function to open the input file in JSON format.
 *
 * \return 1_on_success, 0_on_error.
 */
static inline int
input_open_json (JsonParser * parser)   ///< JsonParser struct.
{
  Experiment *experiment;
  JsonNode *node, *child;
  JsonObject *object;
  JsonArray *array;
  const char *buffer;
  int error_code;
  unsigned int i, n;

#if DEBUG_INPUT
  fprintf (stderr, "input_open_json: start\n");
#endif

  // Resetting input data
  input->type = INPUT_TYPE_JSON;

  // Getting the root node
#if DEBUG_INPUT
  fprintf (stderr, "input_open_json: getting the root node\n");
#endif
  node = json_parser_get_root (parser);
  object = json_node_get_object (node);

  // Getting result and variables file names
  if (!input->result)
    {
      buffer = json_object_get_string_member (object, LABEL_RESULT_FILE);
      if (!buffer)
        buffer = result_name;
      input->result = g_strdup (buffer);
    }
  else
    input->result = g_strdup (result_name);
  if (!input->variables)
    {
      buffer = json_object_get_string_member (object, LABEL_VARIABLES_FILE);
      if (!buffer)
        buffer = variables_name;
      input->variables = g_strdup (buffer);
    }
  else
    input->variables = g_strdup (variables_name);

  // Opening simulator program name
  buffer = json_object_get_string_member (object, LABEL_SIMULATOR);
  if (!buffer)
    {
      input_error (_("Bad simulator program"));
      goto exit_on_error;
    }
  input->simulator = g_strdup (buffer);

  // Opening evaluator program name
  buffer = json_object_get_string_member (object, LABEL_EVALUATOR);
  if (buffer)
    input->evaluator = g_strdup (buffer);

  // Obtaining pseudo-random numbers generator seed
  input->seed
    = jb_json_object_get_uint_with_default (object, LABEL_SEED,
                                            &error_code, DEFAULT_RANDOM_SEED);
  if (!error_code)
    {
      input_error (_("Bad pseudo-random numbers generator seed"));
      goto exit_on_error;
    }

  // Opening algorithm
  buffer = json_object_get_string_member (object, LABEL_ALGORITHM);
  if (!strcmp (buffer, LABEL_MONTE_CARLO))
    {
      input->algorithm = ALGORITHM_MONTE_CARLO;

      // Obtaining simulations number
      input->nsimulations
        = jb_json_object_get_uint (object, LABEL_NSIMULATIONS, &error_code);
      if (!error_code)
        {
          input_error (_("Bad simulations number"));
          goto exit_on_error;
        }
    }
  else if (!strcmp (buffer, LABEL_SWEEP))
    input->algorithm = ALGORITHM_SWEEP;
  else if (!strcmp (buffer, LABEL_ORTHOGONAL))
    input->algorithm = ALGORITHM_ORTHOGONAL;
  else if (!strcmp (buffer, LABEL_GENETIC))
    {
      input->algorithm = ALGORITHM_GENETIC;

      // Obtaining population
      if (json_object_get_member (object, LABEL_NPOPULATION))
        {
          input->nsimulations
            = jb_json_object_get_uint (object, LABEL_NPOPULATION, &error_code);
          if (!error_code || input->nsimulations < 3)
            {
              input_error (_("Invalid population number"));
              goto exit_on_error;
            }
        }
      else
        {
          input_error (_("No population number"));
          goto exit_on_error;
        }

      // Obtaining generations
      if (json_object_get_member (object, LABEL_NGENERATIONS))
        {
          input->niterations
            = jb_json_object_get_uint_with_default (object, LABEL_NGENERATIONS,
                                                    &error_code, 1);
          if (!error_code || !input->niterations)
            {
              input_error (_("Invalid generations number"));
              goto exit_on_error;
            }
        }
      else
        {
          input_error (_("No generations number"));
          goto exit_on_error;
        }

      // Obtaining mutation probability
      if (json_object_get_member (object, LABEL_MUTATION))
        {
          input->mutation_ratio
            = jb_json_object_get_float (object, LABEL_MUTATION, &error_code);
          if (!error_code || input->mutation_ratio < 0.
              || input->mutation_ratio >= 1.)
            {
              input_error (_("Invalid mutation probability"));
              goto exit_on_error;
            }
        }
      else
        {
          input_error (_("No mutation probability"));
          goto exit_on_error;
        }

      // Obtaining reproduction probability
      if (json_object_get_member (object, LABEL_REPRODUCTION))
        {
          input->reproduction_ratio
            = jb_json_object_get_float (object, LABEL_REPRODUCTION,
                                        &error_code);
          if (!error_code || input->reproduction_ratio < 0.
              || input->reproduction_ratio >= 1.0)
            {
              input_error (_("Invalid reproduction probability"));
              goto exit_on_error;
            }
        }
      else
        {
          input_error (_("No reproduction probability"));
          goto exit_on_error;
        }

      // Obtaining adaptation probability
      if (json_object_get_member (object, LABEL_ADAPTATION))
        {
          input->adaptation_ratio
            = jb_json_object_get_float (object, LABEL_ADAPTATION, &error_code);
          if (!error_code || input->adaptation_ratio < 0.
              || input->adaptation_ratio >= 1.)
            {
              input_error (_("Invalid adaptation probability"));
              goto exit_on_error;
            }
        }
      else
        {
          input_error (_("No adaptation probability"));
          goto exit_on_error;
        }

      // Checking survivals
      i = input->mutation_ratio * input->nsimulations;
      i += input->reproduction_ratio * input->nsimulations;
      i += input->adaptation_ratio * input->nsimulations;
      if (i > input->nsimulations - 2)
        {
          input_error
            (_("No enough survival entities to reproduce the population"));
          goto exit_on_error;
        }
    }
  else
    {
      input_error (_("Unknown algorithm"));
      goto exit_on_error;
    }

  if (input->algorithm == ALGORITHM_MONTE_CARLO
      || input->algorithm == ALGORITHM_SWEEP
      || input->algorithm == ALGORITHM_ORTHOGONAL)
    {

      // Obtaining iterations number
      input->niterations
        = jb_json_object_get_uint (object, LABEL_NITERATIONS, &error_code);
      if (!error_code || !input->niterations)
        {
          input_error (_("Bad iterations number"));
          goto exit_on_error;
        }

      // Obtaining best number
      input->nbest
        = jb_json_object_get_uint_with_default (object, LABEL_NBEST,
                                                &error_code, 1);
      if (!error_code || !input->nbest)
        {
          input_error (_("Invalid best number"));
          goto exit_on_error;
        }

      // Obtaining tolerance
      input->tolerance
        = jb_json_object_get_float_with_default (object, LABEL_TOLERANCE,
                                                 &error_code, 0.);
      if (!error_code || input->tolerance < 0.)
        {
          input_error (_("Invalid tolerance"));
          goto exit_on_error;
        }

      // Getting hill climbing method parameters
      if (json_object_get_member (object, LABEL_NSTEPS))
        {
          input->nsteps
            = jb_json_object_get_uint (object, LABEL_NSTEPS, &error_code);
          if (!error_code)
            {
              input_error (_("Invalid steps number"));
              goto exit_on_error;
            }
          buffer = json_object_get_string_member (object, LABEL_CLIMBING);
          if (!strcmp (buffer, LABEL_COORDINATES))
            input->climbing = CLIMBING_METHOD_COORDINATES;
          else if (!strcmp (buffer, LABEL_RANDOM))
            {
              input->climbing = CLIMBING_METHOD_RANDOM;
              input->nestimates
                = jb_json_object_get_uint (object, LABEL_NESTIMATES,
                                           &error_code);
              if (!error_code || !input->nestimates)
                {
                  input_error (_("Invalid estimates number"));
                  goto exit_on_error;
                }
            }
          else
            {
              input_error (_("Unknown method to estimate the hill climbing"));
              goto exit_on_error;
            }
          input->relaxation
            = jb_json_object_get_float_with_default (object, LABEL_RELAXATION,
                                                     &error_code,
                                                     DEFAULT_RELAXATION);
          if (!error_code || input->relaxation < 0. || input->relaxation > 2.)
            {
              input_error (_("Invalid relaxation parameter"));
              goto exit_on_error;
            }
        }
      else
        input->nsteps = 0;
    }
  // Obtaining the threshold
  input->threshold
    = jb_json_object_get_float_with_default (object, LABEL_THRESHOLD,
                                             &error_code, 0.);

  if (!error_code)
    {
      input_error (_("Invalid threshold"));
      goto exit_on_error;
    }

  // Reading the experimental data
  array = json_object_get_array_member (object, LABEL_EXPERIMENTS);
  n = json_array_get_length (array);
  input->experiment = experiment = (Experiment *)
    g_malloc (n * sizeof (Experiment));
  for (i = 0; i < n; ++i)
    {
#if DEBUG_INPUT
      fprintf (stderr, "input_open_json: nexperiments=%u\n",
               input->nexperiments);
#endif
      child = json_array_get_element (array, i);
      if (!input->nexperiments)
        {
          if (!experiment_open_json (experiment, child, 0))
            goto exit_on_error;
        }
      else
        {
          if (!experiment_open_json (experiment + input->nexperiments,
                                     child, experiment->ninputs))
            goto exit_on_error;
          if (experiment[experiment->ninputs].template_flags
              != experiment->template_flags)
            {
              input_error ("bad template inputs");
              goto exit_on_error;
            }
        }
      ++input->nexperiments;
#if DEBUG_INPUT
      fprintf (stderr, "input_open_json: nexperiments=%u\n",
               input->nexperiments);
#endif
    }
  if (!input->nexperiments)
    {
      input_error (_("No optimization experiments"));
      goto exit_on_error;
    }
  input->template_flags = experiment->template_flags;

  // Reading the variables data
  array = json_object_get_array_member (object, LABEL_VARIABLES);
  n = json_array_get_length (array);
  input->variable = (Variable *) g_malloc (n * sizeof (Variable));
  for (i = 0; i < n; ++i)
    {
#if DEBUG_INPUT
      fprintf (stderr, "input_open_json: nvariables=%u\n", input->nvariables);
#endif
      child = json_array_get_element (array, i);
      if (!variable_open_json (input->variable + input->nvariables, child,
                               input->algorithm, input->nsteps))
        goto exit_on_error;
      ++input->nvariables;
    }
  if (!input->nvariables)
    {
      input_error (_("No optimization variables"));
      goto exit_on_error;
    }

  // Obtaining the error norm
  if (json_object_get_member (object, LABEL_NORM))
    {
      buffer = json_object_get_string_member (object, LABEL_NORM);
      if (!strcmp (buffer, LABEL_EUCLIDIAN))
        input->norm = ERROR_NORM_EUCLIDIAN;
      else if (!strcmp (buffer, LABEL_MAXIMUM))
        input->norm = ERROR_NORM_MAXIMUM;
      else if (!strcmp (buffer, LABEL_P))
        {
          input->norm = ERROR_NORM_P;
          input->p = jb_json_object_get_float (object, LABEL_P, &error_code);
          if (!error_code)
            {
              input_error (_("Bad P parameter"));
              goto exit_on_error;
            }
        }
      else if (!strcmp (buffer, LABEL_TAXICAB))
        input->norm = ERROR_NORM_TAXICAB;
      else
        {
          input_error (_("Unknown error norm"));
          goto exit_on_error;
        }
    }
  else
    input->norm = ERROR_NORM_EUCLIDIAN;

  // Closing the JSON document
  g_object_unref (parser);

#if DEBUG_INPUT
  fprintf (stderr, "input_open_json: end\n");
#endif
  return 1;

exit_on_error:
  g_object_unref (parser);
#if DEBUG_INPUT
  fprintf (stderr, "input_open_json: end\n");
#endif
  return 0;
}

/**
 * Function to open the input file.
 *
 * \return 1_on_success, 0_on_error.
 */
int
input_open (char *filename)     ///< Input data file name.
{
  xmlDoc *doc;
  JsonParser *parser;

#if DEBUG_INPUT
  fprintf (stderr, "input_open: start\n");
#endif

  // Resetting input data
  input_new ();

  // Opening input file
#if DEBUG_INPUT
  fprintf (stderr, "input_open: opening the input file %s\n", filename);
  fprintf (stderr, "input_open: trying XML format\n");
#endif
  doc = xmlParseFile (filename);
  if (!doc)
    {
#if DEBUG_INPUT
      fprintf (stderr, "input_open: trying JSON format\n");
#endif
      parser = json_parser_new ();
      if (!json_parser_load_from_file (parser, filename, NULL))
        {
          input_error (_("Unable to parse the input file"));
          goto exit_on_error;
        }
      if (!input_open_json (parser))
        goto exit_on_error;
    }
  else if (!input_open_xml (doc))
    goto exit_on_error;

  // Getting the working directory
  input->directory = g_path_get_dirname (filename);
  input->name = g_path_get_basename (filename);

#if DEBUG_INPUT
  fprintf (stderr, "input_open: end\n");
#endif
  return 1;

exit_on_error:
  jbw_show_error (error_message);
  g_free (error_message);
  input_free ();
#if DEBUG_INPUT
  fprintf (stderr, "input_open: end\n");
#endif
  return 0;
}
