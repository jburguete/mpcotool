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
 * \file main.c
 * \brief Main source file.
 * \authors Javier Burguete and Borja Latorre.
 * \copyright Copyright 2012-2016, all rights reserved.
 */
#define _GNU_SOURCE
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <locale.h>
#include <gsl/gsl_rng.h>
#include <libxml/parser.h>
#include <libintl.h>
#include <glib.h>
#include <glib/gstdio.h>
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
#include "genetic/genetic.h"
#include "utils.h"
#include "experiment.h"
#include "variable.h"
#include "input.h"
#include "optimize.h"
#if HAVE_GTK
#include "interface.h"
#endif

#define DEBUG 0                 ///< Macro to debug.

/**
 * \fn int main (int argn, char **argc)
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
  char *buffer;
#endif

  // Starting pseudo-random numbers generator
#if DEBUG
  fprintf (stderr, "main: starting pseudo-random numbers generator\n");
#endif
  optimize->rng = gsl_rng_alloc (gsl_rng_taus2);

  // Allowing spaces in the XML data file
#if DEBUG
  fprintf (stderr, "main: allowing spaces in the XML data file\n");
#endif
  xmlKeepBlanksDefault (0);

  // Starting MPI
#if HAVE_MPI
#if DEBUG
  fprintf (stderr, "main: starting MPI\n");
#endif
  MPI_Init (&argn, &argc);
  MPI_Comm_size (MPI_COMM_WORLD, &ntasks);
  MPI_Comm_rank (MPI_COMM_WORLD, &optimize->mpi_rank);
  printf ("rank=%d tasks=%d\n", optimize->mpi_rank, ntasks);
#else
  ntasks = 1;
#endif

  // Resetting result and variables file names
#if DEBUG
  fprintf (stderr, "main: resetting result and variables file names\n");
#endif
  input->result = input->variables = NULL;

#if HAVE_GTK

  // Getting threads number and pseudo-random numbers generator seed
  nthreads_direction = nthreads = cores_number ();
  optimize->seed = DEFAULT_RANDOM_SEED;

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
  gtk_init (&argn, &argc);

  // Opening the main window
  window_new ();
  gtk_main ();

  // Freeing memory
  input_free ();
  g_free (buffer);
  gtk_widget_destroy (GTK_WIDGET (window->window));
  g_free (window->application_directory);

#else

  // Checking syntax
  if (argn < 2)
    {
      printf ("The syntax is:\n"
              "./mpcotoolbin [-nthreads x] [-seed s] data_file [result_file] "
              "[variables_file]\n");
      return 1;
    }

  // Getting threads number and pseudo-random numbers generator seed
#if DEBUG
  fprintf (stderr, "main: getting threads number and pseudo-random numbers "
           "generator seed\n");
#endif
  nthreads_direction = nthreads = cores_number ();
  optimize->seed = DEFAULT_RANDOM_SEED;
  if (argn > 2 && !strcmp (argc[1], "-nthreads"))
    {
      nthreads_direction = nthreads = atoi (argc[2]);
      if (!nthreads)
        {
          printf ("Bad threads number\n");
          return 2;
        }
      argc += 2;
      argn -= 2;
      if (argn > 2 && !strcmp (argc[1], "-seed"))
        {
          optimize->seed = atoi (argc[2]);
          argc += 2;
          argn -= 2;
        }
    }
  else if (argn > 2 && !strcmp (argc[1], "-seed"))
    {
      optimize->seed = atoi (argc[2]);
      argc += 2;
      argn -= 2;
      if (argn > 2 && !strcmp (argc[1], "-nthreads"))
        {
          nthreads_direction = nthreads = atoi (argc[2]);
          if (!nthreads)
            {
              printf ("Bad threads number\n");
              return 2;
            }
          argc += 2;
          argn -= 2;
        }
    }
  printf ("nthreads=%u\n", nthreads);
  printf ("seed=%lu\n", optimize->seed);

  // Checking arguments
#if DEBUG
  fprintf (stderr, "main: checking arguments\n");
#endif
  if (argn > 4 || argn < 2)
    {
      printf ("The syntax is:\n"
              "./mpcotoolbin [-nthreads x] [-seed s] data_file [result_file] "
              "[variables_file]\n");
      return 1;
    }
  if (argn > 2)
    input->result = (char *) xmlStrdup ((xmlChar *) argc[2]);
  if (argn == 4)
    input->variables = (char *) xmlStrdup ((xmlChar *) argc[3]);

  // Making optimization
#if DEBUG
  fprintf (stderr, "main: making optimization\n");
#endif
  if (input_open (argc[1]))
    optimize_open ();

  // Freeing memory
#if DEBUG
  fprintf (stderr, "main: freeing memory and closing\n");
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
