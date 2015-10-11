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
 * \file calibrator.c
 * \brief Source file of the calibrator.
 * \authors Javier Burguete and Borja Latorre.
 * \copyright Copyright 2012-2015, all rights reserved.
 */
#define _GNU_SOURCE
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <locale.h>
#include <gsl/gsl_rng.h>
#include <libxml/parser.h>
#include <libintl.h>
#include <glib.h>
#ifdef G_OS_WIN32
#include <windows.h>
#elif (!__BSD_VISIBLE)
#include <alloca.h>
#endif
#if HAVE_MPI
#include <mpi.h>
#endif
#include "genetic/genetic.h"
#include "calibrator.h"
#if HAVE_GTK
#include <gtk/gtk.h>
#include "interface.h"
#endif

/**
 * \def DEBUG
 * \brief Macro to debug.
 */
#define DEBUG 0

/**
 * \var ntasks
 * \brief Number of tasks.
 * \var nthreads
 * \brief Number of threads.
 * \var mutex
 * \brief Mutex struct.
 * \var void (*calibrate_step)(Calibrate*)
 * \brief Pointer to the function to perform a calibration algorithm step.
 * \var input
 * \brief Input struct to define the input file to calibrator.
 * \var calibrate
 * \brief Calibration data.
 */
int ntasks;
unsigned int nthreads;
#if GLIB_MINOR_VERSION >= 32
GMutex mutex[1];
#else
GMutex *mutex;
#endif
void (*calibrate_step) (Calibrate *);
Input input[1];
Calibrate calibrate[1];

#if HAVE_GTK
/**
 * \var window
 * \brief Window struct to define the main interface window.
 */
Window window[1];
#endif

/**
 * \fn void show_error(char *msg)
 * \brief Function to show a dialog with an error message.
 * \param msg
 * \brief Error message.
 */
void
show_error (char *msg)
{
#if HAVE_GTK
  GtkMessageDialog *dlg;

  // Creating the dialog
  dlg = (GtkMessageDialog *) gtk_message_dialog_new
    (window->window, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
     "%s", msg);

  // Setting the dialog title
  gtk_window_set_title (GTK_WINDOW (dlg), gettext ("ERROR!"));

  // Showing the dialog and waiting response
  gtk_dialog_run (GTK_DIALOG (dlg));

  // Closing and freeing memory
  gtk_widget_destroy (GTK_WIDGET (dlg));

#else
  printf ("%s: %s\n", gettext ("ERROR!"), msg);
#endif
}

/**
 * \fn int xml_node_get_int(xmlNode *node, const xmlChar *prop, int *error_code)
 * \brief Function to get an integer number of a XML node property.
 * \param node
 * \brief XML node.
 * \param prop
 * \brief XML property.
 * \param error_code
 * \brief Error code.
 * \return Integer number value.
 */
int
xml_node_get_int (xmlNode * node, const xmlChar * prop, int *error_code)
{
  int i = 0;
  xmlChar *buffer;
  buffer = xmlGetProp (node, prop);
  if (!buffer)
    *error_code = 1;
  else
    {
      if (sscanf ((char *) buffer, "%d", &i) != 1)
        *error_code = 2;
      else
        *error_code = 0;
      xmlFree (buffer);
    }
  return i;
}

/**
 * \fn int xml_node_get_uint(xmlNode *node, const xmlChar *prop, \
 *   int *error_code)
 * \brief Function to get an unsigned integer number of a XML node property.
 * \param node
 * \brief XML node.
 * \param prop
 * \brief XML property.
 * \param error_code
 * \brief Error code.
 * \return Unsigned integer number value.
 */
unsigned int
xml_node_get_uint (xmlNode * node, const xmlChar * prop, int *error_code)
{
  unsigned int i = 0;
  xmlChar *buffer;
  buffer = xmlGetProp (node, prop);
  if (!buffer)
    *error_code = 1;
  else
    {
      if (sscanf ((char *) buffer, "%u", &i) != 1)
        *error_code = 2;
      else
        *error_code = 0;
      xmlFree (buffer);
    }
  return i;
}

/**
 * \fn double xml_node_get_float(xmlNode *node, const xmlChar *prop, \
 *   int *error_code)
 * \brief Function to get a floating point number of a XML node property.
 * \param node
 * \brief XML node.
 * \param prop
 * \brief XML property.
 * \param error_code
 * \brief Error code.
 * \return Floating point number value.
 */
double
xml_node_get_float (xmlNode * node, const xmlChar * prop, int *error_code)
{
  double x = 0.;
  xmlChar *buffer;
  buffer = xmlGetProp (node, prop);
  if (!buffer)
    *error_code = 1;
  else
    {
      if (sscanf ((char *) buffer, "%lf", &x) != 1)
        *error_code = 2;
      else
        *error_code = 0;
      xmlFree (buffer);
    }
  return x;
}

/**
 * \fn int input_open(char *filename)
 * \brief Function to open the input file.
 * \param filename
 * \brief Input data file name.
 * \return 1 on success, 0 on error.
 */
int
input_open (char *filename)
{
  int error_code;
  unsigned int i;
  xmlChar *buffer;
  xmlDoc *doc;
  xmlNode *node, *child;

#if DEBUG
  fprintf (stderr, "input_open: start\n");
#endif

  // Parsing the input file
  doc = xmlParseFile (filename);
  if (!doc)
    {
      show_error (gettext ("Unable to parse the input file"));
      return 0;
    }

  // Getting the root node
  node = xmlDocGetRootElement (doc);
  if (xmlStrcmp (node->name, XML_CALIBRATE))
    {
      show_error (gettext ("Bad root XML node"));
      return 0;
    }

  // Opening simulator program name
  input->simulator = (char *) xmlGetProp (node, XML_SIMULATOR);
  if (!input->simulator)
    {
      show_error (gettext ("Bad simulator program"));
      return 0;
    }

  // Opening evaluator program name
  input->evaluator = (char *) xmlGetProp (node, XML_EVALUATOR);

  // Opening algorithm
  buffer = xmlGetProp (node, XML_ALGORITHM);
  if (!xmlStrcmp (buffer, XML_MONTE_CARLO))
    {
      input->algorithm = ALGORITHM_MONTE_CARLO;

      // Obtaining simulations number
      input->nsimulations =
        xml_node_get_int (node, XML_NSIMULATIONS, &error_code);
      if (error_code)
        {
          show_error (gettext ("Bad simulations number"));
          return 0;
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
          input->nsimulations =
            xml_node_get_uint (node, XML_NPOPULATION, &error_code);
          if (error_code || input->nsimulations < 3)
            {
              show_error (gettext ("Invalid population number"));
              return 0;
            }
        }
      else
        {
          show_error (gettext ("No population number"));
          return 0;
        }

      // Obtaining generations
      if (xmlHasProp (node, XML_NGENERATIONS))
        {
          input->niterations =
            xml_node_get_uint (node, XML_NGENERATIONS, &error_code);
          if (error_code || !input->niterations)
            {
              show_error (gettext ("Invalid generation number"));
              return 0;
            }
        }
      else
        {
          show_error (gettext ("No generation number"));
          return 0;
        }

      // Obtaining mutation probability
      if (xmlHasProp (node, XML_MUTATION))
        {
          input->mutation_ratio =
            xml_node_get_float (node, XML_MUTATION, &error_code);
          if (error_code || input->mutation_ratio < 0.
              || input->mutation_ratio >= 1.)
            {
              show_error (gettext ("Invalid mutation probability"));
              return 0;
            }
        }
      else
        {
          show_error (gettext ("No mutation probability"));
          return 0;
        }

      // Obtaining reproduction probability
      if (xmlHasProp (node, XML_REPRODUCTION))
        {
          input->reproduction_ratio =
            xml_node_get_float (node, XML_REPRODUCTION, &error_code);
          if (error_code || input->reproduction_ratio < 0.
              || input->reproduction_ratio >= 1.0)
            {
              show_error (gettext ("Invalid reproduction probability"));
              return 0;
            }
        }
      else
        {
          show_error (gettext ("No reproduction probability"));
          return 0;
        }

      // Obtaining adaptation probability
      if (xmlHasProp (node, XML_ADAPTATION))
        {
          input->adaptation_ratio =
            xml_node_get_float (node, XML_ADAPTATION, &error_code);
          if (error_code || input->adaptation_ratio < 0.
              || input->adaptation_ratio >= 1.)
            {
              show_error (gettext ("Invalid adaptation probability"));
              return 0;
            }
        }
      else
        {
          show_error (gettext ("No adaptation probability"));
          return 0;
        }

      // Checking survivals
      i = input->mutation_ratio * input->nsimulations;
      i += input->reproduction_ratio * input->nsimulations;
      i += input->adaptation_ratio * input->nsimulations;
      if (i > input->nsimulations - 2)
        {
          show_error (gettext
                      ("No enough survival entities to reproduce the population"));
          return 0;
        }
    }
  else
    {
      show_error (gettext ("Unknown algorithm"));
      return 0;
    }

  if (input->algorithm == ALGORITHM_MONTE_CARLO
      || input->algorithm == ALGORITHM_SWEEP)
    {

      // Obtaining iterations number
      input->niterations =
        xml_node_get_int (node, XML_NITERATIONS, &error_code);
      if (error_code == 1)
        input->niterations = 1;
      else if (error_code)
        {
          show_error (gettext ("Bad iterations number"));
          return 0;
        }

      // Obtaining best number
      if (xmlHasProp (node, XML_NBEST))
        {
          input->nbest = xml_node_get_uint (node, XML_NBEST, &error_code);
          if (error_code || !input->nbest)
            {
              show_error (gettext ("Invalid best number"));
              return 0;
            }
        }
      else
        input->nbest = 1;

      // Obtaining tolerance
      if (xmlHasProp (node, XML_TOLERANCE))
        {
          input->tolerance =
            xml_node_get_float (node, XML_TOLERANCE, &error_code);
          if (error_code || input->tolerance < 0.)
            {
              show_error (gettext ("Invalid tolerance"));
              return 0;
            }
        }
      else
        input->tolerance = 0.;
    }

  // Reading the variables data
  input->nvariables = 0;
  input->label = NULL;
  input->rangemin = NULL;
  input->rangemax = NULL;
  input->rangeminabs = NULL;
  input->rangemaxabs = NULL;
  input->format = NULL;
  input->nsweeps = NULL;
  input->nbits = NULL;
  for (; child; child = child->next)
    {
      if (xmlStrcmp (child->name, XML_VARIABLE))
        {
          show_error (gettext ("Bad XML node"));
          return 0;
        }
      if (xmlHasProp (child, XML_NAME))
        {
          input->label = g_realloc
            (input->label, (1 + input->nvariables) * sizeof (char *));
          input->label[input->nvariables] =
            (char *) xmlGetProp (child, XML_NAME);
        }
      else
        {
          show_error (gettext ("No variable name"));
          return 0;
        }
      if (xmlHasProp (child, XML_MINIMUM))
        {
          input->rangemin = g_realloc
            (input->rangemin, (1 + input->nvariables) * sizeof (double));
          input->rangeminabs = g_realloc
            (input->rangeminabs, (1 + input->nvariables) * sizeof (double));
          input->rangemin[input->nvariables] =
            xml_node_get_float (child, XML_MINIMUM, &error_code);
          if (xmlHasProp (child, XML_ABSOLUTE_MINIMUM))
            {
              input->rangeminabs[input->nvariables] =
                xml_node_get_float (child, XML_ABSOLUTE_MINIMUM, &error_code);
            }
          else
            input->rangeminabs[input->nvariables] = -INFINITY;
        }
      else
        {
          show_error (gettext ("No variable minimum range"));
          return 0;
        }
      if (xmlHasProp (child, XML_MAXIMUM))
        {
          input->rangemax = g_realloc
            (input->rangemax, (1 + input->nvariables) * sizeof (double));
          input->rangemaxabs = g_realloc
            (input->rangemaxabs, (1 + input->nvariables) * sizeof (double));
          input->rangemax[input->nvariables] =
            xml_node_get_float (child, XML_MAXIMUM, &error_code);
          if (xmlHasProp (child, XML_ABSOLUTE_MAXIMUM))
            input->rangemaxabs[input->nvariables]
              = xml_node_get_float (child, XML_ABSOLUTE_MINIMUM, &error_code);
          else
            input->rangemaxabs[input->nvariables] = INFINITY;
        }
      else
        {
          show_error (gettext ("No variable maximum range"));
          return 0;
        }
      input->format = g_realloc (input->format,
                                 (1 + input->nvariables) * sizeof (char *));
      if (xmlHasProp (child, XML_FORMAT))
        {
          input->format[input->nvariables] =
            (char *) xmlGetProp (child, XML_FORMAT);
        }
      else
        {
          input->format[input->nvariables] =
            (char *) xmlStrdup (DEFAULT_FORMAT);
        }
      if (input->algorithm == ALGORITHM_SWEEP)
        {
          if (xmlHasProp (child, XML_NSWEEPS))
            {
              input->nsweeps =
                g_realloc (input->nsweeps,
                           (1 + input->nvariables) * sizeof (unsigned int));
              input->nsweeps[input->nvariables] =
                xml_node_get_uint (child, XML_NSWEEPS, &error_code);
            }
          else
            {
              show_error (gettext ("No variable sweeps number"));
              return 0;
            }
#if DEBUG
          fprintf (stderr, "input_new: nsweeps=%u nsimulations=%u\n",
                   input->nsweeps[input->nvariables], input->nsimulations);
#endif
        }
      if (input->algorithm == ALGORITHM_GENETIC)
        {
          // Obtaining bits representing each variable
          if (xmlHasProp (child, XML_NBITS))
            {
              input->nbits =
                g_realloc (input->nbits,
                           (1 + input->nvariables) * sizeof (unsigned int));
              i = xml_node_get_uint (child, XML_NBITS, &error_code);
              if (error_code || !i)
                {
                  show_error (gettext ("Invalid bit number"));
                  return 0;
                }
              input->nbits[input->nvariables] = i;
            }
          else
            {
              printf ("No variable %u bits number\n", input->nvariables + 1);
              return 0;
            }
        }
      ++input->nvariables;
    }
  if (!input->nvariables)
    {
      show_error (gettext ("No calibration variables"));
      return 0;
    }
#if DEBUG
  fprintf (stderr, "input_open: end\n");
#endif

  return 1;
}

/**
 * \fn void calibrate_input(Calibrate *calibrate, unsigned int simulation, \
 *   char *input, GMappedFile *template)
 * \brief Function to write the simulation input file.
 * \param calibrate
 * \brief Calibration data.
 * \param simulation
 * \brief Simulation number.
 * \param input
 * \brief Input file name.
 * \param template
 * \brief Template of the input file name.
 */
void
calibrate_input (Calibrate * calibrate, unsigned int simulation,
                 char *input, GMappedFile * template)
{
  unsigned int i;
  char buffer[32], value[32], *buffer2, *buffer3, *content;
  FILE *file;
  gsize length;
  GRegex *regex;

#if DEBUG
  fprintf (stderr, "calibrate_input: start\n");
#endif

  // Checking the file
  if (!template)
    goto calibrate_input_end;

  // Opening template
  content = g_mapped_file_get_contents (template);
  length = g_mapped_file_get_length (template);
#if DEBUG
  fprintf (stderr, "calibrate_input: length=%lu\ncontent:\n%s", length,
           content);
#endif
  file = fopen (input, "w");

  // Parsing template
  for (i = 0; i < calibrate->nvariables; ++i)
    {
#if DEBUG
      fprintf (stderr, "calibrate_input: variable=%u\n", i);
#endif
      snprintf (buffer, 32, "@variable%u@", i + 1);
      regex = g_regex_new (buffer, 0, 0, NULL);
      if (i == 0)
        {
          buffer2 = g_regex_replace_literal (regex, content, length, 0,
                                             calibrate->label[i], 0, NULL);
#if DEBUG
          fprintf (stderr, "calibrate_input: buffer2\n%s", buffer2);
#endif
        }
      else
        {
          length = strlen (buffer3);
          buffer2 = g_regex_replace_literal (regex, buffer3, length, 0,
                                             calibrate->label[i], 0, NULL);
          g_free (buffer3);
        }
      g_regex_unref (regex);
      length = strlen (buffer2);
      snprintf (buffer, 32, "@value%u@", i + 1);
      regex = g_regex_new (buffer, 0, 0, NULL);
      snprintf (value, 32, calibrate->format[i],
                calibrate->value[simulation * calibrate->nvariables + i]);

#if DEBUG
      fprintf (stderr, "calibrate_input: value=%s\n", value);
#endif
      buffer3 = g_regex_replace_literal (regex, buffer2, length, 0, value,
                                         0, NULL);
      g_free (buffer2);
      g_regex_unref (regex);
    }

  // Saving input file
  fwrite (buffer3, strlen (buffer3), sizeof (char), file);
  g_free (buffer3);
  fclose (file);

calibrate_input_end:
#if DEBUG
  fprintf (stderr, "calibrate_input: end\n");
#endif
  return;
}

/**
 * \fn double calibrate_parse(Calibrate *calibrate, unsigned int simulation, \
 *   unsigned int experiment)
 * \brief Function to parse input files, simulating and calculating the \
 *   objective function.
 * \param calibrate
 * \brief Calibration data.
 * \param simulation
 * \brief Simulation number.
 * \param experiment
 * \brief Experiment number.
 * \return Objective function value.
 */
double
calibrate_parse (Calibrate * calibrate, unsigned int simulation,
                 unsigned int experiment)
{
  unsigned int i;
  double e;
  char buffer[512], input[MAX_NINPUTS][32], output[32], result[32];
  FILE *file_result;

#if DEBUG
  fprintf (stderr, "calibrate_parse: start\n");
  fprintf (stderr, "calibrate_parse: simulation=%u experiment=%u\n", simulation,
           experiment);
#endif

  // Opening input files
  for (i = 0; i < calibrate->ninputs; ++i)
    {
      snprintf (&input[i][0], 32, "input-%u-%u-%u", i, simulation, experiment);
#if DEBUG
      fprintf (stderr, "calibrate_parse: i=%u input=%s\n", i, &input[i][0]);
#endif
      calibrate_input (calibrate, simulation, &input[i][0],
                       calibrate->file[i][experiment]);
    }
  for (; i < MAX_NINPUTS; ++i)
    strcpy (&input[i][0], "");
#if DEBUG
  fprintf (stderr, "calibrate_parse: parsing end\n");
#endif

  // Performing the simulation
  snprintf (output, 32, "output-%u-%u", simulation, experiment);
  snprintf (buffer, 512, "./%s %s %s %s %s %s %s %s %s %s",
            calibrate->simulator,
            &input[0][0], &input[1][0], &input[2][0], &input[3][0],
            &input[4][0], &input[5][0], &input[6][0], &input[7][0], output);
#if DEBUG
  fprintf (stderr, "calibrate_parse: %s\n", buffer);
#endif
  system (buffer);

  // Checking the objective value function
  if (calibrate->evaluator)
    {
      snprintf (result, 32, "result-%u-%u", simulation, experiment);
      snprintf (buffer, 512, "./%s %s %s %s", calibrate->evaluator, output,
                calibrate->experiment[experiment], result);
#if DEBUG
      fprintf (stderr, "calibrate_parse: %s\n", buffer);
#endif
      system (buffer);
      file_result = fopen (result, "r");
      e = atof (fgets (buffer, 512, file_result));
      fclose (file_result);
    }
  else
    {
      strcpy (result, "");
      file_result = fopen (output, "r");
      e = atof (fgets (buffer, 512, file_result));
      fclose (file_result);
    }

  // Removing files
#if !DEBUG
  for (i = 0; i < calibrate->ninputs; ++i)
    {
      if (calibrate->file[i][0])
        {
          snprintf (buffer, 512, "rm %s", &input[i][0]);
          system (buffer);
        }
    }
  snprintf (buffer, 512, "rm %s %s", output, result);
  system (buffer);
#endif

#if DEBUG
  fprintf (stderr, "calibrate_parse: end\n");
#endif

  // Returning the objective function
  return e * calibrate->weight[experiment];
}

/**
 * \fn void calibrate_best_thread(Calibrate *calibrate, \
 *   unsigned int simulation, double value)
 * \brief Function to save the best simulations of a thread.
 * \param calibrate
 * \brief Calibration data.
 * \param simulation
 * \brief Simulation number.
 * \param value
 * \brief Objective function value.
 */
void
calibrate_best_thread (Calibrate * calibrate, unsigned int simulation,
                       double value)
{
  unsigned int i, j;
  double e;
#if DEBUG
  fprintf (stderr, "calibrate_best_thread: start\n");
#endif
  if (calibrate->nsaveds < calibrate->nbest
      || value < calibrate->error_best[calibrate->nsaveds - 1])
    {
      g_mutex_lock (mutex);
      if (calibrate->nsaveds < calibrate->nbest)
        ++calibrate->nsaveds;
      calibrate->error_best[calibrate->nsaveds - 1] = value;
      calibrate->simulation_best[calibrate->nsaveds - 1] = simulation;
      for (i = calibrate->nsaveds; --i;)
        {
          if (calibrate->error_best[i] < calibrate->error_best[i - 1])
            {
              j = calibrate->simulation_best[i];
              e = calibrate->error_best[i];
              calibrate->simulation_best[i] = calibrate->simulation_best[i - 1];
              calibrate->error_best[i] = calibrate->error_best[i - 1];
              calibrate->simulation_best[i - 1] = j;
              calibrate->error_best[i - 1] = e;
            }
          else
            break;
        }
      g_mutex_unlock (mutex);
    }
#if DEBUG
  fprintf (stderr, "calibrate_best_thread: end\n");
#endif
}

/**
 * \fn void calibrate_best_sequential(Calibrate *calibrate, \
 *   unsigned int simulation, double value)
 * \brief Function to save the best simulations.
 * \param calibrate
 * \brief Calibration data.
 * \param simulation
 * \brief Simulation number.
 * \param value
 * \brief Objective function value.
 */
void
calibrate_best_sequential (Calibrate * calibrate, unsigned int simulation,
                           double value)
{
  unsigned int i, j;
  double e;
#if DEBUG
  fprintf (stderr, "calibrate_best_sequential: start\n");
#endif
  if (calibrate->nsaveds < calibrate->nbest
      || value < calibrate->error_best[calibrate->nsaveds - 1])
    {
      if (calibrate->nsaveds < calibrate->nbest)
        ++calibrate->nsaveds;
      calibrate->error_best[calibrate->nsaveds - 1] = value;
      calibrate->simulation_best[calibrate->nsaveds - 1] = simulation;
      for (i = calibrate->nsaveds; --i;)
        {
          if (calibrate->error_best[i] < calibrate->error_best[i - 1])
            {
              j = calibrate->simulation_best[i];
              e = calibrate->error_best[i];
              calibrate->simulation_best[i] = calibrate->simulation_best[i - 1];
              calibrate->error_best[i] = calibrate->error_best[i - 1];
              calibrate->simulation_best[i - 1] = j;
              calibrate->error_best[i - 1] = e;
            }
          else
            break;
        }
    }
#if DEBUG
  fprintf (stderr, "calibrate_best_sequential: end\n");
#endif
}

/**
 * \fn void* calibrate_thread(ParallelData *data)
 * \brief Function to calibrate on a thread.
 * \param data
 * \brief Function data.
 * \return NULL
 */
void *
calibrate_thread (ParallelData * data)
{
  unsigned int i, j, thread;
  double e;
  Calibrate *calibrate;
#if DEBUG
  fprintf (stderr, "calibrate_thread: start\n");
#endif
  thread = data->thread;
  calibrate = data->calibrate;
#if DEBUG
  fprintf (stderr, "calibrate_thread: thread=%u start=%u end=%u\n", thread,
           calibrate->thread[thread], calibrate->thread[thread + 1]);
#endif
  for (i = calibrate->thread[thread]; i < calibrate->thread[thread + 1]; ++i)
    {
      e = 0.;
      for (j = 0; j < calibrate->nexperiments; ++j)
        e += calibrate_parse (calibrate, i, j);
      calibrate_best_thread (calibrate, i, e);
#if DEBUG
      fprintf (stderr, "calibrate_thread: i=%u e=%lg\n", i, e);
#endif
    }
#if DEBUG
  fprintf (stderr, "calibrate_thread: end\n");
#endif
  g_thread_exit (NULL);
  return NULL;
}

/**
 * \fn void calibrate_sequential(Calibrate *calibrate)
 * \brief Function to calibrate sequentially.
 * \param calibrate
 * \brief Calibration data pointer.
 */
void
calibrate_sequential (Calibrate * calibrate)
{
  unsigned int i, j;
  double e;
#if DEBUG
  fprintf (stderr, "calibrate_sequential: start\n");
  fprintf (stderr, "calibrate_sequential: nstart=%u nend=%u\n",
           calibrate->nstart, calibrate->nend);
#endif
  for (i = calibrate->nstart; i < calibrate->nend; ++i)
    {
      e = 0.;
      for (j = 0; j < calibrate->nexperiments; ++j)
        e += calibrate_parse (calibrate, i, j);
      calibrate_best_sequential (calibrate, i, e);
#if DEBUG
      fprintf (stderr, "calibrate_sequential: i=%u e=%lg\n", i, e);
#endif
    }
#if DEBUG
  fprintf (stderr, "calibrate_sequential: end\n");
#endif
}

/**
 * \fn void calibrate_merge(Calibrate *calibrate, unsigned int nsaveds, \
 *   unsigned int *simulation_best, double *error_best)
 * \brief Function to merge the 2 calibration results.
 * \param calibrate
 * \brief Calibration data.
 * \param nsaveds
 * \brief Number of saved results.
 * \param simulation_best
 * \brief Array of best simulation numbers.
 * \param error_best
 * \brief Array of best objective function values.
 */
void
calibrate_merge (Calibrate * calibrate, unsigned int nsaveds,
                 unsigned int *simulation_best, double *error_best)
{
  unsigned int i, j, k, s[calibrate->nbest];
  double e[calibrate->nbest];
#if DEBUG
  fprintf (stderr, "calibrate_merge: start\n");
#endif
  i = j = k = 0;
  do
    {
      if (i == calibrate->nsaveds)
        {
          s[k] = simulation_best[j];
          e[k] = error_best[j];
          ++j;
          ++k;
          if (j == nsaveds)
            break;
        }
      else if (j == nsaveds)
        {
          s[k] = calibrate->simulation_best[i];
          e[k] = calibrate->error_best[i];
          ++i;
          ++k;
          if (i == calibrate->nsaveds)
            break;
        }
      else if (calibrate->error_best[i] > error_best[j])
        {
          s[k] = simulation_best[j];
          e[k] = error_best[j];
          ++j;
          ++k;
        }
      else
        {
          s[k] = calibrate->simulation_best[i];
          e[k] = calibrate->error_best[i];
          ++i;
          ++k;
        }
    }
  while (k < calibrate->nbest);
  calibrate->nsaveds = k;
  memcpy (calibrate->simulation_best, s, k * sizeof (unsigned int));
  memcpy (calibrate->error_best, e, k * sizeof (double));
#if DEBUG
  fprintf (stderr, "calibrate_merge: end\n");
#endif
}

/**
 * \fn void calibrate_synchronise(Calibrate *calibrate)
 * \brief Function to synchronise the calibration results of MPI tasks.
 * \param calibrate
 * \brief Calibration data.
 */
#if HAVE_MPI
void
calibrate_synchronise (Calibrate * calibrate)
{
  unsigned int i, nsaveds, simulation_best[calibrate->nbest];
  double error_best[calibrate->nbest];
  MPI_Status mpi_stat;
#if DEBUG
  fprintf (stderr, "calibrate_synchronise: start\n");
#endif
  if (calibrate->mpi_rank == 0)
    {
      for (i = 1; i < ntasks; ++i)
        {
          MPI_Recv (&nsaveds, 1, MPI_INT, i, 1, MPI_COMM_WORLD, &mpi_stat);
          MPI_Recv (simulation_best, nsaveds, MPI_INT, i, 1,
                    MPI_COMM_WORLD, &mpi_stat);
          MPI_Recv (error_best, nsaveds, MPI_DOUBLE, i, 1,
                    MPI_COMM_WORLD, &mpi_stat);
          calibrate_merge (calibrate, nsaveds, simulation_best, error_best);
        }
    }
  else
    {
      MPI_Send (&calibrate->nsaveds, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
      MPI_Send (calibrate->simulation_best, calibrate->nsaveds, MPI_INT, 0, 1,
                MPI_COMM_WORLD);
      MPI_Send (calibrate->error_best, calibrate->nsaveds, MPI_DOUBLE, 0, 1,
                MPI_COMM_WORLD);
    }
#if DEBUG
  fprintf (stderr, "calibrate_synchronise: end\n");
#endif
}
#endif

/**
 * \fn void calibrate_sweep(Calibrate *calibrate)
 * \brief Function to calibrate with the sweep algorithm.
 * \param calibrate
 * \brief Calibration data pointer.
 */
void
calibrate_sweep (Calibrate * calibrate)
{
  unsigned int i, j, k, l;
  double e;
  GThread *thread[nthreads];
  ParallelData data[nthreads];
#if DEBUG
  fprintf (stderr, "calibrate_sweep: start\n");
#endif
  for (i = 0; i < calibrate->nsimulations; ++i)
    {
      k = i;
      for (j = 0; j < calibrate->nvariables; ++j)
        {
          l = k % calibrate->nsweeps[j];
          k /= calibrate->nsweeps[j];
          e = calibrate->rangemin[j];
          if (calibrate->nsweeps[j] > 1)
            e += l * (calibrate->rangemax[j] - calibrate->rangemin[j])
              / (calibrate->nsweeps[j] - 1);
          calibrate->value[i * calibrate->nvariables + j] = e;
        }
    }
  calibrate->nsaveds = 0;
  if (nthreads <= 1)
    calibrate_sequential (calibrate);
  else
    {
      for (i = 0; i < nthreads; ++i)
        {
          data[i].calibrate = calibrate;
          data[i].thread = i;
#if GLIB_MINOR_VERSION >= 32
          thread[i] =
            g_thread_new (NULL, (void (*)) calibrate_thread, &data[i]);
#else
          thread[i] =
            g_thread_create ((void (*)) calibrate_thread, &data[i], TRUE, NULL);
#endif
        }
      for (i = 0; i < nthreads; ++i)
        g_thread_join (thread[i]);
    }
#if HAVE_MPI
  // Communicating tasks results
  calibrate_synchronise (calibrate);
#endif
#if DEBUG
  fprintf (stderr, "calibrate_sweep: end\n");
#endif
}

/**
 * \fn void calibrate_MonteCarlo(Calibrate *calibrate)
 * \brief Function to calibrate with the Monte-Carlo algorithm.
 * \param calibrate
 * \brief Calibration data pointer.
 */
void
calibrate_MonteCarlo (Calibrate * calibrate)
{
  unsigned int i, j;
  GThread *thread[nthreads];
  ParallelData data[nthreads];
#if DEBUG
  fprintf (stderr, "calibrate_MonteCarlo: start\n");
#endif
  for (i = 0; i < calibrate->nsimulations; ++i)
    for (j = 0; j < calibrate->nvariables; ++j)
      calibrate->value[i * calibrate->nvariables + j] =
        calibrate->rangemin[j] + gsl_rng_uniform (calibrate->rng)
        * (calibrate->rangemax[j] - calibrate->rangemin[j]);
  calibrate->nsaveds = 0;
  if (nthreads <= 1)
    calibrate_sequential (calibrate);
  else
    {
      for (i = 0; i < nthreads; ++i)
        {
          data[i].calibrate = calibrate;
          data[i].thread = i;
#if GLIB_MINOR_VERSION >= 32
          thread[i] =
            g_thread_new (NULL, (void (*)) calibrate_thread, &data[i]);
#else
          thread[i] =
            g_thread_create ((void (*)) calibrate_thread, &data[i], TRUE, NULL);
#endif
        }
      for (i = 0; i < nthreads; ++i)
        g_thread_join (thread[i]);
    }
#if HAVE_MPI
  // Communicating tasks results
  calibrate_synchronise (calibrate);
#endif
#if DEBUG
  fprintf (stderr, "calibrate_MonteCarlo: end\n");
#endif
}

/**
 * \fn double calibrate_genetic_objective(Entity *entity)
 * \brief Function to calculate the objective function of an entity.
 * \param entity
 * \brief entity data.
 * \return objective function value.
 */
double
calibrate_genetic_objective (Entity * entity)
{
  unsigned int j;
  double objective;
#if DEBUG
  fprintf (stderr, "calibrate_genetic_objective: start\n");
#endif
  for (j = 0; j < calibrate->nvariables; ++j)
    calibrate->value[entity->id * calibrate->nvariables + j]
      = genetic_get_variable (entity, calibrate->genetic_variable + j);
  for (j = 0, objective = 0.; j < calibrate->nexperiments; ++j)
    objective += calibrate_parse (calibrate, entity->id, j);
#if DEBUG
  fprintf (stderr, "calibrate_genetic_objective: end\n");
#endif
  return objective;
}

/**
 * \fn void calibrate_genetic(Calibrate *calibrate)
 * \brief Function to calibrate with the genetic algorithm.
 * \param calibrate
 * \brief Calibration data pointer.
 */
void
calibrate_genetic (Calibrate * calibrate)
{
  unsigned int i;
  char buffer[512], *best_genome;
  double best_objective, *best_variable;
#if DEBUG
  fprintf (stderr, "calibrate_genetic: start\n");
  fprintf (stderr, "calibrate_genetic: ntasks=%u nthreads=%u\n", ntasks,
           nthreads);
  fprintf (stderr,
           "calibrate_genetic: nvariables=%u population=%u generations=%u\n",
           calibrate->nvariables, calibrate->nsimulations,
           calibrate->niterations);
  fprintf (stderr,
           "calibrate_genetic: mutation=%lg reproduction=%lg adaptation=%lg\n",
           calibrate->mutation_ratio, calibrate->reproduction_ratio,
           calibrate->adaptation_ratio);
#endif
  genetic_algorithm_default (calibrate->nvariables,
                             calibrate->genetic_variable,
                             calibrate->nsimulations,
                             calibrate->niterations,
                             calibrate->mutation_ratio,
                             calibrate->reproduction_ratio,
                             calibrate->adaptation_ratio,
                             &calibrate_genetic_objective,
                             &best_genome, &best_variable, &best_objective);
#if DEBUG
  fprintf (stderr, "calibrate_genetic: the best\n");
#endif
  printf ("THE BEST IS\n");
  fprintf (calibrate->result, "THE BEST IS\n");
  printf ("error=%le\n", best_objective);
  fprintf (calibrate->result, "error=%le\n", best_objective);
  for (i = 0; i < calibrate->nvariables; ++i)
    {
      snprintf (buffer, 512, "%s=%s\n",
                calibrate->label[i], calibrate->format[i]);
      printf (buffer, best_variable[i]);
      fprintf (calibrate->result, buffer, best_variable[i]);
    }
  fflush (calibrate->result);
  g_free (best_genome);
  g_free (best_variable);
#if DEBUG
  fprintf (stderr, "calibrate_genetic: start\n");
#endif
}

/**
 * \fn void calibrate_print(Calibrate *calibrate)
 * \brief Function to print the results.
 * \param calibrate
 * \brief Calibration data.
 */
void
calibrate_print (Calibrate * calibrate)
{
  unsigned int i;
  char buffer[512];
#if HAVE_MPI
  if (!calibrate->mpi_rank)
    {
#endif
      printf ("THE BEST IS\n");
      fprintf (calibrate->result, "THE BEST IS\n");
      printf ("error=%le\n", calibrate->error_old[0]);
      fprintf (calibrate->result, "error=%le\n", calibrate->error_old[0]);
      for (i = 0; i < calibrate->nvariables; ++i)
        {
          snprintf (buffer, 512, "%s=%s\n",
                    calibrate->label[i], calibrate->format[i]);
          printf (buffer, calibrate->value_old[i]);
          fprintf (calibrate->result, buffer, calibrate->value_old[i]);
        }
      fflush (calibrate->result);
#if HAVE_MPI
    }
#endif
}

/**
 * \fn void calibrate_save_old(Calibrate *calibrate)
 * \brief Function to save the best results on iterative methods.
 * \param calibrate
 * \brief Calibration data.
 */
void
calibrate_save_old (Calibrate * calibrate)
{
  unsigned int i, j;
#if DEBUG
  fprintf (stderr, "calibrate_save_old: start\n");
#endif
  memcpy (calibrate->error_old, calibrate->error_best,
          calibrate->nbest * sizeof (double));
  for (i = 0; i < calibrate->nbest; ++i)
    {
      j = calibrate->simulation_best[i];
      memcpy (calibrate->value_old + i * calibrate->nvariables,
              calibrate->value + j * calibrate->nvariables,
              calibrate->nvariables * sizeof (double));
    }
#if DEBUG
  for (i = 0; i < calibrate->nvariables; ++i)
    fprintf (stderr, "calibrate_save_old: best variable %u=%lg\n",
             i, calibrate->value_old[i]);
  fprintf (stderr, "calibrate_save_old: end\n");
#endif
}

/**
 * \fn void calibrate_merge_old(Calibrate *calibrate)
 * \brief Function to merge the best results with the previous step best results
 *   on iterative methods.
 * \param calibrate
 * \brief Calibration data.
 */
void
calibrate_merge_old (Calibrate * calibrate)
{
  unsigned int i, j, k;
  double v[calibrate->nbest * calibrate->nvariables], e[calibrate->nbest],
    *enew, *eold;
#if DEBUG
  fprintf (stderr, "calibrate_merge_old: start\n");
#endif
  enew = calibrate->error_best;
  eold = calibrate->error_old;
  i = j = k = 0;
  do
    {
      if (*enew < *eold)
        {
          memcpy (v + k * calibrate->nvariables,
                  calibrate->value
                  + calibrate->simulation_best[i] * calibrate->nvariables,
                  calibrate->nvariables * sizeof (double));
          e[k] = *enew;
          ++k;
          ++enew;
          ++i;
        }
      else
        {
          memcpy (v + k * calibrate->nvariables,
                  calibrate->value_old + j * calibrate->nvariables,
                  calibrate->nvariables * sizeof (double));
          e[k] = *eold;
          ++k;
          ++eold;
          ++j;
        }
    }
  while (k < calibrate->nbest);
  memcpy (calibrate->value_old, v, k * calibrate->nvariables * sizeof (double));
  memcpy (calibrate->error_old, e, k * sizeof (double));
#if DEBUG
  fprintf (stderr, "calibrate_merge_old: end\n");
#endif
}

/**
 * \fn void calibrate_refine(Calibrate *calibrate)
 * \brief Function to refine the search ranges of the variables in iterative
 *   algorithms.
 * \param calibrate
 * \brief Calibration data.
 */
void
calibrate_refine (Calibrate * calibrate)
{
  unsigned int i, j;
  double d;
#if HAVE_MPI
  MPI_Status mpi_stat;
#endif
#if DEBUG
  fprintf (stderr, "calibrate_refine: start\n");
#endif
#if HAVE_MPI
  if (!calibrate->mpi_rank)
    {
#endif
      for (j = 0; j < calibrate->nvariables; ++j)
        {
          calibrate->rangemin[j] = calibrate->rangemax[j]
            = calibrate->value_old[j];
        }
      for (i = 0; ++i < calibrate->nbest;)
        {
          for (j = 0; j < calibrate->nvariables; ++j)
            {
              calibrate->rangemin[j] = fmin (calibrate->rangemin[j],
                                             calibrate->value_old[i *
                                                                  calibrate->nvariables
                                                                  + j]);
              calibrate->rangemax[j] =
                fmax (calibrate->rangemax[j],
                      calibrate->value_old[i * calibrate->nvariables + j]);
            }
        }
      for (j = 0; j < calibrate->nvariables; ++j)
        {
          d = 0.5 * calibrate->tolerance
            * (calibrate->rangemax[j] - calibrate->rangemin[j]);
          calibrate->rangemin[j] -= d;
          calibrate->rangemin[j]
            = fmax (calibrate->rangemin[j], calibrate->rangeminabs[j]);
          calibrate->rangemax[j] += d;
          calibrate->rangemax[j]
            = fmin (calibrate->rangemax[j], calibrate->rangemaxabs[j]);
          printf ("%s min=%lg max=%lg\n", calibrate->label[j],
                  calibrate->rangemin[j], calibrate->rangemax[j]);
          fprintf (calibrate->result, "%s min=%lg max=%lg\n",
                   calibrate->label[j], calibrate->rangemin[j],
                   calibrate->rangemax[j]);
        }
#if HAVE_MPI
      for (i = 1; i < ntasks; ++i)
        {
          MPI_Send (calibrate->rangemin, calibrate->nvariables, MPI_DOUBLE, i,
                    1, MPI_COMM_WORLD);
          MPI_Send (calibrate->rangemax, calibrate->nvariables, MPI_DOUBLE, i,
                    1, MPI_COMM_WORLD);
        }
    }
  else
    {
      MPI_Recv (calibrate->rangemin, calibrate->nvariables, MPI_DOUBLE, 0, 1,
                MPI_COMM_WORLD, &mpi_stat);
      MPI_Recv (calibrate->rangemax, calibrate->nvariables, MPI_DOUBLE, 0, 1,
                MPI_COMM_WORLD, &mpi_stat);
    }
#endif
#if DEBUG
  fprintf (stderr, "calibrate_refine: end\n");
#endif
}

/**
 * \fn void calibrate_iterate(Calibrate *calibrate)
 * \brief Function to iterate the algorithm.
 * \param calibrate
 * \brief Calibration data.
 */
void
calibrate_iterate (Calibrate * calibrate)
{
  unsigned int i;
#if DEBUG
  fprintf (stderr, "calibrate_iterate: start\n");
#endif
  calibrate->error_old
    = (double *) g_malloc (calibrate->nbest * sizeof (double));
  calibrate->value_old = (double *)
    g_malloc (calibrate->nbest * calibrate->nvariables * sizeof (double));
  calibrate_step (calibrate);
  calibrate_save_old (calibrate);
  calibrate_refine (calibrate);
  calibrate_print (calibrate);
  for (i = 1; i < calibrate->niterations; ++i)
    {
      calibrate_step (calibrate);
      calibrate_merge_old (calibrate);
      calibrate_refine (calibrate);
      calibrate_print (calibrate);
    }
  g_free (calibrate->error_old);
  g_free (calibrate->value_old);
#if DEBUG
  fprintf (stderr, "calibrate_iterate: end\n");
#endif
}

/**
 * \fn int calibrate_new(Calibrate *calibrate, char *filename)
 * \brief Function to open and perform a calibration.
 * \param calibrate
 * \brief Calibration data pointer.
 * \param filename
 * \brief Input data file name.
 * \return 1 on success, 0 on error.
 */
int
calibrate_new (Calibrate * calibrate, char *filename)
{
  unsigned int i, j, *nbits;
  int error_code;
  xmlChar *buffer;
  xmlNode *node, *child;
  xmlDoc *doc;
  static const xmlChar *template[MAX_NINPUTS] = {
    XML_TEMPLATE1, XML_TEMPLATE2, XML_TEMPLATE3, XML_TEMPLATE4,
    XML_TEMPLATE5, XML_TEMPLATE6, XML_TEMPLATE7, XML_TEMPLATE8
  };

#if DEBUG
  fprintf (stderr, "calibrate_new: start\n");
#endif

  // Parsing the XML data file
  doc = xmlParseFile (filename);
  if (!doc)
    {
      show_error (gettext ("Unable to parse the data file"));
      return 0;
    }

  // Obtaining the root XML node
  node = xmlDocGetRootElement (doc);
  if (!node)
    {
      show_error (gettext ("No XML nodes in the data file"));
      return 0;
    }
  if (xmlStrcmp (node->name, XML_CALIBRATE))
    {
      show_error (gettext ("Bad name of the XML root node in the data file"));
      return 0;
    }

  // Obtaining the simulator file
  if (xmlHasProp (node, XML_SIMULATOR))
    {
      calibrate->simulator = (char *) xmlGetProp (node, XML_SIMULATOR);
    }
  else
    {
      show_error (gettext ("No simulator in the data file"));
      return 0;
    }

  // Obtaining the evaluator file
  if (xmlHasProp (node, XML_EVALUATOR))
    calibrate->evaluator = (char *) xmlGetProp (node, XML_EVALUATOR);
  else
    calibrate->evaluator = NULL;

  // Reading the algorithm
  if (xmlHasProp (node, XML_ALGORITHM))
    {
      buffer = xmlGetProp (node, XML_ALGORITHM);
      if (!xmlStrcmp (buffer, XML_SWEEP))
        {
          calibrate->algorithm = ALGORITHM_SWEEP;
          calibrate_step = calibrate_sweep;
          xmlFree (buffer);
        }
      else if (!xmlStrcmp (buffer, XML_GENETIC))
        {
          nbits = NULL;
          calibrate->algorithm = ALGORITHM_GENETIC;
          calibrate_step = calibrate_genetic;
          xmlFree (buffer);

          // Obtaining population
          if (xmlHasProp (node, XML_NPOPULATION))
            {
              calibrate->nsimulations =
                xml_node_get_uint (node, XML_NPOPULATION, &error_code);
              if (error_code || calibrate->nsimulations < 3)
                {
                  show_error (gettext ("Invalid population number"));
                  return 0;
                }
            }
          else
            {
              show_error (gettext ("No population number in the data file"));
              return 0;
            }

          // Obtaining generations
          if (xmlHasProp (node, XML_NGENERATIONS))
            {
              calibrate->niterations =
                xml_node_get_uint (node, XML_NGENERATIONS, &error_code);
              if (error_code || !calibrate->niterations)
                {
                  show_error (gettext ("Invalid generation number"));
                  return 0;
                }
            }
          else
            {
              show_error (gettext ("No generation number in the data file"));
              return 0;
            }

          // Obtaining mutation probability
          if (xmlHasProp (node, XML_MUTATION))
            {
              calibrate->mutation_ratio =
                xml_node_get_float (node, XML_MUTATION, &error_code);
              if (error_code || calibrate->mutation_ratio < 0.
                  || calibrate->mutation_ratio >= 1.)
                {
                  show_error (gettext ("Invalid mutation probability"));
                  return 0;
                }
            }
          else
            {
              show_error (gettext ("No mutation probability in the data file"));
              return 0;
            }

          // Obtaining reproduction probability
          if (xmlHasProp (node, XML_REPRODUCTION))
            {
              calibrate->reproduction_ratio =
                xml_node_get_float (node, XML_REPRODUCTION, &error_code);
              if (error_code || calibrate->reproduction_ratio < 0.
                  || calibrate->reproduction_ratio >= 1.0)
                {
                  show_error (gettext ("Invalid reproduction probability"));
                  return 0;
                }
            }
          else
            {
              show_error (gettext
                          ("No reproduction probability in the data file"));
              return 0;
            }

          // Obtaining adaptation probability
          if (xmlHasProp (node, XML_ADAPTATION))
            {
              calibrate->adaptation_ratio =
                xml_node_get_float (node, XML_ADAPTATION, &error_code);
              if (error_code || calibrate->adaptation_ratio < 0.
                  || calibrate->adaptation_ratio >= 1.)
                {
                  show_error (gettext ("Invalid adaptation probability"));
                  return 0;
                }
            }
          else
            {
              show_error (gettext
                          ("No adaptation probability in the data file"));
              return 0;
            }

          // Checking survivals
          i = calibrate->mutation_ratio * calibrate->nsimulations;
          i += calibrate->reproduction_ratio * calibrate->nsimulations;
          i += calibrate->adaptation_ratio * calibrate->nsimulations;
          if (i > calibrate->nsimulations - 2)
            {
              show_error (gettext
                          ("No enough survival entities to reproduce the population"));
              return 0;
            }
        }
      else if (!xmlStrcmp (buffer, XML_MONTE_CARLO))
        {
          calibrate->algorithm = ALGORITHM_MONTE_CARLO;
          calibrate_step = calibrate_MonteCarlo;

          // Obtaining the simulations number
          if (xmlHasProp (node, XML_NSIMULATIONS))
            {
              calibrate->nsimulations =
                xml_node_get_uint (node, XML_NSIMULATIONS, &error_code);
            }
          else
            {
              show_error (gettext
                          ("Monte-Carlo: no simulations number in the data file"));
              return 0;
            }
        }
      else
        {
          show_error (gettext ("Unknown algorithm"));
          return 0;
        }
    }
  else
    {
      calibrate->algorithm = ALGORITHM_MONTE_CARLO;
      calibrate_step = calibrate_MonteCarlo;

      // Obtaining the simulations number
      if (xmlHasProp (node, XML_NSIMULATIONS))
        {
          calibrate->nsimulations =
            xml_node_get_uint (node, XML_NSIMULATIONS, &error_code);
        }
      else
        {
          show_error (gettext
                      ("Default: no simulations number in the data file"));
          return 0;
        }
    }

  // Reading the iterations number
  if (calibrate->algorithm != ALGORITHM_GENETIC)
    {
      if (xmlHasProp (node, XML_NITERATIONS))
        {
          calibrate->niterations =
            xml_node_get_uint (node, XML_NITERATIONS, &error_code);
          if (error_code || !calibrate->niterations)
            {
              show_error (gettext
                          ("Invalid iterations number in the data file"));
              return 0;
            }
        }
      else
        calibrate->niterations = 1;
    }

  // Reading the best simulations number
  if (xmlHasProp (node, XML_NBEST))
    {
      calibrate->nbest = xml_node_get_uint (node, XML_NBEST, &error_code);
      if (!calibrate->nbest)
        {
          show_error (gettext ("Null best number in the data file"));
          return 0;
        }
    }
  else
    calibrate->nbest = 1;
  calibrate->simulation_best =
    (unsigned int *) alloca (calibrate->nbest * sizeof (unsigned int));
  calibrate->error_best =
    (double *) alloca (calibrate->nbest * sizeof (double));

  // Reading the algorithm tolerance
  if (xmlHasProp (node, XML_TOLERANCE))
    {
      calibrate->tolerance =
        xml_node_get_float (node, XML_TOLERANCE, &error_code);
      if (error_code || calibrate->tolerance < 0.)
        {
          show_error (gettext ("Invalid tolerance"));
          return 0;
        }
    }
  else
    calibrate->tolerance = 0.;

  // Reading the experimental data
  calibrate->nexperiments = 0;
  calibrate->experiment = NULL;
  calibrate->weight = NULL;
  for (i = 0; i < MAX_NINPUTS; ++i)
    {
      calibrate->template[i] = NULL;
      calibrate->file[i] = NULL;
    }
  for (child = node->children; child; child = child->next)
    {
      if (xmlStrcmp (child->name, XML_EXPERIMENT))
        break;
#if DEBUG
      fprintf (stderr, "calibrate_new: nexperiments=%u\n",
               calibrate->nexperiments);
#endif
      if (xmlHasProp (child, XML_NAME))
        {
          calibrate->experiment = g_realloc (calibrate->experiment,
                                             (1 +
                                              calibrate->nexperiments) *
                                             sizeof (char *));
          calibrate->experiment[calibrate->nexperiments] =
            (char *) xmlGetProp (child, XML_NAME);
        }
      else
        {
          printf ("No experiment %u file name\n", calibrate->nexperiments + 1);
          return 0;
        }
#if DEBUG
      fprintf (stderr, "calibrate_new: experiment=%s\n",
               calibrate->experiment[calibrate->nexperiments]);
#endif
      calibrate->weight = g_realloc (calibrate->weight,
                                     (1 +
                                      calibrate->nexperiments) *
                                     sizeof (double));
      if (xmlHasProp (child, XML_WEIGHT))
        {
          calibrate->weight[calibrate->nexperiments] =
            xml_node_get_float (child, XML_WEIGHT, &error_code);
        }
      else
        calibrate->weight[calibrate->nexperiments] = 1.;
#if DEBUG
      fprintf (stderr, "calibrate_new: weight=%lg\n",
               calibrate->weight[calibrate->nexperiments]);
#endif
      if (!calibrate->nexperiments)
        calibrate->ninputs = 0;
#if DEBUG
      fprintf (stderr, "calibrate_new: template[0]\n");
#endif
      if (xmlHasProp (child, XML_TEMPLATE1))
        {
          calibrate->template[0] = g_realloc (calibrate->template[0],
                                              (1 +
                                               calibrate->nexperiments) *
                                              sizeof (char *));
          calibrate->template[0][calibrate->nexperiments] =
            (char *) xmlGetProp (child, template[0]);
          calibrate->file[0] =
            g_realloc (calibrate->file[0],
                       (1 + calibrate->nexperiments) * sizeof (GMappedFile *));
#if DEBUG
          fprintf (stderr, "calibrate_new: experiment=%u template1=%s\n",
                   calibrate->nexperiments,
                   calibrate->template[0][calibrate->nexperiments]);
#endif
          calibrate->file[0][calibrate->nexperiments] =
            g_mapped_file_new
            (calibrate->template[0][calibrate->nexperiments], 0, NULL);
          if (!calibrate->nexperiments)
            ++calibrate->ninputs;
#if DEBUG
          fprintf (stderr, "calibrate_new: ninputs=%u\n", calibrate->ninputs);
#endif
        }
      else
        {
          printf ("No experiment %u template1\n", calibrate->nexperiments + 1);
          return 0;
        }
      for (j = 1; j < MAX_NINPUTS; ++j)
        {
#if DEBUG
          fprintf (stderr, "calibrate_new: template%u\n", j + 1);
#endif
          if (xmlHasProp (child, template[j]))
            {
              if (calibrate->nexperiments && calibrate->ninputs < 2)
                {
                  printf ("Experiment %u: bad templates number\n",
                          calibrate->nexperiments + 1);
                  return 0;
                }
              calibrate->template[j] = g_realloc (calibrate->template[j],
                                                  (1 +
                                                   calibrate->nexperiments) *
                                                  sizeof (char *));
              calibrate->template[j][calibrate->nexperiments] =
                (char *) xmlGetProp (child, template[j]);
              calibrate->file[j] =
                g_realloc (calibrate->file[j],
                           (1 +
                            calibrate->nexperiments) * sizeof (GMappedFile *));
#if DEBUG
              fprintf (stderr, "calibrate_new: experiment=%u template%u=%s\n",
                       calibrate->nexperiments, j + 1,
                       calibrate->template[j][calibrate->nexperiments]);
#endif
              calibrate->file[j][calibrate->nexperiments] =
                g_mapped_file_new
                (calibrate->template[j][calibrate->nexperiments], 0, NULL);
              if (!calibrate->nexperiments)
                ++calibrate->ninputs;
#if DEBUG
              fprintf (stderr, "calibrate_new: ninputs=%u\n",
                       calibrate->ninputs);
#endif
            }
          else if (calibrate->nexperiments && calibrate->ninputs > 1)
            {
              printf ("No experiment %u template%u\n",
                      calibrate->nexperiments + 1, j + 1);
              return 0;
            }
          else
            break;
        }
      ++calibrate->nexperiments;
#if DEBUG
      fprintf (stderr, "calibrate_new: nexperiments=%u\n",
               calibrate->nexperiments);
#endif
    }
  if (!calibrate->nexperiments)
    {
      show_error (gettext ("No calibration experiments"));
      return 0;
    }

  // Reading the variables data
  calibrate->nvariables = 0;
  calibrate->label = NULL;
  calibrate->rangemin = NULL;
  calibrate->rangemax = NULL;
  calibrate->rangeminabs = NULL;
  calibrate->rangemaxabs = NULL;
  calibrate->format = NULL;
  calibrate->nsweeps = NULL;
  if (calibrate->algorithm == ALGORITHM_SWEEP)
    calibrate->nsimulations = 1;
  for (; child; child = child->next)
    {
      if (xmlStrcmp (child->name, XML_VARIABLE))
        {
          show_error (gettext ("Bad XML node"));
          return 0;
        }
      if (xmlHasProp (child, XML_NAME))
        {
          calibrate->label = g_realloc (calibrate->label,
                                        (1 +
                                         calibrate->nvariables) *
                                        sizeof (char *));
          calibrate->label[calibrate->nvariables] =
            (char *) xmlGetProp (child, XML_NAME);
        }
      else
        {
          printf ("No variable %u name\n", calibrate->nvariables + 1);
          return 0;
        }
      if (xmlHasProp (child, XML_MINIMUM))
        {
          calibrate->rangemin = g_realloc (calibrate->rangemin,
                                           (1 +
                                            calibrate->nvariables) *
                                           sizeof (double));
          calibrate->rangeminabs =
            g_realloc (calibrate->rangeminabs,
                       (1 + calibrate->nvariables) * sizeof (double));
          calibrate->rangemin[calibrate->nvariables] =
            xml_node_get_float (child, XML_MINIMUM, &error_code);
          if (xmlHasProp (child, XML_ABSOLUTE_MINIMUM))
            {
              calibrate->rangeminabs[calibrate->nvariables] =
                xml_node_get_float (child, XML_ABSOLUTE_MINIMUM, &error_code);
            }
          else
            calibrate->rangeminabs[calibrate->nvariables] = -INFINITY;
        }
      else
        {
          printf ("No variable %u minimum range\n", calibrate->nvariables + 1);
          return 0;
        }
      if (xmlHasProp (child, XML_MAXIMUM))
        {
          calibrate->rangemax = g_realloc (calibrate->rangemax,
                                           (1 +
                                            calibrate->nvariables) *
                                           sizeof (double));
          calibrate->rangemaxabs =
            g_realloc (calibrate->rangemaxabs,
                       (1 + calibrate->nvariables) * sizeof (double));
          calibrate->rangemax[calibrate->nvariables] =
            xml_node_get_float (child, XML_MAXIMUM, &error_code);
          if (xmlHasProp (child, XML_ABSOLUTE_MAXIMUM))
            {
              calibrate->rangemaxabs[calibrate->nvariables]
                = xml_node_get_float (child, XML_ABSOLUTE_MINIMUM, &error_code);
            }
          else
            calibrate->rangemaxabs[calibrate->nvariables] = INFINITY;
        }
      else
        {
          printf ("No variable %u maximum range\n", calibrate->nvariables + 1);
          return 0;
        }
      calibrate->format = g_realloc (calibrate->format,
                                     (1 +
                                      calibrate->nvariables) * sizeof (char *));
      if (xmlHasProp (child, XML_FORMAT))
        {
          calibrate->format[calibrate->nvariables] =
            (char *) xmlGetProp (child, XML_FORMAT);
        }
      else
        {
          calibrate->format[calibrate->nvariables] =
            (char *) xmlStrdup (DEFAULT_FORMAT);
        }
      if (calibrate->algorithm == ALGORITHM_SWEEP)
        {
          if (xmlHasProp (child, XML_NSWEEPS))
            {
              calibrate->nsweeps = g_realloc (calibrate->nsweeps,
                                              (1 +
                                               calibrate->nvariables) *
                                              sizeof (unsigned int));
              calibrate->nsweeps[calibrate->nvariables] =
                xml_node_get_uint (child, XML_NSWEEPS, &error_code);
            }
          else
            {
              printf ("No variable %u sweeps number\n",
                      calibrate->nvariables + 1);
              return 0;
            }
          calibrate->nsimulations *= calibrate->nsweeps[calibrate->nvariables];
#if DEBUG
          fprintf (stderr, "calibrate_new: nsweeps=%u nsimulations=%u\n",
                   calibrate->nsweeps[calibrate->nvariables],
                   calibrate->nsimulations);
#endif
        }
      if (calibrate->algorithm == ALGORITHM_GENETIC)
        {
          // Obtaining bits representing each variable
          if (xmlHasProp (child, XML_NBITS))
            {
              nbits = g_realloc (nbits,
                                 (1 +
                                  calibrate->nvariables) *
                                 sizeof (unsigned int));
              i = xml_node_get_uint (child, XML_NBITS, &error_code);
              if (error_code || !i)
                {
                  show_error (gettext ("Invalid bit number"));
                  return 0;
                }
              nbits[calibrate->nvariables] = i;
            }
          else
            {
              printf ("No variable %u bits number\n",
                      calibrate->nvariables + 1);
              return 0;
            }
        }
      ++calibrate->nvariables;
    }
  if (!calibrate->nvariables)
    {
      show_error (gettext ("No calibration variables"));
      return 0;
    }
#if DEBUG
  fprintf (stderr, "calibrate_new: nvariables=%u\n", calibrate->nvariables);
#endif

  // Allocating values
  calibrate->genetic_variable = NULL;
  if (calibrate->algorithm == ALGORITHM_GENETIC)
    {
      calibrate->genetic_variable = (GeneticVariable *)
        g_malloc (calibrate->nvariables * sizeof (GeneticVariable));
      for (i = 0; i < calibrate->nvariables; ++i)
        {
          calibrate->genetic_variable[i].maximum = calibrate->rangemax[i];
          calibrate->genetic_variable[i].minimum = calibrate->rangemin[i];
          calibrate->genetic_variable[i].nbits = nbits[i];
        }
      g_free (nbits);
    }
  calibrate->value = (double *) g_malloc (calibrate->nsimulations *
                                          calibrate->nvariables *
                                          sizeof (double));

  // Calculating simulations to perform on each task
#if HAVE_MPI
  calibrate->nstart = calibrate->mpi_rank * calibrate->nsimulations / ntasks;
  calibrate->nend = (1 + calibrate->mpi_rank) * calibrate->nsimulations
    / ntasks;
#else
  calibrate->nstart = 0;
  calibrate->nend = calibrate->nsimulations;
#endif
#if DEBUG
  fprintf (stderr, "calibrate_new: nstart=%u nend=%u\n", calibrate->nstart,
           calibrate->nend);
#endif

  // Calculating simulations to perform on each thread
  calibrate->thread
    = (unsigned int *) alloca ((1 + nthreads) * sizeof (unsigned int));
  for (i = 0; i <= nthreads; ++i)
    {
      calibrate->thread[i] = calibrate->nstart
        + i * (calibrate->nend - calibrate->nstart) / nthreads;
#if DEBUG
      fprintf (stderr, "calibrate_new: i=%u thread=%u\n", i,
               calibrate->thread[i]);
#endif
    }

  // Opening result file
  calibrate->result = fopen ("result", "w");

  // Performing the algorithm
  switch (calibrate->algorithm)
    {
      // Genetic algorithm
    case ALGORITHM_GENETIC:
      calibrate_genetic (calibrate);
      break;

      // Iterative algorithm
    default:
      calibrate_iterate (calibrate);
    }

  // Closing the XML document
  xmlFreeDoc (doc);

  // Closing result file
  fclose (calibrate->result);

  // Freeing memory
  xmlFree (calibrate->simulator);
  xmlFree (calibrate->evaluator);
  for (i = 0; i < calibrate->nexperiments; ++i)
    {
      xmlFree (calibrate->experiment[i]);
      for (j = 0; j < calibrate->ninputs; ++j)
        {
          xmlFree (calibrate->template[j][i]);
          g_mapped_file_unref (calibrate->file[j][i]);
        }
    }
  g_free (calibrate->experiment);
  for (i = 0; i < calibrate->ninputs; ++i)
    {
      g_free (calibrate->template[i]);
      g_free (calibrate->file[i]);
    }
  for (i = 0; i < calibrate->nvariables; ++i)
    {
      xmlFree (calibrate->label[i]);
      xmlFree (calibrate->format[i]);
    }
  g_free (calibrate->label);
  g_free (calibrate->rangemin);
  g_free (calibrate->rangemax);
  g_free (calibrate->format);
  g_free (calibrate->nsweeps);
  g_free (calibrate->value);
  g_free (calibrate->genetic_variable);

#if DEBUG
  fprintf (stderr, "calibrate_new: end\n");
#endif

  return 1;
}

#if HAVE_GTK

/**
 * \fn void input_save()
 * \brief Function to save the input file.
 */
void
input_save ()
{
  char *buffer;
  xmlDoc *doc;
  xmlNode *node, *child;
  GtkFileChooserDialog *dlg;

  // Opening the saving dialog
  dlg =
    (GtkFileChooserDialog *) gtk_file_chooser_dialog_new (gettext ("Save file"),
                                                          window->window,
                                                          GTK_FILE_CHOOSER_ACTION_SAVE,
                                                          gettext ("_Cancel"),
                                                          GTK_RESPONSE_CANCEL,
                                                          gettext ("_OK"),
                                                          GTK_RESPONSE_OK,
                                                          NULL);
  gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dlg), TRUE);

  // If OK response then saving
  if (gtk_dialog_run (GTK_DIALOG (dlg)) == GTK_RESPONSE_OK)
    {
      // Opening the input file
      doc = xmlNewDoc ((const xmlChar *) "1.0");

      // Setting root XML node
      node = xmlNewDocNode (doc, 0, XML_CALIBRATE, 0);
      xmlDocSetRootElement (doc, node);

      // Adding properties to the root XML node
      xmlSetProp (node, XML_SIMULATOR,
                  (xmlChar *) gtk_file_chooser_get_filename
                  (GTK_FILE_CHOOSER (window->button_simulator)));
      buffer = gtk_file_chooser_get_filename
        (GTK_FILE_CHOOSER (window->button_evaluator));
      if (xmlStrlen ((xmlChar *) buffer))
        xmlSetProp (node, XML_EVALUATOR, (xmlChar *) buffer);

      // Setting the algorithm
      switch (window_get_algorithm ())
        {
        case ALGORITHM_MONTE_CARLO:
          xmlSetProp (node, XML_NSIMULATIONS,
                      (xmlChar *)
                      gtk_entry_get_text (GTK_ENTRY
                                          (window->entry_simulations)));
          xmlSetProp (node, XML_NITERATIONS,
                      (xmlChar *)
                      gtk_entry_get_text (GTK_ENTRY
                                          (window->entry_iterations)));
          xmlSetProp (node, XML_TOLERANCE,
                      (xmlChar *)
                      gtk_entry_get_text (GTK_ENTRY (window->entry_tolerance)));
          xmlSetProp (node, XML_NBEST,
                      (xmlChar *)
                      gtk_entry_get_text (GTK_ENTRY (window->entry_bests)));
          break;
        case ALGORITHM_SWEEP:
          xmlSetProp (node, XML_ALGORITHM, XML_SWEEP);
          xmlSetProp (node, XML_NITERATIONS,
                      (xmlChar *)
                      gtk_entry_get_text (GTK_ENTRY
                                          (window->entry_iterations)));
          xmlSetProp (node, XML_TOLERANCE,
                      (xmlChar *)
                      gtk_entry_get_text (GTK_ENTRY (window->entry_tolerance)));
          xmlSetProp (node, XML_NBEST,
                      (xmlChar *)
                      gtk_entry_get_text (GTK_ENTRY (window->entry_bests)));
          break;
        default:
          xmlSetProp (node, XML_ALGORITHM, XML_GENETIC);
          xmlSetProp (node, XML_NPOPULATION,
                      (xmlChar *)
                      gtk_entry_get_text (GTK_ENTRY
                                          (window->entry_population)));
          xmlSetProp (node, XML_NGENERATIONS,
                      (xmlChar *)
                      gtk_entry_get_text (GTK_ENTRY
                                          (window->entry_generations)));
          xmlSetProp (node, XML_MUTATION,
                      (xmlChar *)
                      gtk_entry_get_text (GTK_ENTRY (window->entry_mutation)));
          xmlSetProp (node, XML_REPRODUCTION,
                      (xmlChar *)
                      gtk_entry_get_text (GTK_ENTRY
                                          (window->entry_reproduction)));
          xmlSetProp (node, XML_ADAPTATION,
                      (xmlChar *)
                      gtk_entry_get_text (GTK_ENTRY
                                          (window->entry_adaptation)));
          break;
        }

      // Saving the XML file
      buffer = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dlg));
      xmlSaveFormatFile (buffer, doc, 1);

      // Freeing memory
      g_free (buffer);
      xmlFreeDoc (doc);
    }

  // Closing and freeing memory
  gtk_widget_destroy (GTK_WIDGET (dlg));
}

/**
 * \fn void window_help()
 * \brief Function to show a help dialog.
 */
void
window_help ()
{
  gchar *authors[] = {
    "Javier Burguete Tolosa (jburguete@eead.csic.es)",
    "Borja Latorre Garcés (borja.latorre@csic.es)",
    NULL
  };
  gtk_show_about_dialog (window->window,
                         "program_name",
                         "Calibrator",
                         "comments",
                         gettext
                         ("A software to make calibrations of empirical parameters"),
                         "authors", authors, "translator-credits",
                         gettext
                         ("Javier Burguete Tolosa (jburguete@eead.csic.es)"),
                         "version", "1.1.0", "copyright",
                         "Copyright 2012-2015 Javier Burguete Tolosa", "logo",
                         window->logo, "website-label", gettext ("Website"),
                         "website", "https://github.com/jburguete/calibrator",
                         NULL);
}

/**
 * \fn int window_get_algorithm()
 * \brief Function to get the algorithm number.
 * \return Algorithm number.
 */
int
window_get_algorithm ()
{
  unsigned int i;
  for (i = 0; i < NALGORITHMS; ++i)
    if (gtk_toggle_button_get_active
        (GTK_TOGGLE_BUTTON (window->button_algorithm[i])))
      break;
  return i;
}

/**
 * \fn void window_update()
 * \brief Function to update the main window view.
 */
void
window_update ()
{
  gtk_widget_hide (GTK_WIDGET (window->label_simulations));
  gtk_widget_hide (GTK_WIDGET (window->entry_simulations));
  gtk_widget_hide (GTK_WIDGET (window->label_iterations));
  gtk_widget_hide (GTK_WIDGET (window->entry_iterations));
  gtk_widget_hide (GTK_WIDGET (window->label_tolerance));
  gtk_widget_hide (GTK_WIDGET (window->entry_tolerance));
  gtk_widget_hide (GTK_WIDGET (window->label_bests));
  gtk_widget_hide (GTK_WIDGET (window->entry_bests));
  gtk_widget_hide (GTK_WIDGET (window->label_population));
  gtk_widget_hide (GTK_WIDGET (window->entry_population));
  gtk_widget_hide (GTK_WIDGET (window->label_generations));
  gtk_widget_hide (GTK_WIDGET (window->entry_generations));
  gtk_widget_hide (GTK_WIDGET (window->label_mutation));
  gtk_widget_hide (GTK_WIDGET (window->entry_mutation));
  gtk_widget_hide (GTK_WIDGET (window->label_reproduction));
  gtk_widget_hide (GTK_WIDGET (window->entry_reproduction));
  gtk_widget_hide (GTK_WIDGET (window->label_sweeps));
  gtk_widget_hide (GTK_WIDGET (window->entry_sweeps));
  gtk_widget_hide (GTK_WIDGET (window->label_bits));
  gtk_widget_hide (GTK_WIDGET (window->entry_bits));
  switch (window_get_algorithm ())
    {
    case ALGORITHM_MONTE_CARLO:
      gtk_widget_show (GTK_WIDGET (window->label_simulations));
      gtk_widget_show (GTK_WIDGET (window->entry_simulations));
      gtk_widget_show (GTK_WIDGET (window->label_iterations));
      gtk_widget_show (GTK_WIDGET (window->entry_iterations));
      gtk_widget_show (GTK_WIDGET (window->label_tolerance));
      gtk_widget_show (GTK_WIDGET (window->entry_tolerance));
      gtk_widget_show (GTK_WIDGET (window->label_bests));
      gtk_widget_show (GTK_WIDGET (window->entry_bests));
      break;
    case ALGORITHM_SWEEP:
      gtk_widget_show (GTK_WIDGET (window->label_iterations));
      gtk_widget_show (GTK_WIDGET (window->entry_iterations));
      gtk_widget_show (GTK_WIDGET (window->label_tolerance));
      gtk_widget_show (GTK_WIDGET (window->entry_tolerance));
      gtk_widget_show (GTK_WIDGET (window->label_bests));
      gtk_widget_show (GTK_WIDGET (window->entry_bests));
      gtk_widget_show (GTK_WIDGET (window->label_sweeps));
      gtk_widget_show (GTK_WIDGET (window->entry_sweeps));
      break;
    default:
      gtk_widget_show (GTK_WIDGET (window->label_population));
      gtk_widget_show (GTK_WIDGET (window->entry_population));
      gtk_widget_show (GTK_WIDGET (window->label_generations));
      gtk_widget_show (GTK_WIDGET (window->entry_generations));
      gtk_widget_show (GTK_WIDGET (window->label_mutation));
      gtk_widget_show (GTK_WIDGET (window->entry_mutation));
      gtk_widget_show (GTK_WIDGET (window->label_reproduction));
      gtk_widget_show (GTK_WIDGET (window->entry_reproduction));
      gtk_widget_show (GTK_WIDGET (window->label_adaptation));
      gtk_widget_show (GTK_WIDGET (window->entry_adaptation));
      gtk_widget_show (GTK_WIDGET (window->label_bits));
      gtk_widget_show (GTK_WIDGET (window->entry_bits));
    }
}

/**
 * \fn void window_open()
 * \brief Function to open the input data.
 */
void
window_open ()
{
}

/**
 * \fn int window_new(GtkApplication *application)
 * \brief Function to open the main window.
 * \param application
 * \brief Main GtkApplication.
 */
void
window_new (GtkApplication * application)
{
  unsigned int i;
  char *label_algorithm[NALGORITHMS] = {
    "_Monte-Carlo",
    gettext ("_Sweep"),
    gettext ("_Genetic")
  };

  // Creating the window
  window->window = (GtkWindow *) gtk_application_window_new (application);

  // Setting the window title
  gtk_window_set_title (window->window, PROGRAM_INTERFACE);

  // Creating the save button
  window->button_save
    = (GtkButton *) gtk_button_new_with_mnemonic (gettext ("_Save"));
  g_signal_connect (window->button_save, "clicked", input_save, NULL);

  // Creating the help button
  window->button_help
    = (GtkButton *) gtk_button_new_with_mnemonic (gettext ("_Help"));
  g_signal_connect (window->button_help, "clicked", window_help, NULL);

  // Creating the exit button
  window->button_exit
    = (GtkButton *) gtk_button_new_with_mnemonic (gettext ("E_xit"));
  g_signal_connect_swapped (window->button_exit, "clicked",
                            (void (*)) gtk_widget_destroy, window->window);

  // Creating the simulator program label and entry
  window->label_simulator
    = (GtkLabel *) gtk_label_new (gettext ("Simulator program"));
  window->button_simulator
    = (GtkFileChooserButton *) gtk_file_chooser_button_new
    (gettext ("Simulator program"), GTK_FILE_CHOOSER_ACTION_OPEN);

  // Creating the evaluator program label and entry
  window->label_evaluator
    = (GtkLabel *) gtk_label_new (gettext ("Evaluator program"));
  window->button_evaluator
    = (GtkFileChooserButton *) gtk_file_chooser_button_new
    (gettext ("Evaluator program"), GTK_FILE_CHOOSER_ACTION_OPEN);

  // Creating the algorithm properties
  window->label_simulations = (GtkLabel *) gtk_label_new
    (gettext ("Simulations number"));
  window->entry_simulations
    = (GtkSpinButton *) gtk_spin_button_new_with_range (1., 1.e12, 1.);
  window->label_iterations
    = (GtkLabel *) gtk_label_new (gettext ("Iterations number"));
  window->entry_iterations
    = (GtkSpinButton *) gtk_spin_button_new_with_range (1., 1.e6, 1.);
  window->label_tolerance = (GtkLabel *) gtk_label_new (gettext ("Tolerance"));
  window->entry_tolerance
    = (GtkSpinButton *) gtk_spin_button_new_with_range (0., 1., 0.001);
  window->label_bests = (GtkLabel *) gtk_label_new (gettext ("Bests number"));
  window->entry_bests
    = (GtkSpinButton *) gtk_spin_button_new_with_range (1., 1.e6, 1.);
  window->label_population = (GtkLabel *) gtk_label_new
    (gettext ("Population number"));
  window->entry_population
    = (GtkSpinButton *) gtk_spin_button_new_with_range (1., 1.e12, 1.);
  window->label_generations
    = (GtkLabel *) gtk_label_new (gettext ("Generations number"));
  window->entry_generations
    = (GtkSpinButton *) gtk_spin_button_new_with_range (1., 1.e6, 1.);
  window->label_mutation
    = (GtkLabel *) gtk_label_new (gettext ("Mutation ratio"));
  window->entry_mutation
    = (GtkSpinButton *) gtk_spin_button_new_with_range (0., 1., 0.001);
  window->label_reproduction
    = (GtkLabel *) gtk_label_new (gettext ("Reproduction ratio"));
  window->entry_reproduction
    = (GtkSpinButton *) gtk_spin_button_new_with_range (0., 1., 0.001);
  window->label_adaptation
    = (GtkLabel *) gtk_label_new (gettext ("Adaptation ratio"));
  window->entry_adaptation
    = (GtkSpinButton *) gtk_spin_button_new_with_range (0., 1., 0.001);

  // Creating the array of algorithms
  window->grid_algorithm = (GtkGrid *) gtk_grid_new ();
  window->button_algorithm[0] = (GtkRadioButton *)
    gtk_radio_button_new_with_mnemonic (NULL, label_algorithm[0]);
  gtk_grid_attach (window->grid_algorithm,
                   GTK_WIDGET (window->button_algorithm[0]), 0, 0, 1, 1);
  g_signal_connect (window->button_algorithm[0], "clicked", window_update,
                    NULL);
  for (i = 0; ++i < NALGORITHMS;)
    {
      window->button_algorithm[i] = (GtkRadioButton *)
        gtk_radio_button_new_with_mnemonic (gtk_radio_button_get_group
                                            (window->button_algorithm[0]),
                                            label_algorithm[i]);
      gtk_grid_attach (window->grid_algorithm,
                       GTK_WIDGET (window->button_algorithm[i]), 0, i, 1, 1);
      g_signal_connect (window->button_algorithm[i], "clicked", window_update,
                        NULL);
    }
  gtk_grid_attach (window->grid_algorithm,
                   GTK_WIDGET (window->label_simulations), 0, NALGORITHMS, 1,
                   1);
  gtk_grid_attach (window->grid_algorithm,
                   GTK_WIDGET (window->entry_simulations), 1, NALGORITHMS, 1,
                   1);
  gtk_grid_attach (window->grid_algorithm,
                   GTK_WIDGET (window->label_iterations), 0, NALGORITHMS + 1, 1,
                   1);
  gtk_grid_attach (window->grid_algorithm,
                   GTK_WIDGET (window->entry_iterations), 1, NALGORITHMS + 1, 1,
                   1);
  gtk_grid_attach (window->grid_algorithm, GTK_WIDGET (window->label_tolerance),
                   0, NALGORITHMS + 2, 1, 1);
  gtk_grid_attach (window->grid_algorithm, GTK_WIDGET (window->entry_tolerance),
                   1, NALGORITHMS + 2, 1, 1);
  gtk_grid_attach (window->grid_algorithm, GTK_WIDGET (window->label_bests), 0,
                   NALGORITHMS + 3, 1, 1);
  gtk_grid_attach (window->grid_algorithm, GTK_WIDGET (window->entry_bests), 1,
                   NALGORITHMS + 3, 1, 1);
  gtk_grid_attach (window->grid_algorithm,
                   GTK_WIDGET (window->label_population), 0, NALGORITHMS + 4, 1,
                   1);
  gtk_grid_attach (window->grid_algorithm,
                   GTK_WIDGET (window->entry_population), 1, NALGORITHMS + 4, 1,
                   1);
  gtk_grid_attach (window->grid_algorithm,
                   GTK_WIDGET (window->label_generations), 0, NALGORITHMS + 5,
                   1, 1);
  gtk_grid_attach (window->grid_algorithm,
                   GTK_WIDGET (window->entry_generations), 1, NALGORITHMS + 5,
                   1, 1);
  gtk_grid_attach (window->grid_algorithm, GTK_WIDGET (window->label_mutation),
                   0, NALGORITHMS + 6, 1, 1);
  gtk_grid_attach (window->grid_algorithm, GTK_WIDGET (window->entry_mutation),
                   1, NALGORITHMS + 6, 1, 1);
  gtk_grid_attach (window->grid_algorithm,
                   GTK_WIDGET (window->label_reproduction), 0, NALGORITHMS + 7,
                   1, 1);
  gtk_grid_attach (window->grid_algorithm,
                   GTK_WIDGET (window->entry_reproduction), 1, NALGORITHMS + 7,
                   1, 1);
  gtk_grid_attach (window->grid_algorithm,
                   GTK_WIDGET (window->label_adaptation), 0, NALGORITHMS + 8, 1,
                   1);
  gtk_grid_attach (window->grid_algorithm,
                   GTK_WIDGET (window->entry_adaptation), 1, NALGORITHMS + 8, 1,
                   1);
  window->frame_algorithm = (GtkFrame *) gtk_frame_new (gettext ("Algorithm"));
  gtk_container_add (GTK_CONTAINER (window->frame_algorithm),
                     GTK_WIDGET (window->grid_algorithm));

  // Creating the variable widgets
  window->combo_variable = (GtkComboBoxText *) gtk_combo_box_text_new ();
  window->button_add_variable =
    (GtkButton *) gtk_button_new_from_icon_name ("list-add",
                                                 GTK_ICON_SIZE_BUTTON);
  gtk_widget_set_tooltip_text (GTK_WIDGET (window->button_add_variable),
                               gettext ("Add variable"));
  window->button_remove_variable =
    (GtkButton *) gtk_button_new_from_icon_name ("list-remove",
                                                 GTK_ICON_SIZE_BUTTON);
  gtk_widget_set_tooltip_text (GTK_WIDGET (window->button_remove_variable),
                               gettext ("Remove variable"));
  window->label_variable = (GtkLabel *) gtk_label_new (gettext ("Name"));
  window->entry_variable = (GtkEntry *) gtk_entry_new ();
  window->label_min = (GtkLabel *) gtk_label_new (gettext ("Minimum"));
  window->entry_min = (GtkEntry *) gtk_entry_new ();
  window->label_max = (GtkLabel *) gtk_label_new (gettext ("Maximum"));
  window->entry_max = (GtkEntry *) gtk_entry_new ();
  window->label_absmin =
    (GtkLabel *) gtk_label_new (gettext ("Absolute minimum"));
  window->entry_absmin = (GtkEntry *) gtk_entry_new ();
  window->label_absmax =
    (GtkLabel *) gtk_label_new (gettext ("Absolute maximum"));
  window->entry_absmax = (GtkEntry *) gtk_entry_new ();
  window->label_sweeps = (GtkLabel *) gtk_label_new (gettext ("Sweeps number"));
  window->entry_sweeps =
    (GtkSpinButton *) gtk_spin_button_new_with_range (1., 1.e12, 1.);
  window->label_bits = (GtkLabel *) gtk_label_new (gettext ("Bits number"));
  window->entry_bits =
    (GtkSpinButton *) gtk_spin_button_new_with_range (1., 64., 1.);
  window->grid_variable = (GtkGrid *) gtk_grid_new ();
  gtk_grid_attach (window->grid_variable, GTK_WIDGET (window->combo_variable),
                   0, 0, 2, 1);
  gtk_grid_attach (window->grid_variable,
                   GTK_WIDGET (window->button_add_variable), 2, 0, 1, 1);
  gtk_grid_attach (window->grid_variable,
                   GTK_WIDGET (window->button_remove_variable), 3, 0, 1, 1);
  gtk_grid_attach (window->grid_variable, GTK_WIDGET (window->label_variable),
                   0, 1, 1, 1);
  gtk_grid_attach (window->grid_variable, GTK_WIDGET (window->entry_variable),
                   1, 1, 3, 1);
  gtk_grid_attach (window->grid_variable, GTK_WIDGET (window->label_min), 0, 2,
                   1, 1);
  gtk_grid_attach (window->grid_variable, GTK_WIDGET (window->entry_min), 1, 2,
                   3, 1);
  gtk_grid_attach (window->grid_variable, GTK_WIDGET (window->label_max), 0, 3,
                   1, 1);
  gtk_grid_attach (window->grid_variable, GTK_WIDGET (window->entry_max), 1, 3,
                   3, 1);
  gtk_grid_attach (window->grid_variable, GTK_WIDGET (window->label_absmin), 0,
                   4, 1, 1);
  gtk_grid_attach (window->grid_variable, GTK_WIDGET (window->entry_absmin), 1,
                   4, 3, 1);
  gtk_grid_attach (window->grid_variable, GTK_WIDGET (window->label_absmax), 0,
                   5, 1, 1);
  gtk_grid_attach (window->grid_variable, GTK_WIDGET (window->entry_absmax), 1,
                   5, 3, 1);
  gtk_grid_attach (window->grid_variable, GTK_WIDGET (window->label_sweeps), 0,
                   6, 1, 1);
  gtk_grid_attach (window->grid_variable, GTK_WIDGET (window->entry_sweeps), 1,
                   6, 3, 1);
  gtk_grid_attach (window->grid_variable, GTK_WIDGET (window->label_bits), 0, 6,
                   1, 1);
  gtk_grid_attach (window->grid_variable, GTK_WIDGET (window->entry_bits), 1, 6,
                   3, 1);
  window->frame_variable = (GtkFrame *) gtk_frame_new (gettext ("Variable"));
  gtk_container_add (GTK_CONTAINER (window->frame_variable),
                     GTK_WIDGET (window->grid_variable));

  // Creating the experiment widgets
  window->combo_experiment = (GtkComboBoxText *) gtk_combo_box_text_new ();
  window->button_add_experiment =
    (GtkButton *) gtk_button_new_from_icon_name ("list-add",
                                                 GTK_ICON_SIZE_BUTTON);
  gtk_widget_set_tooltip_text (GTK_WIDGET (window->button_add_experiment),
                               gettext ("Add experiment"));
  window->button_remove_experiment =
    (GtkButton *) gtk_button_new_from_icon_name ("list-remove",
                                                 GTK_ICON_SIZE_BUTTON);
  gtk_widget_set_tooltip_text (GTK_WIDGET (window->button_remove_experiment),
                               gettext ("Remove experiment"));
  window->label_experiment = (GtkLabel *) gtk_label_new (gettext ("Name"));
  window->button_experiment =
    (GtkFileChooserButton *)
    gtk_file_chooser_button_new (gettext ("Experimental data file"),
                                 GTK_FILE_CHOOSER_ACTION_OPEN);
  window->label_weight = (GtkLabel *) gtk_label_new (gettext ("Weight"));
  window->entry_weight =
    (GtkSpinButton *) gtk_spin_button_new_with_range (0., 1., 0.001);
  window->grid_experiment = (GtkGrid *) gtk_grid_new ();
  gtk_grid_attach (window->grid_experiment,
                   GTK_WIDGET (window->combo_experiment), 0, 0, 2, 1);
  gtk_grid_attach (window->grid_experiment,
                   GTK_WIDGET (window->button_add_experiment), 2, 0, 1, 1);
  gtk_grid_attach (window->grid_experiment,
                   GTK_WIDGET (window->button_remove_experiment), 3, 0, 1, 1);
  gtk_grid_attach (window->grid_experiment,
                   GTK_WIDGET (window->label_experiment), 0, 1, 1, 1);
  gtk_grid_attach (window->grid_experiment,
                   GTK_WIDGET (window->button_experiment), 1, 1, 3, 1);
  gtk_grid_attach (window->grid_experiment, GTK_WIDGET (window->label_weight),
                   0, 2, 1, 1);
  gtk_grid_attach (window->grid_experiment, GTK_WIDGET (window->entry_weight),
                   1, 2, 3, 1);
  window->frame_experiment =
    (GtkFrame *) gtk_frame_new (gettext ("Experiment"));
  gtk_container_add (GTK_CONTAINER (window->frame_experiment),
                     GTK_WIDGET (window->grid_experiment));

  // Creating the grid and attaching the widgets to the grid
  window->grid = (GtkGrid *) gtk_grid_new ();
  gtk_grid_attach (window->grid, GTK_WIDGET (window->button_save), 0, 0, 1, 1);
  gtk_grid_attach (window->grid, GTK_WIDGET (window->button_help), 1, 0, 1, 1);
  gtk_grid_attach (window->grid, GTK_WIDGET (window->button_exit), 2, 0, 1, 1);
  gtk_grid_attach (window->grid, GTK_WIDGET (window->label_simulator), 0, 1, 1,
                   1);
  gtk_grid_attach (window->grid, GTK_WIDGET (window->button_simulator), 1, 1, 1,
                   1);
  gtk_grid_attach (window->grid, GTK_WIDGET (window->label_evaluator), 2, 1, 1,
                   1);
  gtk_grid_attach (window->grid, GTK_WIDGET (window->button_evaluator), 3, 1, 1,
                   1);
  gtk_grid_attach (window->grid, GTK_WIDGET (window->frame_algorithm), 0, 2, 2,
                   1);
  gtk_grid_attach (window->grid, GTK_WIDGET (window->frame_variable), 2, 2, 2,
                   1);
  gtk_grid_attach (window->grid, GTK_WIDGET (window->frame_experiment), 4, 2, 2,
                   1);
  gtk_container_add (GTK_CONTAINER (window->window), GTK_WIDGET (window->grid));

  // Setting the window logo
  window->logo = gtk_image_get_pixbuf
    (GTK_IMAGE (gtk_image_new_from_file ("logo.png")));
  gtk_window_set_icon (window->window, window->logo);

  // Showing the window
  gtk_widget_show_all (GTK_WIDGET (window->window));
  window_update ();
}

#endif

/**
 * \fn int cores_number()
 * \brief Function to obtain the cores number.
 * \return Cores number.
 */
int
cores_number ()
{
#ifdef G_OS_WIN32
  SYSTEM_INFO sysinfo;
  GetSystemInfo (&sysinfo);
  return sysinfo.dwNumberOfProcessors;
#else
  return (int) sysconf (_SC_NPROCESSORS_ONLN);
#endif
}

/**
 * \fn int main(int argn, char **argc)
 * \brief Main function.
 * \param argn
 * \brief Arguments number.
 * \param argc
 * \brief Arguments pointer.
 * \return 0 on success, >0 on error.
 */
int
main (int argn, char **argc)
{
#if HAVE_GTK

  int status;
  char *buffer;
  GtkApplication *application;
  xmlKeepBlanksDefault (0);
  setlocale (LC_ALL, "");
  setlocale (LC_NUMERIC, "C");
  buffer = g_get_current_dir ();
  bindtextdomain
    (PROGRAM_INTERFACE, g_build_filename (buffer, LOCALE_DIR, NULL));
  bind_textdomain_codeset (PROGRAM_INTERFACE, "UTF-8");
  textdomain (PROGRAM_INTERFACE);
  gtk_disable_setlocale ();
  application = gtk_application_new ("git.jburguete.calibrator",
                                     G_APPLICATION_FLAGS_NONE);
  g_signal_connect (application, "activate", (void (*)) window_new, NULL);
  status = g_application_run (G_APPLICATION (application), argn, argc);
  g_object_unref (application);
  return status;

#else

#if HAVE_MPI
  // Starting MPI
  MPI_Init (&argn, &argc);
  MPI_Comm_size (MPI_COMM_WORLD, &ntasks);
  MPI_Comm_rank (MPI_COMM_WORLD, &calibrate->mpi_rank);
  printf ("rank=%d tasks=%d\n", calibrate->mpi_rank, ntasks);
#else
  ntasks = 1;
#endif

  // Checking syntax
  if (!(argn == 2 || (argn == 4 && !strcmp (argc[1], "-nthreads"))))
    {
      printf ("The syntax is:\ncalibrator [-nthreads x] data_file\n");
#if HAVE_MPI
      // Closing MPI
      MPI_Finalize ();
#endif
      return 1;
    }

  // Getting threads number
  if (argn == 2)
    nthreads = cores_number ();
  else
    nthreads = atoi (argc[2]);
  printf ("nthreads=%u\n", nthreads);
#if GLIB_MINOR_VERSION < 32
  g_thread_init (NULL);
  if (nthreads > 1)
    mutex = g_mutex_new ();
#endif

  // Starting pseudo-random numbers generator
  calibrate->rng = gsl_rng_alloc (gsl_rng_taus2);
  gsl_rng_set (calibrate->rng, DEFAULT_RANDOM_SEED);

  // Allowing spaces in the XML data file
  xmlKeepBlanksDefault (0);

  // Making calibration
  calibrate_new (calibrate, argc[argn - 1]);

  // Freeing memory
  gsl_rng_free (calibrate->rng);

#if HAVE_MPI
  // Closing MPI
  MPI_Finalize ();
#endif

  return 0;

#endif
}
