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
 * \file mpcotool.c
 * \brief Main function source file.
 * \authors Javier Burguete and Borja Latorre.
 * \copyright Copyright 2012-2023, all rights reserved.
 */
#define _GNU_SOURCE
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <math.h>
#include <locale.h>
#include <gsl/gsl_rng.h>
#include <libxml/parser.h>
#include <libintl.h>
#include <glib.h>
#include <json-glib/json-glib.h>
#ifdef G_OS_WIN32
#include <windows.h>
#endif
#if HAVE_MPI
#include <mpi.h>
#endif
#if HAVE_GTK
#include <gio/gio.h>
#include <gtk/gtk.h>
#endif
#include "jb/src/jb_win.h"
#include "genetic/genetic.h"
#include "tools.h"
#include "experiment.h"
#include "variable.h"
#include "input.h"
#include "optimize.h"
#if HAVE_GTK
#include "interface.h"
#endif
#include "mpcotool.h"

#define DEBUG_MPCOTOOL 1        ///< Macro to debug main functions.

/**
 * Main function.
 *
 * \return 0 on success, >0 on error.
 */
int
mpcotool (int argn,             ///< Arguments number.
          char **argc)          ///< Arguments pointer.
{
  const struct option options[] = {
    {"seed", required_argument, NULL, 's'},
    {"nthreads", required_argument, NULL, 't'},
    {NULL, 0, NULL, 0}
  };
#if HAVE_GTK
  GtkApplication *application;
  char *buffer;
#endif
  int o, option_index;

  // Starting pseudo-random numbers generator
#if DEBUG_MPCOTOOL
  fprintf (stderr, "mpcotool: starting pseudo-random numbers generator\n");
#endif
  optimize->rng = gsl_rng_alloc (gsl_rng_taus2);

  // Allowing spaces in the XML data file
#if DEBUG_MPCOTOOL
  fprintf (stderr, "mpcotool: allowing spaces in the XML data file\n");
#endif
  xmlKeepBlanksDefault (0);

  // Starting MPI
#if HAVE_MPI
#if DEBUG_MPCOTOOL
  fprintf (stderr, "mpcotool: starting MPI\n");
#endif
  MPI_Init (&argn, &argc);
  MPI_Comm_size (MPI_COMM_WORLD, &ntasks);
  MPI_Comm_rank (MPI_COMM_WORLD, &optimize->mpi_rank);
  printf ("rank=%d tasks=%d\n", optimize->mpi_rank, ntasks);
#else
  ntasks = 1;
#endif

  // Getting threads number and pseudo-random numbers generator seed
  nthreads_climbing = nthreads = jb_get_ncores ();
  optimize->seed = DEFAULT_RANDOM_SEED;

  // Parsing command line arguments
  while (1)
    {
      o = getopt_long (argn, argc, "s:t:", options, &option_index);
      if (o == -1)
        break;
      switch (o)
        {
        case 's':
          optimize->seed = atol (optarg);
          break;
        case 't':
          nthreads_climbing = nthreads = atoi (optarg);
          break;
        default:
          printf ("%s\n%s\n", _("ERROR!"), _("Unknown option"));
          return 1;
        }
    }
  argn -= optind;

  // Resetting result and variables file names
#if DEBUG_MPCOTOOL
  fprintf (stderr, "mpcotool: resetting result and variables file names\n");
#endif
  input->result = input->variables = NULL;

#if HAVE_GTK

  // Setting local language and international floating point numbers notation
  setlocale (LC_ALL, "");
  setlocale (LC_NUMERIC, "C");
  window->application_directory = g_get_current_dir ();
  buffer = g_build_filename (window->application_directory, LOCALE_DIR, NULL);
  bindtextdomain (PROGRAM_INTERFACE, buffer);
  bind_textdomain_codeset (PROGRAM_INTERFACE, "UTF-8");
  textdomain (PROGRAM_INTERFACE);

  // Initing GTK+
  gtk_disable_setlocale ();
  application = gtk_application_new ("es.csic.eead.auladei.sprinkler",
                                     G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect (application, "activate", G_CALLBACK (window_new), NULL);

  // Opening the main window
  g_application_run (G_APPLICATION (application), 0, NULL);

  // Freeing memory
  input_free ();
  g_free (buffer);
  gtk_window_destroy (window->window);
  g_object_unref (application);
  g_free (window->application_directory);

#else

  // Checking syntax
  if (argn < 1 || argn > 3)
    {
      printf ("The syntax is:\n"
              "./mpcotoolbin [-nthreads x] [-seed s] data_file [result_file] "
              "[variables_file]\n");
      return 2;
    }
  if (argn > 1)
    input->result = (char *) xmlStrdup ((xmlChar *) argc[optind + 1]);
  if (argn == 2)
    input->variables = (char *) xmlStrdup ((xmlChar *) argc[optind + 2]);

  // Making optimization
#if DEBUG_MPCOTOOL
  fprintf (stderr, "mpcotool: making optimization\n");
#endif
  if (input_open (argc[optind]))
    optimize_open ();

  // Freeing memory
#if DEBUG_MPCOTOOL
  fprintf (stderr, "mpcotool: freeing memory and closing\n");
#endif
  optimize_free ();

#endif

  // Closing MPI
#if HAVE_MPI
  MPI_Finalize ();
#endif

  // Freeing memory
  gsl_rng_free (optimize->rng);

  // Closing
  return 0;
}
