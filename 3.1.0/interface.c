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
 * \file interface.c
 * \brief Source file to define the graphical interface functions.
 * \authors Javier Burguete and Borja Latorre.
 * \copyright Copyright 2012-2016, all rights reserved.
 */
#define _GNU_SOURCE
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gsl/gsl_rng.h>
#include <libxml/parser.h>
#include <libintl.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <json-glib/json-glib.h>
#ifdef G_OS_WIN32
#include <windows.h>
#elif !defined (__BSD_VISIBLE)
#include <alloca.h>
#endif
#if HAVE_MPI
#include <mpi.h>
#endif
#include <gio/gio.h>
#include <gtk/gtk.h>
#include "genetic/genetic.h"
#include "utils.h"
#include "experiment.h"
#include "variable.h"
#include "input.h"
#include "optimize.h"
#include "interface.h"

#define DEBUG_INTERFACE 1       ///< Macro to debug interface functions.

/**
 * \def INPUT_FILE
 * \brief Macro to define the initial input file.
 */
#ifdef G_OS_WIN32
#define INPUT_FILE "test-ga-win.xml"
#else
#define INPUT_FILE "test-ga.xml"
#endif

const char *logo[] = {
  "32 32 3 1",
  "     c None",
  ".    c #0000FF",
  "+    c #FF0000",
  "                                ",
  "                                ",
  "                                ",
  "     .      .      .      .     ",
  "     .      .      .      .     ",
  "     .      .      .      .     ",
  "     .      .      .      .     ",
  "     .      .     +++     .     ",
  "     .      .    +++++    .     ",
  "     .      .    +++++    .     ",
  "     .      .    +++++    .     ",
  "    +++     .     +++    +++    ",
  "   +++++    .      .    +++++   ",
  "   +++++    .      .    +++++   ",
  "   +++++    .      .    +++++   ",
  "    +++     .      .     +++    ",
  "     .      .      .      .     ",
  "     .     +++     .      .     ",
  "     .    +++++    .      .     ",
  "     .    +++++    .      .     ",
  "     .    +++++    .      .     ",
  "     .     +++     .      .     ",
  "     .      .      .      .     ",
  "     .      .      .      .     ",
  "     .      .      .      .     ",
  "     .      .      .      .     ",
  "     .      .      .      .     ",
  "     .      .      .      .     ",
  "     .      .      .      .     ",
  "                                ",
  "                                ",
  "                                "
};                              ///< Logo pixmap.

/*
const char * logo[] = {
"32 32 3 1",
"     c #FFFFFFFFFFFF",
".    c #00000000FFFF",
"X    c #FFFF00000000",
"                                ",
"                                ",
"                                ",
"     .      .      .      .     ",
"     .      .      .      .     ",
"     .      .      .      .     ",
"     .      .      .      .     ",
"     .      .     XXX     .     ",
"     .      .    XXXXX    .     ",
"     .      .    XXXXX    .     ",
"     .      .    XXXXX    .     ",
"    XXX     .     XXX    XXX    ",
"   XXXXX    .      .    XXXXX   ",
"   XXXXX    .      .    XXXXX   ",
"   XXXXX    .      .    XXXXX   ",
"    XXX     .      .     XXX    ",
"     .      .      .      .     ",
"     .     XXX     .      .     ",
"     .    XXXXX    .      .     ",
"     .    XXXXX    .      .     ",
"     .    XXXXX    .      .     ",
"     .     XXX     .      .     ",
"     .      .      .      .     ",
"     .      .      .      .     ",
"     .      .      .      .     ",
"     .      .      .      .     ",
"     .      .      .      .     ",
"     .      .      .      .     ",
"     .      .      .      .     ",
"                                ",
"                                ",
"                                "};
*/

Options options[1];
  ///< Options struct to define the options dialog.
Running running[1];
  ///< Running struct to define the running dialog.
Window window[1];
  ///< Window struct to define the main interface window.

/**
 * \fn void input_save_direction_xml (xmlNode *node)
 * \brief Function to save the direction search method data in a XML node.
 * \param node
 * \brief XML node.
 */
void
input_save_direction_xml (xmlNode * node)
{
#if DEBUG_INTERFACE
  fprintf (stderr, "input_save_direction_xml: start\n");
#endif
  if (input->nsteps)
    {
      xml_node_set_uint (node, (const xmlChar *) LABEL_NSTEPS, input->nsteps);
      if (input->relaxation != DEFAULT_RELAXATION)
        xml_node_set_float (node, (const xmlChar * ) LABEL_RELAXATION,
				            input->relaxation);
      switch (input->direction)
        {
        case DIRECTION_METHOD_COORDINATES:
          xmlSetProp (node, (const xmlChar *) LABEL_DIRECTION,
				      (const xmlChar *) LABEL_COORDINATES);
          break;
        default:
          xmlSetProp (node, (const xmlChar *) LABEL_DIRECTION,
				      (const xmlChar *) LABEL_RANDOM);
          xml_node_set_uint (node, (const xmlChar *) LABEL_NESTIMATES,
				             input->nestimates);
        }
    }
#if DEBUG_INTERFACE
  fprintf (stderr, "input_save_direction_xml: end\n");
#endif
}

/**
 * \fn void input_save_direction_json (JsonNode *node)
 * \brief Function to save the direction search method data in a JSON node.
 * \param node
 * \brief JSON node.
 */
void
input_save_direction_json (JsonNode * node)
{
  JsonObject *object;
#if DEBUG_INTERFACE
  fprintf (stderr, "input_save_direction_json: start\n");
#endif
  object = json_node_get_object (node);
  if (input->nsteps)
    {
      json_object_set_uint (object, LABEL_NSTEPS, input->nsteps);
      if (input->relaxation != DEFAULT_RELAXATION)
        json_object_set_float (object, LABEL_RELAXATION, input->relaxation);
      switch (input->direction)
        {
        case DIRECTION_METHOD_COORDINATES:
          json_object_set_string_member (object, LABEL_DIRECTION,
				                         LABEL_COORDINATES);
          break;
        default:
          json_object_set_string_member (object, LABEL_DIRECTION, LABEL_RANDOM);
          json_object_set_uint (object, LABEL_NESTIMATES, input->nestimates);
        }
    }
#if DEBUG_INTERFACE
  fprintf (stderr, "input_save_direction_json: end\n");
#endif
}

/**
 * \fn void input_save_xml (xmlDoc * doc)
 * \brief Function to save the input file in XML format.
 * \param doc
 * \brief xmlDoc struct.
 */
void
input_save_xml (xmlDoc * doc)
{
  unsigned int i, j;
  char *buffer;
  xmlNode *node, *child;
  GFile *file, *file2;

#if DEBUG_INTERFACE
  fprintf (stderr, "input_save_xml: start\n");
#endif

  // Setting root XML node
  node = xmlNewDocNode (doc, 0, (const xmlChar *) LABEL_OPTIMIZE, 0);
  xmlDocSetRootElement (doc, node);

  // Adding properties to the root XML node
  if (xmlStrcmp ((const xmlChar *) input->result, (const xmlChar *) result_name))
    xmlSetProp (node, (const xmlChar *) LABEL_RESULT_FILE, (xmlChar *) input->result);
  if (xmlStrcmp ((const xmlChar *) input->variables, (const xmlChar *) variables_name))
    xmlSetProp (node, (const xmlChar *) LABEL_VARIABLES_FILE, (xmlChar *) input->variables);
  file = g_file_new_for_path (input->directory);
  file2 = g_file_new_for_path (input->simulator);
  buffer = g_file_get_relative_path (file, file2);
  g_object_unref (file2);
  xmlSetProp (node, (const xmlChar *) LABEL_SIMULATOR, (xmlChar *) buffer);
  g_free (buffer);
  if (input->evaluator)
    {
      file2 = g_file_new_for_path (input->evaluator);
      buffer = g_file_get_relative_path (file, file2);
      g_object_unref (file2);
      if (xmlStrlen ((xmlChar *) buffer))
        xmlSetProp (node, (const xmlChar *) LABEL_EVALUATOR, (xmlChar *) buffer);
      g_free (buffer);
    }
  if (input->seed != DEFAULT_RANDOM_SEED)
    xml_node_set_uint (node, (const xmlChar *) LABEL_SEED, input->seed);

  // Setting the algorithm
  buffer = (char *) g_slice_alloc (64);
  switch (input->algorithm)
    {
    case ALGORITHM_MONTE_CARLO:
      xmlSetProp (node, (const xmlChar *) LABEL_ALGORITHM, (const xmlChar *) LABEL_MONTE_CARLO);
      snprintf (buffer, 64, "%u", input->nsimulations);
      xmlSetProp (node, (const xmlChar *) LABEL_NSIMULATIONS, (xmlChar *) buffer);
      snprintf (buffer, 64, "%u", input->niterations);
      xmlSetProp (node, (const xmlChar *) LABEL_NITERATIONS, (xmlChar *) buffer);
      snprintf (buffer, 64, "%.3lg", input->tolerance);
      xmlSetProp (node, (const xmlChar *) LABEL_TOLERANCE, (xmlChar *) buffer);
      snprintf (buffer, 64, "%u", input->nbest);
      xmlSetProp (node, (const xmlChar *) LABEL_NBEST, (xmlChar *) buffer);
      input_save_direction_xml (node);
      break;
    case ALGORITHM_SWEEP:
      xmlSetProp (node, (const xmlChar *) LABEL_ALGORITHM, (const xmlChar *) LABEL_SWEEP);
      snprintf (buffer, 64, "%u", input->niterations);
      xmlSetProp (node, (const xmlChar *) LABEL_NITERATIONS, (xmlChar *) buffer);
      snprintf (buffer, 64, "%.3lg", input->tolerance);
      xmlSetProp (node, (const xmlChar *) LABEL_TOLERANCE, (xmlChar *) buffer);
      snprintf (buffer, 64, "%u", input->nbest);
      xmlSetProp (node, (const xmlChar *) LABEL_NBEST, (xmlChar *) buffer);
      input_save_direction_xml (node);
      break;
    default:
      xmlSetProp (node, (const xmlChar *) LABEL_ALGORITHM, (const xmlChar *) LABEL_GENETIC);
      snprintf (buffer, 64, "%u", input->nsimulations);
      xmlSetProp (node, (const xmlChar *) LABEL_NPOPULATION, (xmlChar *) buffer);
      snprintf (buffer, 64, "%u", input->niterations);
      xmlSetProp (node, (const xmlChar *) LABEL_NGENERATIONS, (xmlChar *) buffer);
      snprintf (buffer, 64, "%.3lg", input->mutation_ratio);
      xmlSetProp (node, (const xmlChar *) LABEL_MUTATION, (xmlChar *) buffer);
      snprintf (buffer, 64, "%.3lg", input->reproduction_ratio);
      xmlSetProp (node, (const xmlChar *) LABEL_REPRODUCTION, (xmlChar *) buffer);
      snprintf (buffer, 64, "%.3lg", input->adaptation_ratio);
      xmlSetProp (node, (const xmlChar *) LABEL_ADAPTATION, (xmlChar *) buffer);
      break;
    }
  g_slice_free1 (64, buffer);
  if (input->threshold != 0.)
    xml_node_set_float (node, (const xmlChar *) LABEL_THRESHOLD, input->threshold);

  // Setting the experimental data
  for (i = 0; i < input->nexperiments; ++i)
    {
      child = xmlNewChild (node, 0, (const xmlChar *) LABEL_EXPERIMENT, 0);
      xmlSetProp (child, (const xmlChar *) LABEL_NAME, (xmlChar *) input->experiment[i].name);
      if (input->experiment[i].weight != 1.)
        xml_node_set_float (child, (const xmlChar *) LABEL_WEIGHT, input->experiment[i].weight);
      for (j = 0; j < input->experiment->ninputs; ++j)
        xmlSetProp (child, (const xmlChar *) template[j],
                    (xmlChar *) input->experiment[i].template[j]);
    }

  // Setting the variables data
  for (i = 0; i < input->nvariables; ++i)
    {
      child = xmlNewChild (node, 0, (const xmlChar *) LABEL_VARIABLE, 0);
      xmlSetProp (child, (const xmlChar *) LABEL_NAME, (xmlChar *) input->variable[i].name);
      xml_node_set_float (child, (const xmlChar *) LABEL_MINIMUM, input->variable[i].rangemin);
      if (input->variable[i].rangeminabs != -G_MAXDOUBLE)
        xml_node_set_float (child, (const xmlChar *) LABEL_ABSOLUTE_MINIMUM,
                            input->variable[i].rangeminabs);
      xml_node_set_float (child, (const xmlChar *) LABEL_MAXIMUM, input->variable[i].rangemax);
      if (input->variable[i].rangemaxabs != G_MAXDOUBLE)
        xml_node_set_float (child, (const xmlChar *) LABEL_ABSOLUTE_MAXIMUM,
                            input->variable[i].rangemaxabs);
      if (input->variable[i].precision != DEFAULT_PRECISION)
        xml_node_set_uint (child, (const xmlChar *) LABEL_PRECISION, input->variable[i].precision);
      if (input->algorithm == ALGORITHM_SWEEP)
        xml_node_set_uint (child, (const xmlChar *) LABEL_NSWEEPS, input->variable[i].nsweeps);
      else if (input->algorithm == ALGORITHM_GENETIC)
        xml_node_set_uint (child, (const xmlChar *) LABEL_NBITS, input->variable[i].nbits);
      if (input->nsteps)
        xml_node_set_float (child, (const xmlChar *) LABEL_STEP, input->variable[i].step);
    }

  // Saving the error norm
  switch (input->norm)
    {
    case ERROR_NORM_MAXIMUM:
      xmlSetProp (node, (const xmlChar *) LABEL_NORM, (const xmlChar *) LABEL_MAXIMUM);
      break;
    case ERROR_NORM_P:
      xmlSetProp (node, (const xmlChar *) LABEL_NORM, (const xmlChar *) LABEL_P);
      xml_node_set_float (node, (const xmlChar *) LABEL_P, input->p);
      break;
    case ERROR_NORM_TAXICAB:
      xmlSetProp (node, (const xmlChar *) LABEL_NORM, (const xmlChar *) LABEL_TAXICAB);
    }

#if DEBUG_INTERFACE
  fprintf (stderr, "input_save: end\n");
#endif
}

/**
 * \fn void input_save_json (JsonGenerator * generator)
 * \brief Function to save the input file in JSON format.
 * \param generator
 * \brief JsonGenerator struct.
 */
void
input_save_json (JsonGenerator * generator)
{
  unsigned int i, j;
  char *buffer;
  JsonNode *node, *child;
  JsonObject *object, *object2;
  JsonArray *array;
  GFile *file, *file2;

#if DEBUG_INTERFACE
  fprintf (stderr, "input_save_json: start\n");
#endif

  // Setting root JSON node
  node = json_node_alloc ();
  object = json_object_new ();
  json_node_init_object (node, object);
  json_generator_set_root (generator, node);

  // Adding properties to the root JSON node
  if (strcmp (input->result, result_name))
    json_object_set_string_member (object, LABEL_RESULT_FILE, input->result);
  if (strcmp (input->variables, variables_name))
    json_object_set_string_member (object, LABEL_VARIABLES_FILE,
		                           input->variables);
  file = g_file_new_for_path (input->directory);
  file2 = g_file_new_for_path (input->simulator);
  buffer = g_file_get_relative_path (file, file2);
  g_object_unref (file2);
  json_object_set_string_member (object, LABEL_SIMULATOR, buffer);
  g_free (buffer);
  if (input->evaluator)
    {
      file2 = g_file_new_for_path (input->evaluator);
      buffer = g_file_get_relative_path (file, file2);
      g_object_unref (file2);
      if (strlen (buffer))
        json_object_set_string_member (object, LABEL_EVALUATOR, buffer);
      g_free (buffer);
    }
  if (input->seed != DEFAULT_RANDOM_SEED)
    json_object_set_uint (object, LABEL_SEED, input->seed);

  // Setting the algorithm
  buffer = (char *) g_slice_alloc (64);
  switch (input->algorithm)
    {
    case ALGORITHM_MONTE_CARLO:
      json_object_set_string_member (object, LABEL_ALGORITHM,
			                         LABEL_MONTE_CARLO);
      snprintf (buffer, 64, "%u", input->nsimulations);
      json_object_set_string_member (object, LABEL_NSIMULATIONS, buffer);
      snprintf (buffer, 64, "%u", input->niterations);
      json_object_set_string_member (object, LABEL_NITERATIONS, buffer);
      snprintf (buffer, 64, "%.3lg", input->tolerance);
      json_object_set_string_member (object, LABEL_TOLERANCE, buffer);
      snprintf (buffer, 64, "%u", input->nbest);
      json_object_set_string_member (object, LABEL_NBEST, buffer);
      input_save_direction_json (node);
      break;
    case ALGORITHM_SWEEP:
      json_object_set_string_member (object, LABEL_ALGORITHM, LABEL_SWEEP);
      snprintf (buffer, 64, "%u", input->niterations);
      json_object_set_string_member (object, LABEL_NITERATIONS, buffer);
      snprintf (buffer, 64, "%.3lg", input->tolerance);
      json_object_set_string_member (object, LABEL_TOLERANCE, buffer);
      snprintf (buffer, 64, "%u", input->nbest);
      json_object_set_string_member (object, LABEL_NBEST, buffer);
      input_save_direction_json (node);
      break;
    default:
      json_object_set_string_member (object, LABEL_ALGORITHM, LABEL_GENETIC);
      snprintf (buffer, 64, "%u", input->nsimulations);
      json_object_set_string_member (object, LABEL_NPOPULATION, buffer);
      snprintf (buffer, 64, "%u", input->niterations);
      json_object_set_string_member (object, LABEL_NGENERATIONS, buffer);
      snprintf (buffer, 64, "%.3lg", input->mutation_ratio);
      json_object_set_string_member (object, LABEL_MUTATION, buffer);
      snprintf (buffer, 64, "%.3lg", input->reproduction_ratio);
      json_object_set_string_member (object, LABEL_REPRODUCTION, buffer);
      snprintf (buffer, 64, "%.3lg", input->adaptation_ratio);
      json_object_set_string_member (object, LABEL_ADAPTATION, buffer);
      break;
    }
  g_slice_free1 (64, buffer);
  if (input->threshold != 0.)
    json_object_set_float (object, LABEL_THRESHOLD, input->threshold);

  // Setting the experimental data
  array = json_array_new ();
  for (i = 0; i < input->nexperiments; ++i)
    {
	  child = json_node_alloc ();
	  object2 = json_object_new ();
      json_object_set_string_member (object2, LABEL_NAME,
			                         input->experiment[i].name);
      if (input->experiment[i].weight != 1.)
        json_object_set_float (object2, LABEL_WEIGHT,
				               input->experiment[i].weight);
      for (j = 0; j < input->experiment->ninputs; ++j)
        json_object_set_string_member (object2, template[j],
                                       input->experiment[i].template[j]);
	  json_node_set_object (child, object2);
	  json_array_add_element (array, child);
    }
  json_object_set_array_member (object, LABEL_EXPERIMENTS, array);

  // Setting the variables data
  array = json_array_new ();
  for (i = 0; i < input->nvariables; ++i)
    {
	  child = json_node_alloc ();
	  object2 = json_object_new ();
      json_object_set_string_member (object2, LABEL_NAME,
			                         input->variable[i].name);
      json_object_set_float (object2, LABEL_MINIMUM,
			                 input->variable[i].rangemin);
      if (input->variable[i].rangeminabs != -G_MAXDOUBLE)
        json_object_set_float (object2, LABEL_ABSOLUTE_MINIMUM,
                               input->variable[i].rangeminabs);
      json_object_set_float (object2, LABEL_MAXIMUM,
			                 input->variable[i].rangemax);
      if (input->variable[i].rangemaxabs != G_MAXDOUBLE)
        json_object_set_float (object2, LABEL_ABSOLUTE_MAXIMUM,
                               input->variable[i].rangemaxabs);
      if (input->variable[i].precision != DEFAULT_PRECISION)
        json_object_set_uint (object2, LABEL_PRECISION,
				              input->variable[i].precision);
      if (input->algorithm == ALGORITHM_SWEEP)
        json_object_set_uint (object2, LABEL_NSWEEPS,
				              input->variable[i].nsweeps);
      else if (input->algorithm == ALGORITHM_GENETIC)
        json_object_set_uint (object2, LABEL_NBITS, input->variable[i].nbits);
      if (input->nsteps)
        json_object_set_float (object, LABEL_STEP, input->variable[i].step);
	  json_node_set_object (child, object2);
	  json_array_add_element (array, child);
    }
  json_object_set_array_member (object, LABEL_VARIABLES, array);

  // Saving the error norm
  switch (input->norm)
    {
    case ERROR_NORM_MAXIMUM:
      json_object_set_string_member (object, LABEL_NORM, LABEL_MAXIMUM);
      break;
    case ERROR_NORM_P:
      json_object_set_string_member (object, LABEL_NORM, LABEL_P);
      json_object_set_float (object, LABEL_P, input->p);
      break;
    case ERROR_NORM_TAXICAB:
      json_object_set_string_member (object, LABEL_NORM, LABEL_TAXICAB);
    }

#if DEBUG_INTERFACE
  fprintf (stderr, "input_save_json: end\n");
#endif
}

/**
 * \fn void input_save (char *filename)
 * \brief Function to save the input file.
 * \param filename
 * \brief Input file name.
 */
void
input_save (char *filename)
{
  xmlDoc *doc;
  JsonGenerator *generator;

#if DEBUG_INTERFACE
  fprintf (stderr, "input_save: start\n");
#endif

  // Getting the input file directory
  input->name = g_path_get_basename (filename);
  input->directory = g_path_get_dirname (filename);

  if (input->type == INPUT_TYPE_XML)
	{
      // Opening the input file
      doc = xmlNewDoc ((const xmlChar *) "1.0");
	  input_save_xml (doc);

      // Saving the XML file
      xmlSaveFormatFile (filename, doc, 1);

      // Freeing memory
      xmlFreeDoc (doc);
	}
  else
	{
      // Opening the input file
      generator = json_generator_new ();
	  json_generator_set_pretty (generator, TRUE);
	  input_save_json (generator);

      // Saving the JSON file
      json_generator_to_file (generator, filename, NULL);

      // Freeing memory
	  g_object_unref (generator);
	}

#if DEBUG_INTERFACE
  fprintf (stderr, "input_save: end\n");
#endif
}

/**
 * \fn void options_new ()
 * \brief Function to open the options dialog.
 */
void
options_new ()
{
#if DEBUG_INTERFACE
  fprintf (stderr, "options_new: start\n");
#endif
  options->label_seed = (GtkLabel *)
    gtk_label_new (gettext ("Pseudo-random numbers generator seed"));
  options->spin_seed = (GtkSpinButton *)
    gtk_spin_button_new_with_range (0., (gdouble) G_MAXULONG, 1.);
  gtk_widget_set_tooltip_text
    (GTK_WIDGET (options->spin_seed),
     gettext ("Seed to init the pseudo-random numbers generator"));
  gtk_spin_button_set_value (options->spin_seed, (gdouble) input->seed);
  options->label_threads = (GtkLabel *)
    gtk_label_new (gettext ("Threads number for the stochastic algorithm"));
  options->spin_threads
    = (GtkSpinButton *) gtk_spin_button_new_with_range (1., 64., 1.);
  gtk_widget_set_tooltip_text
    (GTK_WIDGET (options->spin_threads),
     gettext ("Number of threads to perform the calibration/optimization for "
              "the stochastic algorithm"));
  gtk_spin_button_set_value (options->spin_threads, (gdouble) nthreads);
  options->label_direction = (GtkLabel *)
    gtk_label_new (gettext ("Threads number for the direction search method"));
  options->spin_direction
    = (GtkSpinButton *) gtk_spin_button_new_with_range (1., 64., 1.);
  gtk_widget_set_tooltip_text
    (GTK_WIDGET (options->spin_direction),
     gettext ("Number of threads to perform the calibration/optimization for "
              "the direction search method"));
  gtk_spin_button_set_value (options->spin_direction,
                             (gdouble) nthreads_direction);
  options->grid = (GtkGrid *) gtk_grid_new ();
  gtk_grid_attach (options->grid, GTK_WIDGET (options->label_seed), 0, 0, 1, 1);
  gtk_grid_attach (options->grid, GTK_WIDGET (options->spin_seed), 1, 0, 1, 1);
  gtk_grid_attach (options->grid, GTK_WIDGET (options->label_threads),
                   0, 1, 1, 1);
  gtk_grid_attach (options->grid, GTK_WIDGET (options->spin_threads),
                   1, 1, 1, 1);
  gtk_grid_attach (options->grid, GTK_WIDGET (options->label_direction),
                   0, 2, 1, 1);
  gtk_grid_attach (options->grid, GTK_WIDGET (options->spin_direction),
                   1, 2, 1, 1);
  gtk_widget_show_all (GTK_WIDGET (options->grid));
  options->dialog = (GtkDialog *)
    gtk_dialog_new_with_buttons (gettext ("Options"),
                                 window->window,
                                 GTK_DIALOG_MODAL,
                                 gettext ("_OK"), GTK_RESPONSE_OK,
                                 gettext ("_Cancel"), GTK_RESPONSE_CANCEL,
                                 NULL);
  gtk_container_add
    (GTK_CONTAINER (gtk_dialog_get_content_area (options->dialog)),
     GTK_WIDGET (options->grid));
  if (gtk_dialog_run (options->dialog) == GTK_RESPONSE_OK)
    {
      input->seed
        = (unsigned long int) gtk_spin_button_get_value (options->spin_seed);
      nthreads = gtk_spin_button_get_value_as_int (options->spin_threads);
      nthreads_direction
        = gtk_spin_button_get_value_as_int (options->spin_direction);
    }
  gtk_widget_destroy (GTK_WIDGET (options->dialog));
#if DEBUG_INTERFACE
  fprintf (stderr, "options_new: end\n");
#endif
}

/**
 * \fn void running_new ()
 * \brief Function to open the running dialog.
 */
void
running_new ()
{
#if DEBUG_INTERFACE
  fprintf (stderr, "running_new: start\n");
#endif
  running->label = (GtkLabel *) gtk_label_new (gettext ("Calculating ..."));
  running->spinner = (GtkSpinner *) gtk_spinner_new ();
  running->grid = (GtkGrid *) gtk_grid_new ();
  gtk_grid_attach (running->grid, GTK_WIDGET (running->label), 0, 0, 1, 1);
  gtk_grid_attach (running->grid, GTK_WIDGET (running->spinner), 0, 1, 1, 1);
  running->dialog = (GtkDialog *)
    gtk_dialog_new_with_buttons (gettext ("Calculating"),
                                 window->window, GTK_DIALOG_MODAL, NULL, NULL);
  gtk_container_add
    (GTK_CONTAINER (gtk_dialog_get_content_area (running->dialog)),
     GTK_WIDGET (running->grid));
  gtk_spinner_start (running->spinner);
  gtk_widget_show_all (GTK_WIDGET (running->dialog));
#if DEBUG_INTERFACE
  fprintf (stderr, "running_new: end\n");
#endif
}

/**
 * \fn unsigned int window_get_algorithm ()
 * \brief Function to get the stochastic algorithm number.
 * \return Stochastic algorithm number.
 */
unsigned int
window_get_algorithm ()
{
  unsigned int i;
#if DEBUG_INTERFACE
  fprintf (stderr, "window_get_algorithm: start\n");
#endif
  i = gtk_array_get_active (window->button_algorithm, NALGORITHMS);
#if DEBUG_INTERFACE
  fprintf (stderr, "window_get_algorithm: %u\n", i);
  fprintf (stderr, "window_get_algorithm: end\n");
#endif
  return i;
}

/**
 * \fn unsigned int window_get_direction ()
 * \brief Function to get the direction search method number.
 * \return Direction search method number.
 */
unsigned int
window_get_direction ()
{
  unsigned int i;
#if DEBUG_INTERFACE
  fprintf (stderr, "window_get_direction: start\n");
#endif
  i = gtk_array_get_active (window->button_direction, NDIRECTIONS);
#if DEBUG_INTERFACE
  fprintf (stderr, "window_get_direction: %u\n", i);
  fprintf (stderr, "window_get_direction: end\n");
#endif
  return i;
}

/**
 * \fn unsigned int window_get_norm ()
 * \brief Function to get the norm method number.
 * \return Norm method number.
 */
unsigned int
window_get_norm ()
{
  unsigned int i;
#if DEBUG_INTERFACE
  fprintf (stderr, "window_get_norm: start\n");
#endif
  i = gtk_array_get_active (window->button_norm, NNORMS);
#if DEBUG_INTERFACE
  fprintf (stderr, "window_get_norm: %u\n", i);
  fprintf (stderr, "window_get_norm: end\n");
#endif
  return i;
}

/**
 * \fn void window_save_direction ()
 * \brief Function to save the direction search method data in the input file.
 */
void
window_save_direction ()
{
#if DEBUG_INTERFACE
  fprintf (stderr, "window_save_direction: start\n");
#endif
  if (gtk_toggle_button_get_active
      (GTK_TOGGLE_BUTTON (window->check_direction)))
    {
      input->nsteps = gtk_spin_button_get_value_as_int (window->spin_steps);
      input->relaxation = gtk_spin_button_get_value (window->spin_relaxation);
      switch (window_get_direction ())
        {
        case DIRECTION_METHOD_COORDINATES:
          input->direction = DIRECTION_METHOD_COORDINATES;
          break;
        default:
          input->direction = DIRECTION_METHOD_RANDOM;
          input->nestimates
            = gtk_spin_button_get_value_as_int (window->spin_estimates);
        }
    }
  else
    input->nsteps = 0;
#if DEBUG_INTERFACE
  fprintf (stderr, "window_save_direction: end\n");
#endif
}

/**
 * \fn int window_save ()
 * \brief Function to save the input file.
 * \return 1 on OK, 0 on Cancel.
 */
int
window_save ()
{
  GtkFileChooserDialog *dlg;
  GtkFileFilter *filter1, *filter2;
  char *buffer;

#if DEBUG_INTERFACE
  fprintf (stderr, "window_save: start\n");
#endif

  // Opening the saving dialog
  dlg = (GtkFileChooserDialog *)
    gtk_file_chooser_dialog_new (gettext ("Save file"),
                                 window->window,
                                 GTK_FILE_CHOOSER_ACTION_SAVE,
                                 gettext ("_Cancel"),
                                 GTK_RESPONSE_CANCEL,
                                 gettext ("_OK"), GTK_RESPONSE_OK, NULL);
  gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dlg), TRUE);
  buffer = g_build_filename (input->directory, input->name, NULL);
  gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dlg), buffer);
  g_free (buffer);

  // Adding XML filter
  filter1 = (GtkFileFilter *) gtk_file_filter_new ();
  gtk_file_filter_set_name (filter1, "XML");
  gtk_file_filter_add_pattern (filter1, "*.xml");
  gtk_file_filter_add_pattern (filter1, "*.XML");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dlg), filter1);

  // Adding JSON filter
  filter2 = (GtkFileFilter *) gtk_file_filter_new ();
  gtk_file_filter_set_name (filter2, "JSON");
  gtk_file_filter_add_pattern (filter2, "*.json");
  gtk_file_filter_add_pattern (filter2, "*.JSON");
  gtk_file_filter_add_pattern (filter2, "*.js");
  gtk_file_filter_add_pattern (filter2, "*.JS");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dlg), filter2);

  if (input->type == INPUT_TYPE_XML)
	gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (dlg), filter1);
  else
	gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (dlg), filter2);

  // If OK response then saving
  if (gtk_dialog_run (GTK_DIALOG (dlg)) == GTK_RESPONSE_OK)
    {
      // Setting input file type
      filter1 = gtk_file_chooser_get_filter (GTK_FILE_CHOOSER (dlg));
	  buffer = (char *) gtk_file_filter_get_name (filter1);
	  if (!strcmp (buffer, "XML"))
        input->type = INPUT_TYPE_XML;
	  else
        input->type = INPUT_TYPE_JSON;

      // Adding properties to the root XML node
      input->simulator = gtk_file_chooser_get_filename
        (GTK_FILE_CHOOSER (window->button_simulator));
      if (gtk_toggle_button_get_active
          (GTK_TOGGLE_BUTTON (window->check_evaluator)))
        input->evaluator = gtk_file_chooser_get_filename
          (GTK_FILE_CHOOSER (window->button_evaluator));
      else
        input->evaluator = NULL;
	  if (input->type == INPUT_TYPE_XML)
		{
          input->result
            = (char *) xmlStrdup ((const xmlChar *)
                                  gtk_entry_get_text (window->entry_result));
          input->variables
            = (char *) xmlStrdup ((const xmlChar *)
                                  gtk_entry_get_text (window->entry_variables));
		}
	  else
		{
          input->result = g_strdup (gtk_entry_get_text (window->entry_result));
          input->variables
            = g_strdup (gtk_entry_get_text (window->entry_variables));
		}

      // Setting the algorithm
      switch (window_get_algorithm ())
        {
        case ALGORITHM_MONTE_CARLO:
          input->algorithm = ALGORITHM_MONTE_CARLO;
          input->nsimulations
            = gtk_spin_button_get_value_as_int (window->spin_simulations);
          input->niterations
            = gtk_spin_button_get_value_as_int (window->spin_iterations);
          input->tolerance = gtk_spin_button_get_value (window->spin_tolerance);
          input->nbest = gtk_spin_button_get_value_as_int (window->spin_bests);
          window_save_direction ();
          break;
        case ALGORITHM_SWEEP:
          input->algorithm = ALGORITHM_SWEEP;
          input->niterations
            = gtk_spin_button_get_value_as_int (window->spin_iterations);
          input->tolerance = gtk_spin_button_get_value (window->spin_tolerance);
          input->nbest = gtk_spin_button_get_value_as_int (window->spin_bests);
          window_save_direction ();
          break;
        default:
          input->algorithm = ALGORITHM_GENETIC;
          input->nsimulations
            = gtk_spin_button_get_value_as_int (window->spin_population);
          input->niterations
            = gtk_spin_button_get_value_as_int (window->spin_generations);
          input->mutation_ratio
            = gtk_spin_button_get_value (window->spin_mutation);
          input->reproduction_ratio
            = gtk_spin_button_get_value (window->spin_reproduction);
          input->adaptation_ratio
            = gtk_spin_button_get_value (window->spin_adaptation);
          break;
        }
      input->norm = window_get_norm ();
      input->p = gtk_spin_button_get_value (window->spin_p);
      input->threshold = gtk_spin_button_get_value (window->spin_threshold);

      // Saving the XML file
      buffer = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dlg));
      input_save (buffer);

      // Closing and freeing memory
      g_free (buffer);
      gtk_widget_destroy (GTK_WIDGET (dlg));
#if DEBUG_INTERFACE
      fprintf (stderr, "window_save: end\n");
#endif
      return 1;
    }

  // Closing and freeing memory
  gtk_widget_destroy (GTK_WIDGET (dlg));
#if DEBUG_INTERFACE
  fprintf (stderr, "window_save: end\n");
#endif
  return 0;
}

/**
 * \fn void window_run ()
 * \brief Function to run a optimization.
 */
void
window_run ()
{
  unsigned int i;
  char *msg, *msg2, buffer[64], buffer2[64];
#if DEBUG_INTERFACE
  fprintf (stderr, "window_run: start\n");
#endif
  if (!window_save ())
    {
#if DEBUG_INTERFACE
      fprintf (stderr, "window_run: end\n");
#endif
      return;
    }
  running_new ();
  while (gtk_events_pending ())
    gtk_main_iteration ();
  optimize_open ();
#if DEBUG_INTERFACE
  fprintf (stderr, "window_run: closing running dialog\n");
#endif
  gtk_spinner_stop (running->spinner);
  gtk_widget_destroy (GTK_WIDGET (running->dialog));
#if DEBUG_INTERFACE
  fprintf (stderr, "window_run: displaying results\n");
#endif
  snprintf (buffer, 64, "error = %.15le\n", optimize->error_old[0]);
  msg2 = g_strdup (buffer);
  for (i = 0; i < optimize->nvariables; ++i, msg2 = msg)
    {
      snprintf (buffer, 64, "%s = %s\n",
                input->variable[i].name, format[input->variable[i].precision]);
      snprintf (buffer2, 64, buffer, optimize->value_old[i]);
      msg = g_strconcat (msg2, buffer2, NULL);
      g_free (msg2);
    }
  snprintf (buffer, 64, "%s = %.6lg s", gettext ("Calculation time"),
            optimize->calculation_time);
  msg = g_strconcat (msg2, buffer, NULL);
  g_free (msg2);
  show_message (gettext ("Best result"), msg, INFO_TYPE);
  g_free (msg);
#if DEBUG_INTERFACE
  fprintf (stderr, "window_run: freeing memory\n");
#endif
  optimize_free ();
#if DEBUG_INTERFACE
  fprintf (stderr, "window_run: end\n");
#endif
}

/**
 * \fn void window_help ()
 * \brief Function to show a help dialog.
 */
void
window_help ()
{
  char *buffer, *buffer2;
#if DEBUG_INTERFACE
  fprintf (stderr, "window_help: start\n");
#endif
  buffer2 = g_build_filename (window->application_directory, "..", "manuals",
                              gettext ("user-manual.pdf"), NULL);
  buffer = g_filename_to_uri (buffer2, NULL, NULL);
  g_free (buffer2);
  gtk_show_uri (NULL, buffer, GDK_CURRENT_TIME, NULL);
#if DEBUG_INTERFACE
  fprintf (stderr, "window_help: uri=%s\n", buffer);
#endif
  g_free (buffer);
#if DEBUG_INTERFACE
  fprintf (stderr, "window_help: end\n");
#endif
}

/**
 * \fn void window_about ()
 * \brief Function to show an about dialog.
 */
void
window_about ()
{
  static const gchar *authors[] = {
    "Javier Burguete Tolosa <jburguete@eead.csic.es>",
    "Borja Latorre Garcés <borja.latorre@csic.es>",
    NULL
  };
#if DEBUG_INTERFACE
  fprintf (stderr, "window_about: start\n");
#endif
  gtk_show_about_dialog
    (window->window,
     "program_name", "MPCOTool",
     "comments",
     gettext ("The Multi-Purposes Calibration and Optimization Tool.\n"
              "A software to perform calibrations or optimizations of "
              "empirical parameters"),
     "authors", authors,
     "translator-credits", "Javier Burguete Tolosa <jburguete@eead.csic.es>",
     "version", "3.1.0",
     "copyright", "Copyright 2012-2016 Javier Burguete Tolosa",
     "logo", window->logo,
     "website", "https://github.com/jburguete/mpcotool",
     "license-type", GTK_LICENSE_BSD, NULL);
#if DEBUG_INTERFACE
  fprintf (stderr, "window_about: end\n");
#endif
}

/**
 * \fn void window_update_direction ()
 * \brief Function to update direction search method widgets view in the main
 *   window.
 */
void
window_update_direction ()
{
#if DEBUG_INTERFACE
  fprintf (stderr, "window_update_direction: start\n");
#endif
  gtk_widget_show (GTK_WIDGET (window->check_direction));
  if (gtk_toggle_button_get_active
      (GTK_TOGGLE_BUTTON (window->check_direction)))
    {
      gtk_widget_show (GTK_WIDGET (window->grid_direction));
      gtk_widget_show (GTK_WIDGET (window->label_step));
      gtk_widget_show (GTK_WIDGET (window->spin_step));
    }
  switch (window_get_direction ())
    {
    case DIRECTION_METHOD_COORDINATES:
      gtk_widget_hide (GTK_WIDGET (window->label_estimates));
      gtk_widget_hide (GTK_WIDGET (window->spin_estimates));
      break;
    default:
      gtk_widget_show (GTK_WIDGET (window->label_estimates));
      gtk_widget_show (GTK_WIDGET (window->spin_estimates));
    }
#if DEBUG_INTERFACE
  fprintf (stderr, "window_update_direction: end\n");
#endif
}

/**
 * \fn void window_update ()
 * \brief Function to update the main window view.
 */
void
window_update ()
{
  unsigned int i;
#if DEBUG_INTERFACE
  fprintf (stderr, "window_update: start\n");
#endif
  gtk_widget_set_sensitive
    (GTK_WIDGET (window->button_evaluator),
     gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
                                   (window->check_evaluator)));
  gtk_widget_hide (GTK_WIDGET (window->label_simulations));
  gtk_widget_hide (GTK_WIDGET (window->spin_simulations));
  gtk_widget_hide (GTK_WIDGET (window->label_iterations));
  gtk_widget_hide (GTK_WIDGET (window->spin_iterations));
  gtk_widget_hide (GTK_WIDGET (window->label_tolerance));
  gtk_widget_hide (GTK_WIDGET (window->spin_tolerance));
  gtk_widget_hide (GTK_WIDGET (window->label_bests));
  gtk_widget_hide (GTK_WIDGET (window->spin_bests));
  gtk_widget_hide (GTK_WIDGET (window->label_population));
  gtk_widget_hide (GTK_WIDGET (window->spin_population));
  gtk_widget_hide (GTK_WIDGET (window->label_generations));
  gtk_widget_hide (GTK_WIDGET (window->spin_generations));
  gtk_widget_hide (GTK_WIDGET (window->label_mutation));
  gtk_widget_hide (GTK_WIDGET (window->spin_mutation));
  gtk_widget_hide (GTK_WIDGET (window->label_reproduction));
  gtk_widget_hide (GTK_WIDGET (window->spin_reproduction));
  gtk_widget_hide (GTK_WIDGET (window->label_adaptation));
  gtk_widget_hide (GTK_WIDGET (window->spin_adaptation));
  gtk_widget_hide (GTK_WIDGET (window->label_sweeps));
  gtk_widget_hide (GTK_WIDGET (window->spin_sweeps));
  gtk_widget_hide (GTK_WIDGET (window->label_bits));
  gtk_widget_hide (GTK_WIDGET (window->spin_bits));
  gtk_widget_hide (GTK_WIDGET (window->check_direction));
  gtk_widget_hide (GTK_WIDGET (window->grid_direction));
  gtk_widget_hide (GTK_WIDGET (window->label_step));
  gtk_widget_hide (GTK_WIDGET (window->spin_step));
  gtk_widget_hide (GTK_WIDGET (window->label_p));
  gtk_widget_hide (GTK_WIDGET (window->spin_p));
  i = gtk_spin_button_get_value_as_int (window->spin_iterations);
  switch (window_get_algorithm ())
    {
    case ALGORITHM_MONTE_CARLO:
      gtk_widget_show (GTK_WIDGET (window->label_simulations));
      gtk_widget_show (GTK_WIDGET (window->spin_simulations));
      gtk_widget_show (GTK_WIDGET (window->label_iterations));
      gtk_widget_show (GTK_WIDGET (window->spin_iterations));
      if (i > 1)
        {
          gtk_widget_show (GTK_WIDGET (window->label_tolerance));
          gtk_widget_show (GTK_WIDGET (window->spin_tolerance));
          gtk_widget_show (GTK_WIDGET (window->label_bests));
          gtk_widget_show (GTK_WIDGET (window->spin_bests));
        }
      window_update_direction ();
      break;
    case ALGORITHM_SWEEP:
      gtk_widget_show (GTK_WIDGET (window->label_iterations));
      gtk_widget_show (GTK_WIDGET (window->spin_iterations));
      if (i > 1)
        {
          gtk_widget_show (GTK_WIDGET (window->label_tolerance));
          gtk_widget_show (GTK_WIDGET (window->spin_tolerance));
          gtk_widget_show (GTK_WIDGET (window->label_bests));
          gtk_widget_show (GTK_WIDGET (window->spin_bests));
        }
      gtk_widget_show (GTK_WIDGET (window->label_sweeps));
      gtk_widget_show (GTK_WIDGET (window->spin_sweeps));
      gtk_widget_show (GTK_WIDGET (window->check_direction));
      window_update_direction ();
      break;
    default:
      gtk_widget_show (GTK_WIDGET (window->label_population));
      gtk_widget_show (GTK_WIDGET (window->spin_population));
      gtk_widget_show (GTK_WIDGET (window->label_generations));
      gtk_widget_show (GTK_WIDGET (window->spin_generations));
      gtk_widget_show (GTK_WIDGET (window->label_mutation));
      gtk_widget_show (GTK_WIDGET (window->spin_mutation));
      gtk_widget_show (GTK_WIDGET (window->label_reproduction));
      gtk_widget_show (GTK_WIDGET (window->spin_reproduction));
      gtk_widget_show (GTK_WIDGET (window->label_adaptation));
      gtk_widget_show (GTK_WIDGET (window->spin_adaptation));
      gtk_widget_show (GTK_WIDGET (window->label_bits));
      gtk_widget_show (GTK_WIDGET (window->spin_bits));
    }
  gtk_widget_set_sensitive
    (GTK_WIDGET (window->button_remove_experiment), input->nexperiments > 1);
  gtk_widget_set_sensitive
    (GTK_WIDGET (window->button_remove_variable), input->nvariables > 1);
  for (i = 0; i < input->experiment->ninputs; ++i)
    {
      gtk_widget_show (GTK_WIDGET (window->check_template[i]));
      gtk_widget_show (GTK_WIDGET (window->button_template[i]));
      gtk_widget_set_sensitive (GTK_WIDGET (window->check_template[i]), 0);
      gtk_widget_set_sensitive (GTK_WIDGET (window->button_template[i]), 1);
      g_signal_handler_block
        (window->check_template[i], window->id_template[i]);
      g_signal_handler_block (window->button_template[i], window->id_input[i]);
      gtk_toggle_button_set_active
        (GTK_TOGGLE_BUTTON (window->check_template[i]), 1);
      g_signal_handler_unblock
        (window->button_template[i], window->id_input[i]);
      g_signal_handler_unblock
        (window->check_template[i], window->id_template[i]);
    }
  if (i > 0)
    {
      gtk_widget_set_sensitive (GTK_WIDGET (window->check_template[i - 1]), 1);
      gtk_widget_set_sensitive
        (GTK_WIDGET (window->button_template[i - 1]),
         gtk_toggle_button_get_active
         GTK_TOGGLE_BUTTON (window->check_template[i - 1]));
    }
  if (i < MAX_NINPUTS)
    {
      gtk_widget_show (GTK_WIDGET (window->check_template[i]));
      gtk_widget_show (GTK_WIDGET (window->button_template[i]));
      gtk_widget_set_sensitive (GTK_WIDGET (window->check_template[i]), 1);
      gtk_widget_set_sensitive
        (GTK_WIDGET (window->button_template[i]),
         gtk_toggle_button_get_active
         GTK_TOGGLE_BUTTON (window->check_template[i]));
      g_signal_handler_block
        (window->check_template[i], window->id_template[i]);
      g_signal_handler_block (window->button_template[i], window->id_input[i]);
      gtk_toggle_button_set_active
        (GTK_TOGGLE_BUTTON (window->check_template[i]), 0);
      g_signal_handler_unblock
        (window->button_template[i], window->id_input[i]);
      g_signal_handler_unblock
        (window->check_template[i], window->id_template[i]);
    }
  while (++i < MAX_NINPUTS)
    {
      gtk_widget_hide (GTK_WIDGET (window->check_template[i]));
      gtk_widget_hide (GTK_WIDGET (window->button_template[i]));
    }
  gtk_widget_set_sensitive
    (GTK_WIDGET (window->spin_minabs),
     gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (window->check_minabs)));
  gtk_widget_set_sensitive
    (GTK_WIDGET (window->spin_maxabs),
     gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (window->check_maxabs)));
  if (window_get_norm () == ERROR_NORM_P)
    {
      gtk_widget_show (GTK_WIDGET (window->label_p));
      gtk_widget_show (GTK_WIDGET (window->spin_p));
    }
#if DEBUG_INTERFACE
  fprintf (stderr, "window_update: end\n");
#endif
}

/**
 * \fn void window_set_algorithm ()
 * \brief Function to avoid memory errors changing the algorithm.
 */
void
window_set_algorithm ()
{
  int i;
#if DEBUG_INTERFACE
  fprintf (stderr, "window_set_algorithm: start\n");
#endif
  i = window_get_algorithm ();
  switch (i)
    {
    case ALGORITHM_SWEEP:
      i = gtk_combo_box_get_active (GTK_COMBO_BOX (window->combo_variable));
      if (i < 0)
        i = 0;
      gtk_spin_button_set_value (window->spin_sweeps,
                                 (gdouble) input->variable[i].nsweeps);
      break;
    case ALGORITHM_GENETIC:
      i = gtk_combo_box_get_active (GTK_COMBO_BOX (window->combo_variable));
      if (i < 0)
        i = 0;
      gtk_spin_button_set_value (window->spin_bits,
                                 (gdouble) input->variable[i].nbits);
    }
  window_update ();
#if DEBUG_INTERFACE
  fprintf (stderr, "window_set_algorithm: end\n");
#endif
}

/**
 * \fn void window_set_experiment ()
 * \brief Function to set the experiment data in the main window.
 */
void
window_set_experiment ()
{
  unsigned int i, j;
  char *buffer1, *buffer2;
#if DEBUG_INTERFACE
  fprintf (stderr, "window_set_experiment: start\n");
#endif
  i = gtk_combo_box_get_active (GTK_COMBO_BOX (window->combo_experiment));
  gtk_spin_button_set_value (window->spin_weight, input->experiment[i].weight);
  buffer1 = gtk_combo_box_text_get_active_text (window->combo_experiment);
  buffer2 = g_build_filename (input->directory, buffer1, NULL);
  g_free (buffer1);
  g_signal_handler_block
    (window->button_experiment, window->id_experiment_name);
  gtk_file_chooser_set_filename
    (GTK_FILE_CHOOSER (window->button_experiment), buffer2);
  g_signal_handler_unblock
    (window->button_experiment, window->id_experiment_name);
  g_free (buffer2);
  for (j = 0; j < input->experiment->ninputs; ++j)
    {
      g_signal_handler_block (window->button_template[j], window->id_input[j]);
      buffer2 = g_build_filename (input->directory,
                                  input->experiment[i].template[j], NULL);
      gtk_file_chooser_set_filename
        (GTK_FILE_CHOOSER (window->button_template[j]), buffer2);
      g_free (buffer2);
      g_signal_handler_unblock
        (window->button_template[j], window->id_input[j]);
    }
#if DEBUG_INTERFACE
  fprintf (stderr, "window_set_experiment: end\n");
#endif
}

/**
 * \fn void window_remove_experiment ()
 * \brief Function to remove an experiment in the main window.
 */
void
window_remove_experiment ()
{
  unsigned int i, j;
#if DEBUG_INTERFACE
  fprintf (stderr, "window_remove_experiment: start\n");
#endif
  i = gtk_combo_box_get_active (GTK_COMBO_BOX (window->combo_experiment));
  g_signal_handler_block (window->combo_experiment, window->id_experiment);
  gtk_combo_box_text_remove (window->combo_experiment, i);
  g_signal_handler_unblock (window->combo_experiment, window->id_experiment);
  experiment_free (input->experiment + i, input->type);
  --input->nexperiments;
  for (j = i; j < input->nexperiments; ++j)
    memcpy (input->experiment + j, input->experiment + j + 1,
            sizeof (Experiment));
  j = input->nexperiments - 1;
  if (i > j)
    i = j;
  for (j = 0; j < input->experiment->ninputs; ++j)
    g_signal_handler_block (window->button_template[j], window->id_input[j]);
  g_signal_handler_block
    (window->button_experiment, window->id_experiment_name);
  gtk_combo_box_set_active (GTK_COMBO_BOX (window->combo_experiment), i);
  g_signal_handler_unblock
    (window->button_experiment, window->id_experiment_name);
  for (j = 0; j < input->experiment->ninputs; ++j)
    g_signal_handler_unblock (window->button_template[j], window->id_input[j]);
  window_update ();
#if DEBUG_INTERFACE
  fprintf (stderr, "window_remove_experiment: end\n");
#endif
}

/**
 * \fn void window_add_experiment ()
 * \brief Function to add an experiment in the main window.
 */
void
window_add_experiment ()
{
  unsigned int i, j;
#if DEBUG_INTERFACE
  fprintf (stderr, "window_add_experiment: start\n");
#endif
  i = gtk_combo_box_get_active (GTK_COMBO_BOX (window->combo_experiment));
  g_signal_handler_block (window->combo_experiment, window->id_experiment);
  gtk_combo_box_text_insert_text
    (window->combo_experiment, i, input->experiment[i].name);
  g_signal_handler_unblock (window->combo_experiment, window->id_experiment);
  input->experiment = (Experiment *) g_realloc
    (input->experiment, (input->nexperiments + 1) * sizeof (Experiment));
  for (j = input->nexperiments - 1; j > i; --j)
    memcpy (input->experiment + j + 1, input->experiment + j,
            sizeof (Experiment));
  input->experiment[j + 1].weight = input->experiment[j].weight;
  input->experiment[j + 1].ninputs = input->experiment[j].ninputs;
  if (input->type == INPUT_TYPE_XML)
    {
      input->experiment[j + 1].name
        = (char *) xmlStrdup ((xmlChar *) input->experiment[j].name);
      for (j = 0; j < input->experiment->ninputs; ++j)
        input->experiment[i + 1].template[j]
          = (char *) xmlStrdup ((xmlChar *) input->experiment[i].template[j]);
	}
  else
    {
      input->experiment[j + 1].name = g_strdup (input->experiment[j].name);
      for (j = 0; j < input->experiment->ninputs; ++j)
        input->experiment[i + 1].template[j]
          = g_strdup (input->experiment[i].template[j]);
	}
  ++input->nexperiments;
  for (j = 0; j < input->experiment->ninputs; ++j)
    g_signal_handler_block (window->button_template[j], window->id_input[j]);
  g_signal_handler_block
    (window->button_experiment, window->id_experiment_name);
  gtk_combo_box_set_active (GTK_COMBO_BOX (window->combo_experiment), i + 1);
  g_signal_handler_unblock
    (window->button_experiment, window->id_experiment_name);
  for (j = 0; j < input->experiment->ninputs; ++j)
    g_signal_handler_unblock (window->button_template[j], window->id_input[j]);
  window_update ();
#if DEBUG_INTERFACE
  fprintf (stderr, "window_add_experiment: end\n");
#endif
}

/**
 * \fn void window_name_experiment ()
 * \brief Function to set the experiment name in the main window.
 */
void
window_name_experiment ()
{
  unsigned int i;
  char *buffer;
  GFile *file1, *file2;
#if DEBUG_INTERFACE
  fprintf (stderr, "window_name_experiment: start\n");
#endif
  i = gtk_combo_box_get_active (GTK_COMBO_BOX (window->combo_experiment));
  file1
    = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (window->button_experiment));
  file2 = g_file_new_for_path (input->directory);
  buffer = g_file_get_relative_path (file2, file1);
  g_signal_handler_block (window->combo_experiment, window->id_experiment);
  gtk_combo_box_text_remove (window->combo_experiment, i);
  gtk_combo_box_text_insert_text (window->combo_experiment, i, buffer);
  gtk_combo_box_set_active (GTK_COMBO_BOX (window->combo_experiment), i);
  g_signal_handler_unblock (window->combo_experiment, window->id_experiment);
  g_free (buffer);
  g_object_unref (file2);
  g_object_unref (file1);
#if DEBUG_INTERFACE
  fprintf (stderr, "window_name_experiment: end\n");
#endif
}

/**
 * \fn void window_weight_experiment ()
 * \brief Function to update the experiment weight in the main window.
 */
void
window_weight_experiment ()
{
  unsigned int i;
#if DEBUG_INTERFACE
  fprintf (stderr, "window_weight_experiment: start\n");
#endif
  i = gtk_combo_box_get_active (GTK_COMBO_BOX (window->combo_experiment));
  input->experiment[i].weight = gtk_spin_button_get_value (window->spin_weight);
#if DEBUG_INTERFACE
  fprintf (stderr, "window_weight_experiment: end\n");
#endif
}

/**
 * \fn void window_inputs_experiment ()
 * \brief Function to update the experiment input templates number in the main
 *   window.
 */
void
window_inputs_experiment ()
{
  unsigned int j;
#if DEBUG_INTERFACE
  fprintf (stderr, "window_inputs_experiment: start\n");
#endif
  j = input->experiment->ninputs - 1;
  if (j
      && !gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
                                        (window->check_template[j])))
    --input->experiment->ninputs;
  if (input->experiment->ninputs < MAX_NINPUTS
      && gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
                                       (window->check_template[j])))
    ++input->experiment->ninputs;
  window_update ();
#if DEBUG_INTERFACE
  fprintf (stderr, "window_inputs_experiment: end\n");
#endif
}

/**
 * \fn void window_template_experiment (void *data)
 * \brief Function to update the experiment i-th input template in the main
 *   window.
 * \param data
 * \brief Callback data (i-th input template).
 */
void
window_template_experiment (void *data)
{
  unsigned int i, j;
  char *buffer;
  GFile *file1, *file2;
#if DEBUG_INTERFACE
  fprintf (stderr, "window_template_experiment: start\n");
#endif
  i = (size_t) data;
  j = gtk_combo_box_get_active (GTK_COMBO_BOX (window->combo_experiment));
  file1
    = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (window->button_template[i]));
  file2 = g_file_new_for_path (input->directory);
  buffer = g_file_get_relative_path (file2, file1);
  if (input->type == INPUT_TYPE_XML)
    input->experiment[j].template[i] = (char *) xmlStrdup ((xmlChar *) buffer);
  else
    input->experiment[j].template[i] = g_strdup (buffer);
  g_free (buffer);
  g_object_unref (file2);
  g_object_unref (file1);
#if DEBUG_INTERFACE
  fprintf (stderr, "window_template_experiment: end\n");
#endif
}

/**
 * \fn void window_set_variable ()
 * \brief Function to set the variable data in the main window.
 */
void
window_set_variable ()
{
  unsigned int i;
#if DEBUG_INTERFACE
  fprintf (stderr, "window_set_variable: start\n");
#endif
  i = gtk_combo_box_get_active (GTK_COMBO_BOX (window->combo_variable));
  g_signal_handler_block (window->entry_variable, window->id_variable_label);
  gtk_entry_set_text (window->entry_variable, input->variable[i].name);
  g_signal_handler_unblock (window->entry_variable, window->id_variable_label);
  gtk_spin_button_set_value (window->spin_min, input->variable[i].rangemin);
  gtk_spin_button_set_value (window->spin_max, input->variable[i].rangemax);
  if (input->variable[i].rangeminabs != -G_MAXDOUBLE)
    {
      gtk_spin_button_set_value (window->spin_minabs,
                                 input->variable[i].rangeminabs);
      gtk_toggle_button_set_active
        (GTK_TOGGLE_BUTTON (window->check_minabs), 1);
    }
  else
    {
      gtk_spin_button_set_value (window->spin_minabs, -G_MAXDOUBLE);
      gtk_toggle_button_set_active
        (GTK_TOGGLE_BUTTON (window->check_minabs), 0);
    }
  if (input->variable[i].rangemaxabs != G_MAXDOUBLE)
    {
      gtk_spin_button_set_value (window->spin_maxabs,
                                 input->variable[i].rangemaxabs);
      gtk_toggle_button_set_active
        (GTK_TOGGLE_BUTTON (window->check_maxabs), 1);
    }
  else
    {
      gtk_spin_button_set_value (window->spin_maxabs, G_MAXDOUBLE);
      gtk_toggle_button_set_active
        (GTK_TOGGLE_BUTTON (window->check_maxabs), 0);
    }
  gtk_spin_button_set_value (window->spin_precision,
                             input->variable[i].precision);
  gtk_spin_button_set_value (window->spin_steps, (gdouble) input->nsteps);
  if (input->nsteps)
    gtk_spin_button_set_value (window->spin_step, input->variable[i].step);
#if DEBUG_INTERFACE
  fprintf (stderr, "window_set_variable: precision[%u]=%u\n", i,
           input->variable[i].precision);
#endif
  switch (window_get_algorithm ())
    {
    case ALGORITHM_SWEEP:
      gtk_spin_button_set_value (window->spin_sweeps,
                                 (gdouble) input->variable[i].nsweeps);
#if DEBUG_INTERFACE
      fprintf (stderr, "window_set_variable: nsweeps[%u]=%u\n", i,
               input->variable[i].nsweeps);
#endif
      break;
    case ALGORITHM_GENETIC:
      gtk_spin_button_set_value (window->spin_bits,
                                 (gdouble) input->variable[i].nbits);
#if DEBUG_INTERFACE
      fprintf (stderr, "window_set_variable: nbits[%u]=%u\n", i,
               input->variable[i].nbits);
#endif
      break;
    }
  window_update ();
#if DEBUG_INTERFACE
  fprintf (stderr, "window_set_variable: end\n");
#endif
}

/**
 * \fn void window_remove_variable ()
 * \brief Function to remove a variable in the main window.
 */
void
window_remove_variable ()
{
  unsigned int i, j;
#if DEBUG_INTERFACE
  fprintf (stderr, "window_remove_variable: start\n");
#endif
  i = gtk_combo_box_get_active (GTK_COMBO_BOX (window->combo_variable));
  g_signal_handler_block (window->combo_variable, window->id_variable);
  gtk_combo_box_text_remove (window->combo_variable, i);
  g_signal_handler_unblock (window->combo_variable, window->id_variable);
  xmlFree (input->variable[i].name);
  --input->nvariables;
  for (j = i; j < input->nvariables; ++j)
    memcpy (input->variable + j, input->variable + j + 1, sizeof (Variable));
  j = input->nvariables - 1;
  if (i > j)
    i = j;
  g_signal_handler_block (window->entry_variable, window->id_variable_label);
  gtk_combo_box_set_active (GTK_COMBO_BOX (window->combo_variable), i);
  g_signal_handler_unblock (window->entry_variable, window->id_variable_label);
  window_update ();
#if DEBUG_INTERFACE
  fprintf (stderr, "window_remove_variable: end\n");
#endif
}

/**
 * \fn void window_add_variable ()
 * \brief Function to add a variable in the main window.
 */
void
window_add_variable ()
{
  unsigned int i, j;
#if DEBUG_INTERFACE
  fprintf (stderr, "window_add_variable: start\n");
#endif
  i = gtk_combo_box_get_active (GTK_COMBO_BOX (window->combo_variable));
  g_signal_handler_block (window->combo_variable, window->id_variable);
  gtk_combo_box_text_insert_text (window->combo_variable, i,
                                  input->variable[i].name);
  g_signal_handler_unblock (window->combo_variable, window->id_variable);
  input->variable = (Variable *) g_realloc
    (input->variable, (input->nvariables + 1) * sizeof (Variable));
  for (j = input->nvariables - 1; j > i; --j)
    memcpy (input->variable + j + 1, input->variable + j, sizeof (Variable));
  memcpy (input->variable + j + 1, input->variable + j, sizeof (Variable));
  if (input->type == INPUT_TYPE_XML)
    input->variable[j + 1].name
      = (char *) xmlStrdup ((xmlChar *) input->variable[j].name);
  else
    input->variable[j + 1].name = g_strdup (input->variable[j].name);
  ++input->nvariables;
  g_signal_handler_block (window->entry_variable, window->id_variable_label);
  gtk_combo_box_set_active (GTK_COMBO_BOX (window->combo_variable), i + 1);
  g_signal_handler_unblock (window->entry_variable, window->id_variable_label);
  window_update ();
#if DEBUG_INTERFACE
  fprintf (stderr, "window_add_variable: end\n");
#endif
}

/**
 * \fn void window_label_variable ()
 * \brief Function to set the variable label in the main window.
 */
void
window_label_variable ()
{
  unsigned int i;
  const char *buffer;
#if DEBUG_INTERFACE
  fprintf (stderr, "window_label_variable: start\n");
#endif
  i = gtk_combo_box_get_active (GTK_COMBO_BOX (window->combo_variable));
  buffer = gtk_entry_get_text (window->entry_variable);
  g_signal_handler_block (window->combo_variable, window->id_variable);
  gtk_combo_box_text_remove (window->combo_variable, i);
  gtk_combo_box_text_insert_text (window->combo_variable, i, buffer);
  gtk_combo_box_set_active (GTK_COMBO_BOX (window->combo_variable), i);
  g_signal_handler_unblock (window->combo_variable, window->id_variable);
#if DEBUG_INTERFACE
  fprintf (stderr, "window_label_variable: end\n");
#endif
}

/**
 * \fn void window_precision_variable ()
 * \brief Function to update the variable precision in the main window.
 */
void
window_precision_variable ()
{
  unsigned int i;
#if DEBUG_INTERFACE
  fprintf (stderr, "window_precision_variable: start\n");
#endif
  i = gtk_combo_box_get_active (GTK_COMBO_BOX (window->combo_variable));
  input->variable[i].precision
    = (unsigned int) gtk_spin_button_get_value_as_int (window->spin_precision);
  gtk_spin_button_set_digits (window->spin_min, input->variable[i].precision);
  gtk_spin_button_set_digits (window->spin_max, input->variable[i].precision);
  gtk_spin_button_set_digits (window->spin_minabs,
                              input->variable[i].precision);
  gtk_spin_button_set_digits (window->spin_maxabs,
                              input->variable[i].precision);
#if DEBUG_INTERFACE
  fprintf (stderr, "window_precision_variable: end\n");
#endif
}

/**
 * \fn void window_rangemin_variable ()
 * \brief Function to update the variable rangemin in the main window.
 */
void
window_rangemin_variable ()
{
  unsigned int i;
#if DEBUG_INTERFACE
  fprintf (stderr, "window_rangemin_variable: start\n");
#endif
  i = gtk_combo_box_get_active (GTK_COMBO_BOX (window->combo_variable));
  input->variable[i].rangemin = gtk_spin_button_get_value (window->spin_min);
#if DEBUG_INTERFACE
  fprintf (stderr, "window_rangemin_variable: end\n");
#endif
}

/**
 * \fn void window_rangemax_variable ()
 * \brief Function to update the variable rangemax in the main window.
 */
void
window_rangemax_variable ()
{
  unsigned int i;
#if DEBUG_INTERFACE
  fprintf (stderr, "window_rangemax_variable: start\n");
#endif
  i = gtk_combo_box_get_active (GTK_COMBO_BOX (window->combo_variable));
  input->variable[i].rangemax = gtk_spin_button_get_value (window->spin_max);
#if DEBUG_INTERFACE
  fprintf (stderr, "window_rangemax_variable: end\n");
#endif
}

/**
 * \fn void window_rangeminabs_variable ()
 * \brief Function to update the variable rangeminabs in the main window.
 */
void
window_rangeminabs_variable ()
{
  unsigned int i;
#if DEBUG_INTERFACE
  fprintf (stderr, "window_rangeminabs_variable: start\n");
#endif
  i = gtk_combo_box_get_active (GTK_COMBO_BOX (window->combo_variable));
  input->variable[i].rangeminabs
    = gtk_spin_button_get_value (window->spin_minabs);
#if DEBUG_INTERFACE
  fprintf (stderr, "window_rangeminabs_variable: end\n");
#endif
}

/**
 * \fn void window_rangemaxabs_variable ()
 * \brief Function to update the variable rangemaxabs in the main window.
 */
void
window_rangemaxabs_variable ()
{
  unsigned int i;
#if DEBUG_INTERFACE
  fprintf (stderr, "window_rangemaxabs_variable: start\n");
#endif
  i = gtk_combo_box_get_active (GTK_COMBO_BOX (window->combo_variable));
  input->variable[i].rangemaxabs
    = gtk_spin_button_get_value (window->spin_maxabs);
#if DEBUG_INTERFACE
  fprintf (stderr, "window_rangemaxabs_variable: end\n");
#endif
}

/**
 * \fn void window_step_variable ()
 * \brief Function to update the variable step in the main window.
 */
void
window_step_variable ()
{
  unsigned int i;
#if DEBUG_INTERFACE
  fprintf (stderr, "window_step_variable: start\n");
#endif
  i = gtk_combo_box_get_active (GTK_COMBO_BOX (window->combo_variable));
  input->variable[i].step = gtk_spin_button_get_value (window->spin_step);
#if DEBUG_INTERFACE
  fprintf (stderr, "window_step_variable: end\n");
#endif
}

/**
 * \fn void window_update_variable ()
 * \brief Function to update the variable data in the main window.
 */
void
window_update_variable ()
{
  int i;
#if DEBUG_INTERFACE
  fprintf (stderr, "window_update_variable: start\n");
#endif
  i = gtk_combo_box_get_active (GTK_COMBO_BOX (window->combo_variable));
  if (i < 0)
    i = 0;
  switch (window_get_algorithm ())
    {
    case ALGORITHM_SWEEP:
      input->variable[i].nsweeps
        = gtk_spin_button_get_value_as_int (window->spin_sweeps);
#if DEBUG_INTERFACE
      fprintf (stderr, "window_update_variable: nsweeps[%d]=%u\n", i,
               input->variable[i].nsweeps);
#endif
      break;
    case ALGORITHM_GENETIC:
      input->variable[i].nbits
        = gtk_spin_button_get_value_as_int (window->spin_bits);
#if DEBUG_INTERFACE
      fprintf (stderr, "window_update_variable: nbits[%d]=%u\n", i,
               input->variable[i].nbits);
#endif
    }
#if DEBUG_INTERFACE
  fprintf (stderr, "window_update_variable: end\n");
#endif
}

/**
 * \fn int window_read (char *filename)
 * \brief Function to read the input data of a file.
 * \param filename
 * \brief File name.
 * \return 1 on succes, 0 on error.
 */
int
window_read (char *filename)
{
  unsigned int i;
  char *buffer;
#if DEBUG_INTERFACE
  fprintf (stderr, "window_read: start\n");
#endif

  // Reading new input file
  input_free ();
  if (!input_open (filename))
    {
#if DEBUG_INTERFACE
      fprintf (stderr, "window_read: end\n");
#endif
      return 0;
    }

  // Setting GTK+ widgets data
  gtk_entry_set_text (window->entry_result, input->result);
  gtk_entry_set_text (window->entry_variables, input->variables);
  buffer = g_build_filename (input->directory, input->simulator, NULL);
  gtk_file_chooser_set_filename (GTK_FILE_CHOOSER
                                 (window->button_simulator), buffer);
  g_free (buffer);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (window->check_evaluator),
                                (size_t) input->evaluator);
  if (input->evaluator)
    {
      buffer = g_build_filename (input->directory, input->evaluator, NULL);
      gtk_file_chooser_set_filename (GTK_FILE_CHOOSER
                                     (window->button_evaluator), buffer);
      g_free (buffer);
    }
  gtk_toggle_button_set_active
    (GTK_TOGGLE_BUTTON (window->button_algorithm[input->algorithm]), TRUE);
  switch (input->algorithm)
    {
    case ALGORITHM_MONTE_CARLO:
      gtk_spin_button_set_value (window->spin_simulations,
                                 (gdouble) input->nsimulations);
    case ALGORITHM_SWEEP:
      gtk_spin_button_set_value (window->spin_iterations,
                                 (gdouble) input->niterations);
      gtk_spin_button_set_value (window->spin_bests, (gdouble) input->nbest);
      gtk_spin_button_set_value (window->spin_tolerance, input->tolerance);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (window->check_direction),
                                    input->nsteps);
      if (input->nsteps)
        {
          gtk_toggle_button_set_active
            (GTK_TOGGLE_BUTTON (window->button_direction
                                [input->direction]), TRUE);
          gtk_spin_button_set_value (window->spin_steps,
                                     (gdouble) input->nsteps);
          gtk_spin_button_set_value (window->spin_relaxation,
                                     (gdouble) input->relaxation);
          switch (input->direction)
            {
            case DIRECTION_METHOD_RANDOM:
              gtk_spin_button_set_value (window->spin_estimates,
                                         (gdouble) input->nestimates);
            }
        }
      break;
    default:
      gtk_spin_button_set_value (window->spin_population,
                                 (gdouble) input->nsimulations);
      gtk_spin_button_set_value (window->spin_generations,
                                 (gdouble) input->niterations);
      gtk_spin_button_set_value (window->spin_mutation, input->mutation_ratio);
      gtk_spin_button_set_value (window->spin_reproduction,
                                 input->reproduction_ratio);
      gtk_spin_button_set_value (window->spin_adaptation,
                                 input->adaptation_ratio);
    }
  gtk_toggle_button_set_active
    (GTK_TOGGLE_BUTTON (window->button_norm[input->norm]), TRUE);
  gtk_spin_button_set_value (window->spin_p, input->p);
  gtk_spin_button_set_value (window->spin_threshold, input->threshold);
  g_signal_handler_block (window->combo_experiment, window->id_experiment);
  g_signal_handler_block (window->button_experiment,
                          window->id_experiment_name);
  gtk_combo_box_text_remove_all (window->combo_experiment);
  for (i = 0; i < input->nexperiments; ++i)
    gtk_combo_box_text_append_text (window->combo_experiment,
                                    input->experiment[i].name);
  g_signal_handler_unblock
    (window->button_experiment, window->id_experiment_name);
  g_signal_handler_unblock (window->combo_experiment, window->id_experiment);
  gtk_combo_box_set_active (GTK_COMBO_BOX (window->combo_experiment), 0);
  g_signal_handler_block (window->combo_variable, window->id_variable);
  g_signal_handler_block (window->entry_variable, window->id_variable_label);
  gtk_combo_box_text_remove_all (window->combo_variable);
  for (i = 0; i < input->nvariables; ++i)
    gtk_combo_box_text_append_text (window->combo_variable,
                                    input->variable[i].name);
  g_signal_handler_unblock (window->entry_variable, window->id_variable_label);
  g_signal_handler_unblock (window->combo_variable, window->id_variable);
  gtk_combo_box_set_active (GTK_COMBO_BOX (window->combo_variable), 0);
  window_set_variable ();
  window_update ();

#if DEBUG_INTERFACE
  fprintf (stderr, "window_read: end\n");
#endif
  return 1;
}

/**
 * \fn void window_open ()
 * \brief Function to open the input data.
 */
void
window_open ()
{
  GtkFileChooserDialog *dlg;
  GtkFileFilter *filter;
  char *buffer, *directory, *name;

#if DEBUG_INTERFACE
  fprintf (stderr, "window_open: start\n");
#endif

  // Saving a backup of the current input file
  directory = g_strdup (input->directory);
  name = g_strdup (input->name);

  // Opening dialog
  dlg = (GtkFileChooserDialog *)
    gtk_file_chooser_dialog_new (gettext ("Open input file"),
                                 window->window,
                                 GTK_FILE_CHOOSER_ACTION_OPEN,
                                 gettext ("_Cancel"), GTK_RESPONSE_CANCEL,
                                 gettext ("_OK"), GTK_RESPONSE_OK, NULL);

  // Adding XML filter
  filter = (GtkFileFilter *) gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, "XML");
  gtk_file_filter_add_pattern (filter, "*.xml");
  gtk_file_filter_add_pattern (filter, "*.XML");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dlg), filter);

  // Adding JSON filter
  filter = (GtkFileFilter *) gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, "JSON");
  gtk_file_filter_add_pattern (filter, "*.json");
  gtk_file_filter_add_pattern (filter, "*.JSON");
  gtk_file_filter_add_pattern (filter, "*.js");
  gtk_file_filter_add_pattern (filter, "*.JS");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dlg), filter);

  // If OK saving
  while (gtk_dialog_run (GTK_DIALOG (dlg)) == GTK_RESPONSE_OK)
    {

      // Traying to open the input file
      buffer = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dlg));
      if (!window_read (buffer))
        {
#if DEBUG_INTERFACE
          fprintf (stderr, "window_open: error reading input file\n");
#endif
          g_free (buffer);

          // Reading backup file on error
          buffer = g_build_filename (directory, name, NULL);
          if (!input_open (buffer))
            {

              // Closing on backup file reading error
#if DEBUG_INTERFACE
              fprintf (stderr, "window_read: error reading backup file\n");
#endif
              g_free (buffer);
              break;
            }
          g_free (buffer);
        }
      else
        {
          g_free (buffer);
          break;
        }
    }

  // Freeing and closing
  g_free (name);
  g_free (directory);
  gtk_widget_destroy (GTK_WIDGET (dlg));
#if DEBUG_INTERFACE
  fprintf (stderr, "window_open: end\n");
#endif
}

/**
 * \fn void window_new ()
 * \brief Function to open the main window.
 */
void
window_new ()
{
  unsigned int i;
  char *buffer, *buffer2, buffer3[64];
  char *label_algorithm[NALGORITHMS] = {
    "_Monte-Carlo", gettext ("_Sweep"), gettext ("_Genetic")
  };
  char *tip_algorithm[NALGORITHMS] = {
    gettext ("Monte-Carlo brute force algorithm"),
    gettext ("Sweep brute force algorithm"),
    gettext ("Genetic algorithm")
  };
  char *label_direction[NDIRECTIONS] = {
    gettext ("_Coordinates descent"), gettext ("_Random")
  };
  char *tip_direction[NDIRECTIONS] = {
    gettext ("Coordinates direction estimate method"),
    gettext ("Random direction estimate method")
  };
  char *label_norm[NNORMS] = { "L2", "L∞", "Lp", "L1" };
  char *tip_norm[NNORMS] = {
    gettext ("Euclidean error norm (L2)"),
    gettext ("Maximum error norm (L∞)"),
    gettext ("P error norm (Lp)"),
    gettext ("Taxicab error norm (L1)")
  };

#if DEBUG_INTERFACE
  fprintf (stderr, "window_new: start\n");
#endif

  // Creating the window
  window->window = main_window
    = (GtkWindow *) gtk_window_new (GTK_WINDOW_TOPLEVEL);

  // Finish when closing the window
  g_signal_connect (window->window, "delete-event", gtk_main_quit, NULL);

  // Setting the window title
  gtk_window_set_title (window->window, "MPCOTool");

  // Creating the open button
  window->button_open = (GtkToolButton *) gtk_tool_button_new
    (gtk_image_new_from_icon_name ("document-open",
                                   GTK_ICON_SIZE_LARGE_TOOLBAR),
     gettext ("Open"));
  g_signal_connect (window->button_open, "clicked", window_open, NULL);

  // Creating the save button
  window->button_save = (GtkToolButton *) gtk_tool_button_new
    (gtk_image_new_from_icon_name ("document-save",
                                   GTK_ICON_SIZE_LARGE_TOOLBAR),
     gettext ("Save"));
  g_signal_connect (window->button_save, "clicked", (void (*)) window_save,
                    NULL);

  // Creating the run button
  window->button_run = (GtkToolButton *) gtk_tool_button_new
    (gtk_image_new_from_icon_name ("system-run",
                                   GTK_ICON_SIZE_LARGE_TOOLBAR),
     gettext ("Run"));
  g_signal_connect (window->button_run, "clicked", window_run, NULL);

  // Creating the options button
  window->button_options = (GtkToolButton *) gtk_tool_button_new
    (gtk_image_new_from_icon_name ("preferences-system",
                                   GTK_ICON_SIZE_LARGE_TOOLBAR),
     gettext ("Options"));
  g_signal_connect (window->button_options, "clicked", options_new, NULL);

  // Creating the help button
  window->button_help = (GtkToolButton *) gtk_tool_button_new
    (gtk_image_new_from_icon_name ("help-browser",
                                   GTK_ICON_SIZE_LARGE_TOOLBAR),
     gettext ("Help"));
  g_signal_connect (window->button_help, "clicked", window_help, NULL);

  // Creating the about button
  window->button_about = (GtkToolButton *) gtk_tool_button_new
    (gtk_image_new_from_icon_name ("help-about",
                                   GTK_ICON_SIZE_LARGE_TOOLBAR),
     gettext ("About"));
  g_signal_connect (window->button_about, "clicked", window_about, NULL);

  // Creating the exit button
  window->button_exit = (GtkToolButton *) gtk_tool_button_new
    (gtk_image_new_from_icon_name ("application-exit",
                                   GTK_ICON_SIZE_LARGE_TOOLBAR),
     gettext ("Exit"));
  g_signal_connect (window->button_exit, "clicked", gtk_main_quit, NULL);

  // Creating the buttons bar
  window->bar_buttons = (GtkToolbar *) gtk_toolbar_new ();
  gtk_toolbar_insert
    (window->bar_buttons, GTK_TOOL_ITEM (window->button_open), 0);
  gtk_toolbar_insert
    (window->bar_buttons, GTK_TOOL_ITEM (window->button_save), 1);
  gtk_toolbar_insert
    (window->bar_buttons, GTK_TOOL_ITEM (window->button_run), 2);
  gtk_toolbar_insert
    (window->bar_buttons, GTK_TOOL_ITEM (window->button_options), 3);
  gtk_toolbar_insert
    (window->bar_buttons, GTK_TOOL_ITEM (window->button_help), 4);
  gtk_toolbar_insert
    (window->bar_buttons, GTK_TOOL_ITEM (window->button_about), 5);
  gtk_toolbar_insert
    (window->bar_buttons, GTK_TOOL_ITEM (window->button_exit), 6);
  gtk_toolbar_set_style (window->bar_buttons, GTK_TOOLBAR_BOTH);

  // Creating the simulator program label and entry
  window->label_simulator
    = (GtkLabel *) gtk_label_new (gettext ("Simulator program"));
  window->button_simulator = (GtkFileChooserButton *)
    gtk_file_chooser_button_new (gettext ("Simulator program"),
                                 GTK_FILE_CHOOSER_ACTION_OPEN);
  gtk_widget_set_tooltip_text (GTK_WIDGET (window->button_simulator),
                               gettext ("Simulator program executable file"));
  gtk_widget_set_hexpand (GTK_WIDGET (window->button_simulator), TRUE);

  // Creating the evaluator program label and entry
  window->check_evaluator = (GtkCheckButton *)
    gtk_check_button_new_with_mnemonic (gettext ("_Evaluator program"));
  g_signal_connect (window->check_evaluator, "toggled", window_update, NULL);
  window->button_evaluator = (GtkFileChooserButton *)
    gtk_file_chooser_button_new (gettext ("Evaluator program"),
                                 GTK_FILE_CHOOSER_ACTION_OPEN);
  gtk_widget_set_tooltip_text
    (GTK_WIDGET (window->button_evaluator),
     gettext ("Optional evaluator program executable file"));

  // Creating the results files labels and entries
  window->label_result = (GtkLabel *) gtk_label_new (gettext ("Result file"));
  window->entry_result = (GtkEntry *) gtk_entry_new ();
  gtk_widget_set_tooltip_text
    (GTK_WIDGET (window->entry_result), gettext ("Best results file"));
  window->label_variables
    = (GtkLabel *) gtk_label_new (gettext ("Variables file"));
  window->entry_variables = (GtkEntry *) gtk_entry_new ();
  gtk_widget_set_tooltip_text
    (GTK_WIDGET (window->entry_variables),
     gettext ("All simulated results file"));

  // Creating the files grid and attaching widgets
  window->grid_files = (GtkGrid *) gtk_grid_new ();
  gtk_grid_attach (window->grid_files, GTK_WIDGET (window->label_simulator),
                   0, 0, 1, 1);
  gtk_grid_attach (window->grid_files, GTK_WIDGET (window->button_simulator),
                   1, 0, 1, 1);
  gtk_grid_attach (window->grid_files, GTK_WIDGET (window->check_evaluator),
                   0, 1, 1, 1);
  gtk_grid_attach (window->grid_files, GTK_WIDGET (window->button_evaluator),
                   1, 1, 1, 1);
  gtk_grid_attach (window->grid_files, GTK_WIDGET (window->label_result),
                   0, 2, 1, 1);
  gtk_grid_attach (window->grid_files, GTK_WIDGET (window->entry_result),
                   1, 2, 1, 1);
  gtk_grid_attach (window->grid_files, GTK_WIDGET (window->label_variables),
                   0, 3, 1, 1);
  gtk_grid_attach (window->grid_files, GTK_WIDGET (window->entry_variables),
                   1, 3, 1, 1);

  // Creating the algorithm properties
  window->label_simulations = (GtkLabel *) gtk_label_new
    (gettext ("Simulations number"));
  window->spin_simulations
    = (GtkSpinButton *) gtk_spin_button_new_with_range (1., 1.e12, 1.);
  gtk_widget_set_tooltip_text
    (GTK_WIDGET (window->spin_simulations),
     gettext ("Number of simulations to perform for each iteration"));
  gtk_widget_set_hexpand (GTK_WIDGET (window->spin_simulations), TRUE);
  window->label_iterations = (GtkLabel *)
    gtk_label_new (gettext ("Iterations number"));
  window->spin_iterations
    = (GtkSpinButton *) gtk_spin_button_new_with_range (1., 1.e6, 1.);
  gtk_widget_set_tooltip_text
    (GTK_WIDGET (window->spin_iterations), gettext ("Number of iterations"));
  g_signal_connect
    (window->spin_iterations, "value-changed", window_update, NULL);
  gtk_widget_set_hexpand (GTK_WIDGET (window->spin_iterations), TRUE);
  window->label_tolerance = (GtkLabel *) gtk_label_new (gettext ("Tolerance"));
  window->spin_tolerance
    = (GtkSpinButton *) gtk_spin_button_new_with_range (0., 1., 0.001);
  gtk_widget_set_tooltip_text
    (GTK_WIDGET (window->spin_tolerance),
     gettext ("Tolerance to set the variable interval on the next iteration"));
  window->label_bests = (GtkLabel *) gtk_label_new (gettext ("Bests number"));
  window->spin_bests
    = (GtkSpinButton *) gtk_spin_button_new_with_range (1., 1.e6, 1.);
  gtk_widget_set_tooltip_text
    (GTK_WIDGET (window->spin_bests),
     gettext ("Number of best simulations used to set the variable interval "
              "on the next iteration"));
  window->label_population
    = (GtkLabel *) gtk_label_new (gettext ("Population number"));
  window->spin_population
    = (GtkSpinButton *) gtk_spin_button_new_with_range (1., 1.e12, 1.);
  gtk_widget_set_tooltip_text
    (GTK_WIDGET (window->spin_population),
     gettext ("Number of population for the genetic algorithm"));
  gtk_widget_set_hexpand (GTK_WIDGET (window->spin_population), TRUE);
  window->label_generations
    = (GtkLabel *) gtk_label_new (gettext ("Generations number"));
  window->spin_generations
    = (GtkSpinButton *) gtk_spin_button_new_with_range (1., 1.e6, 1.);
  gtk_widget_set_tooltip_text
    (GTK_WIDGET (window->spin_generations),
     gettext ("Number of generations for the genetic algorithm"));
  window->label_mutation
    = (GtkLabel *) gtk_label_new (gettext ("Mutation ratio"));
  window->spin_mutation
    = (GtkSpinButton *) gtk_spin_button_new_with_range (0., 1., 0.001);
  gtk_widget_set_tooltip_text
    (GTK_WIDGET (window->spin_mutation),
     gettext ("Ratio of mutation for the genetic algorithm"));
  window->label_reproduction
    = (GtkLabel *) gtk_label_new (gettext ("Reproduction ratio"));
  window->spin_reproduction
    = (GtkSpinButton *) gtk_spin_button_new_with_range (0., 1., 0.001);
  gtk_widget_set_tooltip_text
    (GTK_WIDGET (window->spin_reproduction),
     gettext ("Ratio of reproduction for the genetic algorithm"));
  window->label_adaptation
    = (GtkLabel *) gtk_label_new (gettext ("Adaptation ratio"));
  window->spin_adaptation
    = (GtkSpinButton *) gtk_spin_button_new_with_range (0., 1., 0.001);
  gtk_widget_set_tooltip_text
    (GTK_WIDGET (window->spin_adaptation),
     gettext ("Ratio of adaptation for the genetic algorithm"));
  window->label_threshold = (GtkLabel *) gtk_label_new (gettext ("Threshold"));
  window->spin_threshold = (GtkSpinButton *) gtk_spin_button_new_with_range
    (-G_MAXDOUBLE, G_MAXDOUBLE, precision[DEFAULT_PRECISION]);
  gtk_widget_set_tooltip_text
    (GTK_WIDGET (window->spin_threshold),
     gettext ("Threshold in the objective function to finish the simulations"));
  window->scrolled_threshold
    = (GtkScrolledWindow *) gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (window->scrolled_threshold),
                     GTK_WIDGET (window->spin_threshold));
//  gtk_widget_set_hexpand (GTK_WIDGET (window->scrolled_threshold), TRUE);
//  gtk_widget_set_halign (GTK_WIDGET (window->scrolled_threshold),
//                               GTK_ALIGN_FILL);

  // Creating the direction search method properties
  window->check_direction = (GtkCheckButton *)
    gtk_check_button_new_with_mnemonic (gettext ("_Direction search method"));
  g_signal_connect (window->check_direction, "clicked", window_update, NULL);
  window->grid_direction = (GtkGrid *) gtk_grid_new ();
  window->button_direction[0] = (GtkRadioButton *)
    gtk_radio_button_new_with_mnemonic (NULL, label_direction[0]);
  gtk_grid_attach (window->grid_direction,
                   GTK_WIDGET (window->button_direction[0]), 0, 0, 1, 1);
  g_signal_connect (window->button_direction[0], "clicked", window_update,
                    NULL);
  for (i = 0; ++i < NDIRECTIONS;)
    {
      window->button_direction[i] = (GtkRadioButton *)
        gtk_radio_button_new_with_mnemonic
        (gtk_radio_button_get_group (window->button_direction[0]),
         label_direction[i]);
      gtk_widget_set_tooltip_text (GTK_WIDGET (window->button_direction[i]),
                                   tip_direction[i]);
      gtk_grid_attach (window->grid_direction,
                       GTK_WIDGET (window->button_direction[i]), 0, i, 1, 1);
      g_signal_connect (window->button_direction[i], "clicked",
                        window_update, NULL);
    }
  window->label_steps = (GtkLabel *) gtk_label_new (gettext ("Steps number"));
  window->spin_steps = (GtkSpinButton *)
    gtk_spin_button_new_with_range (1., 1.e12, 1.);
  gtk_widget_set_hexpand (GTK_WIDGET (window->spin_steps), TRUE);
  window->label_estimates
    = (GtkLabel *) gtk_label_new (gettext ("Direction estimates number"));
  window->spin_estimates = (GtkSpinButton *)
    gtk_spin_button_new_with_range (1., 1.e3, 1.);
  window->label_relaxation
    = (GtkLabel *) gtk_label_new (gettext ("Relaxation parameter"));
  window->spin_relaxation = (GtkSpinButton *)
    gtk_spin_button_new_with_range (0., 2., 0.001);
  gtk_grid_attach (window->grid_direction, GTK_WIDGET (window->label_steps),
                   0, NDIRECTIONS, 1, 1);
  gtk_grid_attach (window->grid_direction, GTK_WIDGET (window->spin_steps),
                   1, NDIRECTIONS, 1, 1);
  gtk_grid_attach (window->grid_direction, GTK_WIDGET (window->label_estimates),
                   0, NDIRECTIONS + 1, 1, 1);
  gtk_grid_attach (window->grid_direction, GTK_WIDGET (window->spin_estimates),
                   1, NDIRECTIONS + 1, 1, 1);
  gtk_grid_attach (window->grid_direction,
                   GTK_WIDGET (window->label_relaxation), 0, NDIRECTIONS + 2, 1,
                   1);
  gtk_grid_attach (window->grid_direction, GTK_WIDGET (window->spin_relaxation),
                   1, NDIRECTIONS + 2, 1, 1);

  // Creating the array of algorithms
  window->grid_algorithm = (GtkGrid *) gtk_grid_new ();
  window->button_algorithm[0] = (GtkRadioButton *)
    gtk_radio_button_new_with_mnemonic (NULL, label_algorithm[0]);
  gtk_widget_set_tooltip_text (GTK_WIDGET (window->button_algorithm[0]),
                               tip_algorithm[0]);
  gtk_grid_attach (window->grid_algorithm,
                   GTK_WIDGET (window->button_algorithm[0]), 0, 0, 1, 1);
  g_signal_connect (window->button_algorithm[0], "clicked",
                    window_set_algorithm, NULL);
  for (i = 0; ++i < NALGORITHMS;)
    {
      window->button_algorithm[i] = (GtkRadioButton *)
        gtk_radio_button_new_with_mnemonic
        (gtk_radio_button_get_group (window->button_algorithm[0]),
         label_algorithm[i]);
      gtk_widget_set_tooltip_text (GTK_WIDGET (window->button_algorithm[i]),
                                   tip_algorithm[i]);
      gtk_grid_attach (window->grid_algorithm,
                       GTK_WIDGET (window->button_algorithm[i]), 0, i, 1, 1);
      g_signal_connect (window->button_algorithm[i], "clicked",
                        window_set_algorithm, NULL);
    }
  gtk_grid_attach (window->grid_algorithm,
                   GTK_WIDGET (window->label_simulations), 0,
                   NALGORITHMS, 1, 1);
  gtk_grid_attach (window->grid_algorithm,
                   GTK_WIDGET (window->spin_simulations), 1, NALGORITHMS, 1, 1);
  gtk_grid_attach (window->grid_algorithm,
                   GTK_WIDGET (window->label_iterations), 0,
                   NALGORITHMS + 1, 1, 1);
  gtk_grid_attach (window->grid_algorithm,
                   GTK_WIDGET (window->spin_iterations), 1,
                   NALGORITHMS + 1, 1, 1);
  gtk_grid_attach (window->grid_algorithm,
                   GTK_WIDGET (window->label_tolerance), 0,
                   NALGORITHMS + 2, 1, 1);
  gtk_grid_attach (window->grid_algorithm,
                   GTK_WIDGET (window->spin_tolerance), 1,
                   NALGORITHMS + 2, 1, 1);
  gtk_grid_attach (window->grid_algorithm,
                   GTK_WIDGET (window->label_bests), 0, NALGORITHMS + 3, 1, 1);
  gtk_grid_attach (window->grid_algorithm,
                   GTK_WIDGET (window->spin_bests), 1, NALGORITHMS + 3, 1, 1);
  gtk_grid_attach (window->grid_algorithm,
                   GTK_WIDGET (window->label_population), 0,
                   NALGORITHMS + 4, 1, 1);
  gtk_grid_attach (window->grid_algorithm,
                   GTK_WIDGET (window->spin_population), 1,
                   NALGORITHMS + 4, 1, 1);
  gtk_grid_attach (window->grid_algorithm,
                   GTK_WIDGET (window->label_generations), 0,
                   NALGORITHMS + 5, 1, 1);
  gtk_grid_attach (window->grid_algorithm,
                   GTK_WIDGET (window->spin_generations), 1,
                   NALGORITHMS + 5, 1, 1);
  gtk_grid_attach (window->grid_algorithm,
                   GTK_WIDGET (window->label_mutation), 0,
                   NALGORITHMS + 6, 1, 1);
  gtk_grid_attach (window->grid_algorithm,
                   GTK_WIDGET (window->spin_mutation), 1,
                   NALGORITHMS + 6, 1, 1);
  gtk_grid_attach (window->grid_algorithm,
                   GTK_WIDGET (window->label_reproduction), 0,
                   NALGORITHMS + 7, 1, 1);
  gtk_grid_attach (window->grid_algorithm,
                   GTK_WIDGET (window->spin_reproduction), 1,
                   NALGORITHMS + 7, 1, 1);
  gtk_grid_attach (window->grid_algorithm,
                   GTK_WIDGET (window->label_adaptation), 0,
                   NALGORITHMS + 8, 1, 1);
  gtk_grid_attach (window->grid_algorithm,
                   GTK_WIDGET (window->spin_adaptation), 1,
                   NALGORITHMS + 8, 1, 1);
  gtk_grid_attach (window->grid_algorithm,
                   GTK_WIDGET (window->check_direction), 0,
                   NALGORITHMS + 9, 2, 1);
  gtk_grid_attach (window->grid_algorithm,
                   GTK_WIDGET (window->grid_direction), 0,
                   NALGORITHMS + 10, 2, 1);
  gtk_grid_attach (window->grid_algorithm, GTK_WIDGET (window->label_threshold),
                   0, NALGORITHMS + 11, 1, 1);
  gtk_grid_attach (window->grid_algorithm,
                   GTK_WIDGET (window->scrolled_threshold), 1,
                   NALGORITHMS + 11, 1, 1);
  window->frame_algorithm = (GtkFrame *) gtk_frame_new (gettext ("Algorithm"));
  gtk_container_add (GTK_CONTAINER (window->frame_algorithm),
                     GTK_WIDGET (window->grid_algorithm));

  // Creating the variable widgets
  window->combo_variable = (GtkComboBoxText *) gtk_combo_box_text_new ();
  gtk_widget_set_tooltip_text
    (GTK_WIDGET (window->combo_variable), gettext ("Variables selector"));
  window->id_variable = g_signal_connect
    (window->combo_variable, "changed", window_set_variable, NULL);
  window->button_add_variable
    = (GtkButton *) gtk_button_new_from_icon_name ("list-add",
                                                   GTK_ICON_SIZE_BUTTON);
  g_signal_connect
    (window->button_add_variable, "clicked", window_add_variable, NULL);
  gtk_widget_set_tooltip_text
    (GTK_WIDGET (window->button_add_variable), gettext ("Add variable"));
  window->button_remove_variable
    = (GtkButton *) gtk_button_new_from_icon_name ("list-remove",
                                                   GTK_ICON_SIZE_BUTTON);
  g_signal_connect
    (window->button_remove_variable, "clicked", window_remove_variable, NULL);
  gtk_widget_set_tooltip_text
    (GTK_WIDGET (window->button_remove_variable), gettext ("Remove variable"));
  window->label_variable = (GtkLabel *) gtk_label_new (gettext ("Name"));
  window->entry_variable = (GtkEntry *) gtk_entry_new ();
  gtk_widget_set_tooltip_text
    (GTK_WIDGET (window->entry_variable), gettext ("Variable name"));
  gtk_widget_set_hexpand (GTK_WIDGET (window->entry_variable), TRUE);
  window->id_variable_label = g_signal_connect
    (window->entry_variable, "changed", window_label_variable, NULL);
  window->label_min = (GtkLabel *) gtk_label_new (gettext ("Minimum"));
  window->spin_min = (GtkSpinButton *) gtk_spin_button_new_with_range
    (-G_MAXDOUBLE, G_MAXDOUBLE, precision[DEFAULT_PRECISION]);
  gtk_widget_set_tooltip_text
    (GTK_WIDGET (window->spin_min),
     gettext ("Minimum initial value of the variable"));
  window->scrolled_min
    = (GtkScrolledWindow *) gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (window->scrolled_min),
                     GTK_WIDGET (window->spin_min));
  g_signal_connect (window->spin_min, "value-changed",
                    window_rangemin_variable, NULL);
  window->label_max = (GtkLabel *) gtk_label_new (gettext ("Maximum"));
  window->spin_max = (GtkSpinButton *) gtk_spin_button_new_with_range
    (-G_MAXDOUBLE, G_MAXDOUBLE, precision[DEFAULT_PRECISION]);
  gtk_widget_set_tooltip_text
    (GTK_WIDGET (window->spin_max),
     gettext ("Maximum initial value of the variable"));
  window->scrolled_max
    = (GtkScrolledWindow *) gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (window->scrolled_max),
                     GTK_WIDGET (window->spin_max));
  g_signal_connect (window->spin_max, "value-changed",
                    window_rangemax_variable, NULL);
  window->check_minabs = (GtkCheckButton *)
    gtk_check_button_new_with_mnemonic (gettext ("_Absolute minimum"));
  g_signal_connect (window->check_minabs, "toggled", window_update, NULL);
  window->spin_minabs = (GtkSpinButton *) gtk_spin_button_new_with_range
    (-G_MAXDOUBLE, G_MAXDOUBLE, precision[DEFAULT_PRECISION]);
  gtk_widget_set_tooltip_text
    (GTK_WIDGET (window->spin_minabs),
     gettext ("Minimum allowed value of the variable"));
  window->scrolled_minabs
    = (GtkScrolledWindow *) gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (window->scrolled_minabs),
                     GTK_WIDGET (window->spin_minabs));
  g_signal_connect (window->spin_minabs, "value-changed",
                    window_rangeminabs_variable, NULL);
  window->check_maxabs = (GtkCheckButton *)
    gtk_check_button_new_with_mnemonic (gettext ("_Absolute maximum"));
  g_signal_connect (window->check_maxabs, "toggled", window_update, NULL);
  window->spin_maxabs = (GtkSpinButton *) gtk_spin_button_new_with_range
    (-G_MAXDOUBLE, G_MAXDOUBLE, precision[DEFAULT_PRECISION]);
  gtk_widget_set_tooltip_text
    (GTK_WIDGET (window->spin_maxabs),
     gettext ("Maximum allowed value of the variable"));
  window->scrolled_maxabs
    = (GtkScrolledWindow *) gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (window->scrolled_maxabs),
                     GTK_WIDGET (window->spin_maxabs));
  g_signal_connect (window->spin_maxabs, "value-changed",
                    window_rangemaxabs_variable, NULL);
  window->label_precision
    = (GtkLabel *) gtk_label_new (gettext ("Precision digits"));
  window->spin_precision = (GtkSpinButton *)
    gtk_spin_button_new_with_range (0., (gdouble) DEFAULT_PRECISION, 1.);
  gtk_widget_set_tooltip_text
    (GTK_WIDGET (window->spin_precision),
     gettext ("Number of precision floating point digits\n"
              "0 is for integer numbers"));
  g_signal_connect (window->spin_precision, "value-changed",
                    window_precision_variable, NULL);
  window->label_sweeps = (GtkLabel *) gtk_label_new (gettext ("Sweeps number"));
  window->spin_sweeps
    = (GtkSpinButton *) gtk_spin_button_new_with_range (1., 1.e12, 1.);
  gtk_widget_set_tooltip_text
    (GTK_WIDGET (window->spin_sweeps),
     gettext ("Number of steps sweeping the variable"));
  g_signal_connect
    (window->spin_sweeps, "value-changed", window_update_variable, NULL);
  window->label_bits = (GtkLabel *) gtk_label_new (gettext ("Bits number"));
  window->spin_bits
    = (GtkSpinButton *) gtk_spin_button_new_with_range (1., 64., 1.);
  gtk_widget_set_tooltip_text
    (GTK_WIDGET (window->spin_bits),
     gettext ("Number of bits to encode the variable"));
  g_signal_connect
    (window->spin_bits, "value-changed", window_update_variable, NULL);
  window->label_step = (GtkLabel *) gtk_label_new (gettext ("Step size"));
  window->spin_step = (GtkSpinButton *) gtk_spin_button_new_with_range
    (-G_MAXDOUBLE, G_MAXDOUBLE, precision[DEFAULT_PRECISION]);
  gtk_widget_set_tooltip_text
    (GTK_WIDGET (window->spin_step),
     gettext ("Initial step size for the direction search method"));
  window->scrolled_step
    = (GtkScrolledWindow *) gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (window->scrolled_step),
                     GTK_WIDGET (window->spin_step));
  g_signal_connect
    (window->spin_step, "value-changed", window_step_variable, NULL);
  window->grid_variable = (GtkGrid *) gtk_grid_new ();
  gtk_grid_attach (window->grid_variable,
                   GTK_WIDGET (window->combo_variable), 0, 0, 2, 1);
  gtk_grid_attach (window->grid_variable,
                   GTK_WIDGET (window->button_add_variable), 2, 0, 1, 1);
  gtk_grid_attach (window->grid_variable,
                   GTK_WIDGET (window->button_remove_variable), 3, 0, 1, 1);
  gtk_grid_attach (window->grid_variable,
                   GTK_WIDGET (window->label_variable), 0, 1, 1, 1);
  gtk_grid_attach (window->grid_variable,
                   GTK_WIDGET (window->entry_variable), 1, 1, 3, 1);
  gtk_grid_attach (window->grid_variable,
                   GTK_WIDGET (window->label_min), 0, 2, 1, 1);
  gtk_grid_attach (window->grid_variable,
                   GTK_WIDGET (window->scrolled_min), 1, 2, 3, 1);
  gtk_grid_attach (window->grid_variable,
                   GTK_WIDGET (window->label_max), 0, 3, 1, 1);
  gtk_grid_attach (window->grid_variable,
                   GTK_WIDGET (window->scrolled_max), 1, 3, 3, 1);
  gtk_grid_attach (window->grid_variable,
                   GTK_WIDGET (window->check_minabs), 0, 4, 1, 1);
  gtk_grid_attach (window->grid_variable,
                   GTK_WIDGET (window->scrolled_minabs), 1, 4, 3, 1);
  gtk_grid_attach (window->grid_variable,
                   GTK_WIDGET (window->check_maxabs), 0, 5, 1, 1);
  gtk_grid_attach (window->grid_variable,
                   GTK_WIDGET (window->scrolled_maxabs), 1, 5, 3, 1);
  gtk_grid_attach (window->grid_variable,
                   GTK_WIDGET (window->label_precision), 0, 6, 1, 1);
  gtk_grid_attach (window->grid_variable,
                   GTK_WIDGET (window->spin_precision), 1, 6, 3, 1);
  gtk_grid_attach (window->grid_variable,
                   GTK_WIDGET (window->label_sweeps), 0, 7, 1, 1);
  gtk_grid_attach (window->grid_variable,
                   GTK_WIDGET (window->spin_sweeps), 1, 7, 3, 1);
  gtk_grid_attach (window->grid_variable,
                   GTK_WIDGET (window->label_bits), 0, 8, 1, 1);
  gtk_grid_attach (window->grid_variable,
                   GTK_WIDGET (window->spin_bits), 1, 8, 3, 1);
  gtk_grid_attach (window->grid_variable,
                   GTK_WIDGET (window->label_step), 0, 9, 1, 1);
  gtk_grid_attach (window->grid_variable,
                   GTK_WIDGET (window->scrolled_step), 1, 9, 3, 1);
  window->frame_variable = (GtkFrame *) gtk_frame_new (gettext ("Variable"));
  gtk_container_add (GTK_CONTAINER (window->frame_variable),
                     GTK_WIDGET (window->grid_variable));

  // Creating the experiment widgets
  window->combo_experiment = (GtkComboBoxText *) gtk_combo_box_text_new ();
  gtk_widget_set_tooltip_text (GTK_WIDGET (window->combo_experiment),
                               gettext ("Experiment selector"));
  window->id_experiment = g_signal_connect
    (window->combo_experiment, "changed", window_set_experiment, NULL);
  window->button_add_experiment
    = (GtkButton *) gtk_button_new_from_icon_name ("list-add",
                                                   GTK_ICON_SIZE_BUTTON);
  g_signal_connect
    (window->button_add_experiment, "clicked", window_add_experiment, NULL);
  gtk_widget_set_tooltip_text (GTK_WIDGET (window->button_add_experiment),
                               gettext ("Add experiment"));
  window->button_remove_experiment
    = (GtkButton *) gtk_button_new_from_icon_name ("list-remove",
                                                   GTK_ICON_SIZE_BUTTON);
  g_signal_connect (window->button_remove_experiment, "clicked",
                    window_remove_experiment, NULL);
  gtk_widget_set_tooltip_text (GTK_WIDGET (window->button_remove_experiment),
                               gettext ("Remove experiment"));
  window->label_experiment
    = (GtkLabel *) gtk_label_new (gettext ("Experimental data file"));
  window->button_experiment = (GtkFileChooserButton *)
    gtk_file_chooser_button_new (gettext ("Experimental data file"),
                                 GTK_FILE_CHOOSER_ACTION_OPEN);
  gtk_widget_set_tooltip_text (GTK_WIDGET (window->button_experiment),
                               gettext ("Experimental data file"));
  window->id_experiment_name
    = g_signal_connect (window->button_experiment, "selection-changed",
                        window_name_experiment, NULL);
  gtk_widget_set_hexpand (GTK_WIDGET (window->button_experiment), TRUE);
  window->label_weight = (GtkLabel *) gtk_label_new (gettext ("Weight"));
  window->spin_weight
    = (GtkSpinButton *) gtk_spin_button_new_with_range (0., 1., 0.001);
  gtk_widget_set_tooltip_text
    (GTK_WIDGET (window->spin_weight),
     gettext ("Weight factor to build the objective function"));
  g_signal_connect
    (window->spin_weight, "value-changed", window_weight_experiment, NULL);
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
  gtk_grid_attach (window->grid_experiment,
                   GTK_WIDGET (window->label_weight), 0, 2, 1, 1);
  gtk_grid_attach (window->grid_experiment,
                   GTK_WIDGET (window->spin_weight), 1, 2, 3, 1);
  for (i = 0; i < MAX_NINPUTS; ++i)
    {
      snprintf (buffer3, 64, "%s %u", gettext ("Input template"), i + 1);
      window->check_template[i] = (GtkCheckButton *)
        gtk_check_button_new_with_label (buffer3);
      window->id_template[i]
        = g_signal_connect (window->check_template[i], "toggled",
                            window_inputs_experiment, NULL);
      gtk_grid_attach (window->grid_experiment,
                       GTK_WIDGET (window->check_template[i]), 0, 3 + i, 1, 1);
      window->button_template[i] = (GtkFileChooserButton *)
        gtk_file_chooser_button_new (gettext ("Input template"),
                                     GTK_FILE_CHOOSER_ACTION_OPEN);
      gtk_widget_set_tooltip_text
        (GTK_WIDGET (window->button_template[i]),
         gettext ("Experimental input template file"));
      window->id_input[i]
        = g_signal_connect_swapped (window->button_template[i],
                                    "selection-changed",
                                    (void (*)) window_template_experiment,
                                    (void *) (size_t) i);
      gtk_grid_attach (window->grid_experiment,
                       GTK_WIDGET (window->button_template[i]), 1, 3 + i, 3, 1);
    }
  window->frame_experiment
    = (GtkFrame *) gtk_frame_new (gettext ("Experiment"));
  gtk_container_add (GTK_CONTAINER (window->frame_experiment),
                     GTK_WIDGET (window->grid_experiment));

  // Creating the error norm widgets
  window->frame_norm = (GtkFrame *) gtk_frame_new (gettext ("Error norm"));
  window->grid_norm = (GtkGrid *) gtk_grid_new ();
  gtk_container_add (GTK_CONTAINER (window->frame_norm),
                     GTK_WIDGET (window->grid_norm));
  window->button_norm[0] = (GtkRadioButton *)
    gtk_radio_button_new_with_mnemonic (NULL, label_norm[0]);
  gtk_widget_set_tooltip_text (GTK_WIDGET (window->button_norm[0]),
                               tip_norm[0]);
  gtk_grid_attach (window->grid_norm,
                   GTK_WIDGET (window->button_norm[0]), 0, 0, 1, 1);
  g_signal_connect (window->button_norm[0], "clicked", window_update, NULL);
  for (i = 0; ++i < NNORMS;)
    {
      window->button_norm[i] = (GtkRadioButton *)
        gtk_radio_button_new_with_mnemonic
        (gtk_radio_button_get_group (window->button_norm[0]), label_norm[i]);
      gtk_widget_set_tooltip_text (GTK_WIDGET (window->button_norm[i]),
                                   tip_norm[i]);
      gtk_grid_attach (window->grid_norm,
                       GTK_WIDGET (window->button_norm[i]), 0, i, 1, 1);
      g_signal_connect (window->button_norm[i], "clicked", window_update, NULL);
    }
  window->label_p = (GtkLabel *) gtk_label_new (gettext ("P parameter"));
  gtk_grid_attach (window->grid_norm, GTK_WIDGET (window->label_p), 1, 1, 1, 1);
  window->spin_p = (GtkSpinButton *)
    gtk_spin_button_new_with_range (-G_MAXDOUBLE, G_MAXDOUBLE, 0.01);
  gtk_widget_set_tooltip_text
    (GTK_WIDGET (window->spin_p), gettext ("P parameter for the P error norm"));
  window->scrolled_p
    = (GtkScrolledWindow *) gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (window->scrolled_p),
                     GTK_WIDGET (window->spin_p));
  gtk_widget_set_hexpand (GTK_WIDGET (window->scrolled_p), TRUE);
  gtk_widget_set_halign (GTK_WIDGET (window->scrolled_p), GTK_ALIGN_FILL);
  gtk_grid_attach (window->grid_norm, GTK_WIDGET (window->scrolled_p),
                   1, 2, 1, 2);

  // Creating the grid and attaching the widgets to the grid
  window->grid = (GtkGrid *) gtk_grid_new ();
  gtk_grid_attach (window->grid, GTK_WIDGET (window->bar_buttons), 0, 0, 3, 1);
  gtk_grid_attach (window->grid, GTK_WIDGET (window->grid_files), 0, 1, 1, 1);
  gtk_grid_attach (window->grid,
                   GTK_WIDGET (window->frame_algorithm), 0, 2, 1, 1);
  gtk_grid_attach (window->grid,
                   GTK_WIDGET (window->frame_variable), 1, 2, 1, 1);
  gtk_grid_attach (window->grid,
                   GTK_WIDGET (window->frame_experiment), 2, 2, 1, 1);
  gtk_grid_attach (window->grid, GTK_WIDGET (window->frame_norm), 1, 1, 2, 1);
  gtk_container_add (GTK_CONTAINER (window->window), GTK_WIDGET (window->grid));

  // Setting the window logo
  window->logo = gdk_pixbuf_new_from_xpm_data (logo);
  gtk_window_set_icon (window->window, window->logo);

  // Showing the window
  gtk_widget_show_all (GTK_WIDGET (window->window));

  // In GTK+ 3.16 and 3.18 the default scrolled size is wrong
#if GTK_MINOR_VERSION >= 16
  gtk_widget_set_size_request (GTK_WIDGET (window->scrolled_min), -1, 40);
  gtk_widget_set_size_request (GTK_WIDGET (window->scrolled_max), -1, 40);
  gtk_widget_set_size_request (GTK_WIDGET (window->scrolled_minabs), -1, 40);
  gtk_widget_set_size_request (GTK_WIDGET (window->scrolled_maxabs), -1, 40);
  gtk_widget_set_size_request (GTK_WIDGET (window->scrolled_step), -1, 40);
  gtk_widget_set_size_request (GTK_WIDGET (window->scrolled_p), -1, 40);
  gtk_widget_set_size_request (GTK_WIDGET (window->scrolled_threshold), -1, 40);
#endif

  // Reading initial example
  input_new ();
  buffer2 = g_get_current_dir ();
  buffer = g_build_filename (buffer2, "..", "tests", "test1", INPUT_FILE, NULL);
  g_free (buffer2);
  window_read (buffer);
  g_free (buffer);

#if DEBUG_INTERFACE
  fprintf (stderr, "window_new: start\n");
#endif
}
