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
 * \file variable.c
 * \brief Source file to define the variable data.
 * \authors Javier Burguete and Borja Latorre.
 * \copyright Copyright 2012-2018, all rights reserved.
 */
#define _GNU_SOURCE
#include "config.h"
#include <stdio.h>
#include <libxml/parser.h>
#include <libintl.h>
#include <glib.h>
#include <json-glib/json-glib.h>
#include "utils.h"
#include "variable.h"

#define DEBUG_VARIABLE 0        ///< Macro to debug variable functions.

const char *format[NPRECISIONS] = {
  "%.0lf", "%.1lf", "%.2lf", "%.3lf", "%.4lf", "%.5lf", "%.6lf", "%.7lf",
  "%.8lf", "%.9lf", "%.10lf", "%.11lf", "%.12lf", "%.13lf", "%.14lf"
};                              ///< Array of C-strings with variable formats.

const double precision[NPRECISIONS] = {
  1., 0.1, 0.01, 1e-3, 1e-4, 1e-5, 1e-6, 1e-7, 1e-8, 1e-9, 1e-10, 1e-11,
  1e-12, 1e-13, 1e-14
};                              ///< Array of variable precisions.

/**
 * Function to create a new Variable struct.
 */
void
variable_new (Variable * variable)      ///< Variable struct.
{
#if DEBUG_VARIABLE
  fprintf (stderr, "variable_new: start\n");
#endif
  variable->name = NULL;
#if DEBUG_VARIABLE
  fprintf (stderr, "variable_new: end\n");
#endif
}

/**
 * Function to free the memory of a Variable struct.
 */
void
variable_free (Variable * variable,
///< Variable struct.
               unsigned int type)
///< Type of input file.
{
#if DEBUG_VARIABLE
  fprintf (stderr, "variable_free: start\n");
#endif
  if (type == INPUT_TYPE_XML)
    xmlFree (variable->name);
  else
    g_free (variable->name);
#if DEBUG_VARIABLE
  fprintf (stderr, "variable_free: end\n");
#endif
}

/**
 * Function to print a message error opening an Variable struct.
 */
void
variable_error (Variable * variable,
///< Variable struct.
                char *message)
///< Error message.
{
  char buffer[64];
  if (!variable->name)
    snprintf (buffer, 64, "%s: %s", _("Variable"), message);
  else
    snprintf (buffer, 64, "%s %s: %s", _("Variable"), variable->name, message);
  error_message = g_strdup (buffer);
}

/**
 * Function to open the variable file.
 *
 * \return 1 on success, 0 on error.
 */
int
variable_open_xml (Variable * variable, ///< Variable struct.
                   xmlNode * node,      ///< XML node.
                   unsigned int algorithm,      ///< Algorithm type.
                   unsigned int nsteps)
                   ///< Number of steps to do the hill climbing method.
{
  int error_code;

#if DEBUG_VARIABLE
  fprintf (stderr, "variable_open_xml: start\n");
#endif

  variable->name = (char *) xmlGetProp (node, (const xmlChar *) LABEL_NAME);
  if (!variable->name)
    {
      variable_error (variable, _("no name"));
      goto exit_on_error;
    }
  if (xmlHasProp (node, (const xmlChar *) LABEL_MINIMUM))
    {
      variable->rangemin
        = xml_node_get_float (node, (const xmlChar *) LABEL_MINIMUM,
                              &error_code);
      if (error_code)
        {
          variable_error (variable, _("bad minimum"));
          goto exit_on_error;
        }
      variable->rangeminabs = xml_node_get_float_with_default
        (node, (const xmlChar *) LABEL_ABSOLUTE_MINIMUM, -G_MAXDOUBLE,
         &error_code);
      if (error_code)
        {
          variable_error (variable, _("bad absolute minimum"));
          goto exit_on_error;
        }
      if (variable->rangemin < variable->rangeminabs)
        {
          variable_error (variable, _("minimum range not allowed"));
          goto exit_on_error;
        }
    }
  else
    {
      variable_error (variable, _("no minimum range"));
      goto exit_on_error;
    }
  if (xmlHasProp (node, (const xmlChar *) LABEL_MAXIMUM))
    {
      variable->rangemax
        = xml_node_get_float (node, (const xmlChar *) LABEL_MAXIMUM,
                              &error_code);
      if (error_code)
        {
          variable_error (variable, _("bad maximum"));
          goto exit_on_error;
        }
      variable->rangemaxabs = xml_node_get_float_with_default
        (node, (const xmlChar *) LABEL_ABSOLUTE_MAXIMUM, G_MAXDOUBLE,
         &error_code);
      if (error_code)
        {
          variable_error (variable, _("bad absolute maximum"));
          goto exit_on_error;
        }
      if (variable->rangemax > variable->rangemaxabs)
        {
          variable_error (variable, _("maximum range not allowed"));
          goto exit_on_error;
        }
      if (variable->rangemax < variable->rangemin)
        {
          variable_error (variable, _("bad range"));
          goto exit_on_error;
        }
    }
  else
    {
      variable_error (variable, _("no maximum range"));
      goto exit_on_error;
    }
  variable->precision
    = xml_node_get_uint_with_default (node, (const xmlChar *) LABEL_PRECISION,
                                      DEFAULT_PRECISION, &error_code);
  if (error_code || variable->precision >= NPRECISIONS)
    {
      variable_error (variable, _("bad precision"));
      goto exit_on_error;
    }
  if (algorithm == ALGORITHM_SWEEP || algorithm == ALGORITHM_ORTHOGONAL)
    {
      if (xmlHasProp (node, (const xmlChar *) LABEL_NSWEEPS))
        {
          variable->nsweeps
            = xml_node_get_uint (node, (const xmlChar *) LABEL_NSWEEPS,
                                 &error_code);
          if (error_code || !variable->nsweeps)
            {
              variable_error (variable, _("bad sweeps"));
              goto exit_on_error;
            }
        }
      else
        {
          variable_error (variable, _("no sweeps number"));
          goto exit_on_error;
        }
#if DEBUG_VARIABLE
      fprintf (stderr, "variable_open_xml: nsweeps=%u\n", variable->nsweeps);
#endif
    }
  if (algorithm == ALGORITHM_GENETIC)
    {
      // Obtaining bits representing each variable
      if (xmlHasProp (node, (const xmlChar *) LABEL_NBITS))
        {
          variable->nbits
            = xml_node_get_uint (node, (const xmlChar *) LABEL_NBITS,
                                 &error_code);
          if (error_code || !variable->nbits)
            {
              variable_error (variable, _("invalid bits number"));
              goto exit_on_error;
            }
        }
      else
        {
          variable_error (variable, _("no bits number"));
          goto exit_on_error;
        }
    }
  else if (nsteps)
    {
      variable->step
        = xml_node_get_float (node, (const xmlChar *) LABEL_STEP, &error_code);
      if (error_code || variable->step < 0.)
        {
          variable_error (variable, _("bad step size"));
          goto exit_on_error;
        }
    }

#if DEBUG_VARIABLE
  fprintf (stderr, "variable_open_xml: end\n");
#endif
  return 1;
exit_on_error:
  variable_free (variable, INPUT_TYPE_XML);
#if DEBUG_VARIABLE
  fprintf (stderr, "variable_open_xml: end\n");
#endif
  return 0;
}

/**
 * Function to open the variable file.
 *
 * \return 1 on success, 0 on error.
 */
int
variable_open_json (Variable * variable,        ///< Variable struct.
                    JsonNode * node,    ///< XML node.
                    unsigned int algorithm,     ///< Algorithm type.
                    unsigned int nsteps)
                    ///< Number of steps to do the hill climbing method.
{
  JsonObject *object;
  const char *label;
  int error_code;
#if DEBUG_VARIABLE
  fprintf (stderr, "variable_open_json: start\n");
#endif
  object = json_node_get_object (node);
  label = json_object_get_string_member (object, LABEL_NAME);
  if (!label)
    {
      variable_error (variable, _("no name"));
      goto exit_on_error;
    }
  variable->name = g_strdup (label);
  if (json_object_get_member (object, LABEL_MINIMUM))
    {
      variable->rangemin
        = json_object_get_float (object, LABEL_MINIMUM, &error_code);
      if (error_code)
        {
          variable_error (variable, _("bad minimum"));
          goto exit_on_error;
        }
      variable->rangeminabs
        = json_object_get_float_with_default (object, LABEL_ABSOLUTE_MINIMUM,
                                              -G_MAXDOUBLE, &error_code);
      if (error_code)
        {
          variable_error (variable, _("bad absolute minimum"));
          goto exit_on_error;
        }
      if (variable->rangemin < variable->rangeminabs)
        {
          variable_error (variable, _("minimum range not allowed"));
          goto exit_on_error;
        }
    }
  else
    {
      variable_error (variable, _("no minimum range"));
      goto exit_on_error;
    }
  if (json_object_get_member (object, LABEL_MAXIMUM))
    {
      variable->rangemax
        = json_object_get_float (object, LABEL_MAXIMUM, &error_code);
      if (error_code)
        {
          variable_error (variable, _("bad maximum"));
          goto exit_on_error;
        }
      variable->rangemaxabs
        = json_object_get_float_with_default (object, LABEL_ABSOLUTE_MAXIMUM,
                                              G_MAXDOUBLE, &error_code);
      if (error_code)
        {
          variable_error (variable, _("bad absolute maximum"));
          goto exit_on_error;
        }
      if (variable->rangemax > variable->rangemaxabs)
        {
          variable_error (variable, _("maximum range not allowed"));
          goto exit_on_error;
        }
      if (variable->rangemax < variable->rangemin)
        {
          variable_error (variable, _("bad range"));
          goto exit_on_error;
        }
    }
  else
    {
      variable_error (variable, _("no maximum range"));
      goto exit_on_error;
    }
  variable->precision
    = json_object_get_uint_with_default (object, LABEL_PRECISION,
                                         DEFAULT_PRECISION, &error_code);
  if (error_code || variable->precision >= NPRECISIONS)
    {
      variable_error (variable, _("bad precision"));
      goto exit_on_error;
    }
  if (algorithm == ALGORITHM_SWEEP || algorithm == ALGORITHM_ORTHOGONAL)
    {
      if (json_object_get_member (object, LABEL_NSWEEPS))
        {
          variable->nsweeps
            = json_object_get_uint (object, LABEL_NSWEEPS, &error_code);
          if (error_code || !variable->nsweeps)
            {
              variable_error (variable, _("bad sweeps"));
              goto exit_on_error;
            }
        }
      else
        {
          variable_error (variable, _("no sweeps number"));
          goto exit_on_error;
        }
#if DEBUG_VARIABLE
      fprintf (stderr, "variable_open_json: nsweeps=%u\n", variable->nsweeps);
#endif
    }
  if (algorithm == ALGORITHM_GENETIC)
    {
      // Obtaining bits representing each variable
      if (json_object_get_member (object, LABEL_NBITS))
        {
          variable->nbits
            = json_object_get_uint (object, LABEL_NBITS, &error_code);
          if (error_code || !variable->nbits)
            {
              variable_error (variable, _("invalid bits number"));
              goto exit_on_error;
            }
        }
      else
        {
          variable_error (variable, _("no bits number"));
          goto exit_on_error;
        }
    }
  else if (nsteps)
    {
      variable->step = json_object_get_float (object, LABEL_STEP, &error_code);
      if (error_code || variable->step < 0.)
        {
          variable_error (variable, _("bad step size"));
          goto exit_on_error;
        }
    }

#if DEBUG_VARIABLE
  fprintf (stderr, "variable_open_json: end\n");
#endif
  return 1;
exit_on_error:
  variable_free (variable, INPUT_TYPE_JSON);
#if DEBUG_VARIABLE
  fprintf (stderr, "variable_open_json: end\n");
#endif
  return 0;
}
