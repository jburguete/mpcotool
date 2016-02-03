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
 * \file experiment.c
 * \brief Source file to define the experiment data.
 * \authors Javier Burguete and Borja Latorre.
 * \copyright Copyright 2012-2016, all rights reserved.
 */
#define _GNU_SOURCE
#include "config.h"
#include <stdio.h>
#include <libxml/parser.h>
#include <libintl.h>
#include <glib.h>
#include "utils.h"
#include "experiment.h"

#define DEBUG_EXPERIMENT 0                 ///< Macro to debug.

const xmlChar *template[MAX_NINPUTS] = {
  XML_TEMPLATE1, XML_TEMPLATE2, XML_TEMPLATE3, XML_TEMPLATE4,
  XML_TEMPLATE5, XML_TEMPLATE6, XML_TEMPLATE7, XML_TEMPLATE8
};

///< Array of xmlChar strings with template labels.

/**
 * \fn void experiment_new (Experiment * experiment)
 * \brief Function to create a new Experiment struct.
 * \param experiment
 * \brief Experiment struct.
 */
void
experiment_new (Experiment * experiment)
{
  unsigned int i;
#if DEBUG_EXPERIMENT
  fprintf (stderr, "experiment_new: start\n");
#endif
  experiment->name = NULL;
  experiment->ninputs = 0;
  for (i = 0; i < MAX_NINPUTS; ++i)
    experiment->template[i] = NULL;
#if DEBUG_EXPERIMENT
  fprintf (stderr, "input_new: end\n");
#endif
}

/**
 * \fn void experiment_free (Experiment * experiment)
 * \brief Function to free the memory of an Experiment struct.
 * \param experiment
 * \brief Experiment struct.
 */
void
experiment_free (Experiment * experiment)
{
  unsigned int i;
#if DEBUG_EXPERIMENT
  fprintf (stderr, "experiment_free: start\n");
#endif
  for (i = 0; i < experiment->ninputs; ++i)
    xmlFree (experiment->template[i]);
  xmlFree (experiment->name);
  experiment->ninputs = 0;
#if DEBUG_EXPERIMENT
  fprintf (stderr, "experiment_free: end\n");
#endif
}

/**
 * \fn void experiment_error (Experiment * experiment, char *message)
 * \brief Function to print a message error opening an Experiment struct.
 * \param experiment
 * \brief Experiment struct.
 * \param message
 * \brief Error message.
 */
void
experiment_error (Experiment * experiment, char *message)
{
  char buffer[64];
  if (!experiment->name)
    snprintf (buffer, 64, "%s: %s", gettext ("Experiment"), message);
  else
    snprintf (buffer, 64, "%s %s: %s", gettext ("Experiment"), experiment->name,
              message);
  error_message = g_strdup (buffer);
}

/**
 * \fn int experiment_open (Experiment * experiment, xmlNode * node, \
 *   unsigned int ninputs)
 * \brief Function to open the Experiment struct on a XML node.
 * \param experiment
 * \brief Experiment struct.
 * \param node
 * \brief XML node.
 * \param ninputs
 * \brief Number of the simulator input files.
 * \return 1 on success, 0 on error.
 */
int
experiment_open (Experiment * experiment, xmlNode * node, unsigned int ninputs)
{
  char buffer[64];
  int error_code;
  unsigned int i;

#if DEBUG_EXPERIMENT
  fprintf (stderr, "experiment_open: start\n");
#endif

  // Resetting experiment data
  experiment_new (experiment);

  // Reading the experimental data
  experiment->name = (char *) xmlGetProp (node, XML_NAME);
  if (!experiment->name)
    {
      experiment_error (experiment, gettext ("no data file name"));
      goto exit_on_error;
    }
#if DEBUG_EXPERIMENT
  fprintf (stderr, "experiment_open: name=%s\n", experiment->name);
#endif
  experiment->weight
    = xml_node_get_float_with_default (node, XML_WEIGHT, 1., &error_code);
  if (error_code)
    {
      experiment_error (experiment, gettext ("bad weight"));
      goto exit_on_error;
    }
#if DEBUG_EXPERIMENT
  fprintf (stderr, "experiment_open: weight=%lg\n", experiment->weight);
#endif
  experiment->template[0] = (char *) xmlGetProp (node, template[0]);
  if (experiment->template[0])
    {
#if DEBUG_EXPERIMENT
      fprintf (stderr, "experiment_open: experiment=%s template1=%s\n",
               experiment->name, buffer2[0]);
#endif
      ++experiment->ninputs;
    }
  else
    {
      experiment_error (experiment, gettext ("no template"));
      goto exit_on_error;
    }
  for (i = 1; i < MAX_NINPUTS; ++i)
    {
#if DEBUG_EXPERIMENT
      fprintf (stderr, "experiment_open: template%u\n", i + 1);
#endif
      if (xmlHasProp (node, template[i]))
        {
          if (ninputs && ninputs <= i)
            {
              experiment_error (experiment, gettext ("bad templates number"));
              goto exit_on_error;
            }
          experiment->template[i] = (char *) xmlGetProp (node, template[i]);
#if DEBUG_EXPERIMENT
          fprintf (stderr, "experiment_open: experiment=%s template%u=%s\n",
                   experiment->nexperiments, experiment->name,
                   experiment->template[i]);
#endif
          ++experiment->ninputs;
        }
      else if (ninputs && ninputs > i)
        {
          snprintf (buffer, 64, "%s%u", gettext ("no template"), i + 1);
          experiment_error (experiment, buffer);
          goto exit_on_error;
        }
      else
        break;
    }

#if DEBUG_EXPERIMENT
  fprintf (stderr, "experiment_open: end\n");
#endif
  return 1;

exit_on_error:
  experiment_free (experiment);
#if DEBUG_EXPERIMENT
  fprintf (stderr, "experiment_open: end\n");
#endif
  return 0;
}
