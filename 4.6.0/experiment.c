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
 * \file experiment.c
 * \brief Source file to define the experiment data.
 * \authors Javier Burguete and Borja Latorre.
 * \copyright Copyright 2012-2023, all rights reserved.
 */
#define _GNU_SOURCE
#include "config.h"
#include <stdio.h>
#include <libxml/parser.h>
#include <libintl.h>
#include <glib.h>
#include <json-glib/json-glib.h>
#include "jb/src/jb_xml.h"
#include "jb/src/jb_json.h"
#include "jb/src/jb_win.h"
#include "tools.h"
#include "experiment.h"

#define DEBUG_EXPERIMENT 0      ///< Macro to debug experiment functions.

///> Array of strings with stencil labels.
const char *stencil[MAX_NINPUTS] = {
  LABEL_TEMPLATE1, LABEL_TEMPLATE2, LABEL_TEMPLATE3, LABEL_TEMPLATE4,
  LABEL_TEMPLATE5, LABEL_TEMPLATE6, LABEL_TEMPLATE7, LABEL_TEMPLATE8
};

///> Array of strings with binary stencil labels.
const char *stencilbin[MAX_NINPUTS] = {
  LABEL_INPUT1, LABEL_INPUT2, LABEL_INPUT3, LABEL_INPUT4,
  LABEL_INPUT5, LABEL_INPUT6, LABEL_INPUT7, LABEL_INPUT8
};

/**
 * Function to create a new Experiment struct.
 */
static void
experiment_new (Experiment * experiment)        ///< Experiment struct.
{
  unsigned int i;
#if DEBUG_EXPERIMENT
  fprintf (stderr, "experiment_new: start\n");
#endif
  experiment->name = NULL;
  experiment->ninputs = experiment->template_flags = 0;
  for (i = 0; i < MAX_NINPUTS; ++i)
    experiment->stencil[i] = NULL;
#if DEBUG_EXPERIMENT
  fprintf (stderr, "input_new: end\n");
#endif
}

/**
 * Function to free the memory of an Experiment struct.
 */
void
experiment_free (Experiment * experiment,       ///< Experiment struct.
                 unsigned int type)     ///< Type of input file.
{
  unsigned int i;
#if DEBUG_EXPERIMENT
  fprintf (stderr, "experiment_free: start\n");
#endif
  if (type == INPUT_TYPE_XML)
    {
      for (i = 0; i < experiment->ninputs; ++i)
        xmlFree (experiment->stencil[i]);
      xmlFree (experiment->name);
    }
  else
    {
      for (i = 0; i < experiment->ninputs; ++i)
        g_free (experiment->stencil[i]);
      g_free (experiment->name);
    }
  experiment->ninputs = experiment->template_flags = 0;
#if DEBUG_EXPERIMENT
  fprintf (stderr, "experiment_free: end\n");
#endif
}

/**
 * Function to print a message error opening an Experiment struct.
 */
void
experiment_error (Experiment * experiment,      ///< Experiment struct.
                  char *message)        ///< Error message.
{
  if (!experiment->name)
    error_message = g_strconcat (_("Experiment"), ": ", message, NULL);
  else
    error_message = g_strconcat (_("Experiment"), " ", experiment->name, ": ",
                                 message, NULL);
}

/**
 * Function to open the Experiment struct on a XML node.
 *
 * \return 1 on success, 0 on error.
 */
int
experiment_open_xml (Experiment * experiment,   ///< Experiment struct.
                     xmlNode * node,    ///< XML node.
                     unsigned int ninputs)
                     ///< Number of the simulator input files.
{
  char buffer[64];
  int error_code;
  unsigned int i;
  unsigned int flags = 1;

#if DEBUG_EXPERIMENT
  fprintf (stderr, "experiment_open_xml: start\n");
#endif

  // Resetting experiment data
  experiment_new (experiment);

  // Reading the experimental data
  experiment->name = (char *) xmlGetProp (node, (const xmlChar *) LABEL_NAME);
  if (!experiment->name)
    {
      experiment_error (experiment, _("no data file name"));
      goto exit_on_error;
    }
#if DEBUG_EXPERIMENT
  fprintf (stderr, "experiment_open_xml: name=%s\n", experiment->name);
#endif
  experiment->weight
    = jb_xml_node_get_float_with_default (node, (const xmlChar *) LABEL_WEIGHT,
                                          &error_code, 1.);
  if (!error_code)
    {
      experiment_error (experiment, _("bad weight"));
      goto exit_on_error;
    }
#if DEBUG_EXPERIMENT
  fprintf (stderr, "experiment_open_xml: weight=%lg\n", experiment->weight);
#endif
  experiment->stencil[0]
    = (char *) xmlGetProp (node, (const xmlChar *) stencil[0]);
  if (experiment->stencil[0])
    {
#if DEBUG_EXPERIMENT
      fprintf (stderr, "experiment_open_xml: experiment=%s stencil1=%s\n",
               experiment->name, stencil[0]);
#endif
      ++experiment->ninputs;
      experiment->template_flags |= flags;
    }
  else
    {
      experiment->stencil[0]
        = (char *) xmlGetProp (node, (const xmlChar *) stencilbin[0]);
      if (experiment->stencil[0])
        {
#if DEBUG_EXPERIMENT
          fprintf (stderr, "experiment_open_xml: experiment=%s stencil1=%s\n",
                   experiment->name, stencilbin[0]);
#endif
          ++experiment->ninputs;
        }
      else
        {
          experiment_error (experiment, _("no template"));
          goto exit_on_error;
        }
    }
  for (i = 1; i < MAX_NINPUTS; ++i)
    {
#if DEBUG_EXPERIMENT
      fprintf (stderr, "experiment_open_xml: stencil%u\n", i + 1);
#endif
      flags <<= 1;
      if (xmlHasProp (node, (const xmlChar *) stencil[i]))
        {
          if (ninputs && ninputs <= i)
            {
              experiment_error (experiment, _("bad templates number"));
              goto exit_on_error;
            }
          experiment->stencil[i]
            = (char *) xmlGetProp (node, (const xmlChar *) stencil[i]);
#if DEBUG_EXPERIMENT
          fprintf (stderr,
                   "experiment_open_xml: experiment=%s stencil%u=%s\n",
                   experiment->nexperiments, experiment->name,
                   experiment->stencil[i]);
#endif
          ++experiment->ninputs;
          experiment->template_flags |= flags;
        }
      else if (xmlHasProp (node, (const xmlChar *) stencilbin[i]))
        {
          if (ninputs && ninputs <= i)
            {
              experiment_error (experiment, _("bad templates number"));
              goto exit_on_error;
            }
          experiment->stencil[i]
            = (char *) xmlGetProp (node, (const xmlChar *) stencilbin[i]);
#if DEBUG_EXPERIMENT
          fprintf (stderr,
                   "experiment_open_xml: experiment=%s stencil%u=%s\n",
                   experiment->nexperiments, experiment->name,
                   experiment->stencil[i]);
#endif
          ++experiment->ninputs;
        }
      else if (ninputs && ninputs > i)
        {
          snprintf (buffer, 64, "%s%u", _("no template"), i + 1);
          experiment_error (experiment, buffer);
          goto exit_on_error;
        }
      else
        break;
    }

#if DEBUG_EXPERIMENT
  fprintf (stderr, "experiment_open_xml: end\n");
#endif
  return 1;

exit_on_error:
  experiment_free (experiment, INPUT_TYPE_XML);
#if DEBUG_EXPERIMENT
  fprintf (stderr, "experiment_open_xml: end\n");
#endif
  return 0;
}

/**
 * Function to open the Experiment struct on a XML node.
 *
 * \return 1 on success, 0 on error.
 */
int
experiment_open_json (Experiment * experiment,  ///< Experiment struct.
                      JsonNode * node,  ///< JSON node.
                      unsigned int ninputs)
                      ///< Number of the simulator input files.
{
  char buffer[64];
  JsonObject *object;
  const char *name;
  int error_code;
  unsigned int i;
  unsigned int flags = 1;

#if DEBUG_EXPERIMENT
  fprintf (stderr, "experiment_open_json: start\n");
#endif

  // Resetting experiment data
  experiment_new (experiment);

  // Getting JSON object
  object = json_node_get_object (node);

  // Reading the experimental data
  name = json_object_get_string_member (object, LABEL_NAME);
  if (!name)
    {
      experiment_error (experiment, _("no data file name"));
      goto exit_on_error;
    }
  experiment->name = g_strdup (name);
#if DEBUG_EXPERIMENT
  fprintf (stderr, "experiment_open_json: name=%s\n", experiment->name);
#endif
  experiment->weight
    = jb_json_object_get_float_with_default (object, LABEL_WEIGHT, &error_code,
                                             1.);
  if (!error_code)
    {
      experiment_error (experiment, _("bad weight"));
      goto exit_on_error;
    }
#if DEBUG_EXPERIMENT
  fprintf (stderr, "experiment_open_json: weight=%lg\n", experiment->weight);
#endif
  name = json_object_get_string_member (object, stencil[0]);
  if (name)
    {
#if DEBUG_EXPERIMENT
      fprintf (stderr, "experiment_open_json: experiment=%s template1=%s\n",
               name, stencil[0]);
#endif
      ++experiment->ninputs;
      experiment->template_flags |= flags;
    }
  else
    {
      name = json_object_get_string_member (object, stencilbin[0]);
      if (name)
        {
#if DEBUG_EXPERIMENT
          fprintf (stderr, "experiment_open_json: experiment=%s template1=%s\n",
                   name, stencilbin[0]);
#endif
          ++experiment->ninputs;
        }
      else
        {
          experiment_error (experiment, _("no template"));
          goto exit_on_error;
        }
    }
  experiment->stencil[0] = g_strdup (name);
  for (i = 1; i < MAX_NINPUTS; ++i)
    {
#if DEBUG_EXPERIMENT
      fprintf (stderr, "experiment_open_json: stencil%u\n", i + 1);
#endif
      flags <<= 1;
      if (json_object_get_member (object, stencil[i]))
        {
          if (ninputs && ninputs <= i)
            {
              experiment_error (experiment, _("bad templates number"));
              goto exit_on_error;
            }
          name = json_object_get_string_member (object, stencil[i]);
#if DEBUG_EXPERIMENT
          fprintf (stderr,
                   "experiment_open_json: experiment=%s stencil%u=%s\n",
                   experiment->nexperiments, name, stencil[i]);
#endif
          experiment->stencil[i] = g_strdup (name);
          ++experiment->ninputs;
          experiment->template_flags |= flags;
        }
      else if (json_object_get_member (object, stencilbin[i]))
        {
          if (ninputs && ninputs <= i)
            {
              experiment_error (experiment, _("bad templates number"));
              goto exit_on_error;
            }
          name = json_object_get_string_member (object, stencilbin[i]);
#if DEBUG_EXPERIMENT
          fprintf (stderr,
                   "experiment_open_json: experiment=%s stencil%u=%s\n",
                   experiment->nexperiments, name, stencilbin[i]);
#endif
          experiment->stencil[i] = g_strdup (name);
          ++experiment->ninputs;
        }
      else if (ninputs && ninputs > i)
        {
          snprintf (buffer, 64, "%s%u", _("no template"), i + 1);
          experiment_error (experiment, buffer);
          goto exit_on_error;
        }
      else
        break;
    }

#if DEBUG_EXPERIMENT
  fprintf (stderr, "experiment_open_json: end\n");
#endif
  return 1;

exit_on_error:
  experiment_free (experiment, INPUT_TYPE_JSON);
#if DEBUG_EXPERIMENT
  fprintf (stderr, "experiment_open_json: end\n");
#endif
  return 0;
}
