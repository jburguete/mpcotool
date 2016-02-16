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
 * \file optimize.c
 * \brief Source file to define the optimization functions.
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
#ifdef G_OS_WIN32
#include <windows.h>
#elif !defined(__BSD_VISIBLE)
#include <alloca.h>
#endif
#if HAVE_MPI
#include <mpi.h>
#endif
#include "genetic/genetic.h"
#include "utils.h"
#include "experiment.h"
#include "variable.h"
#include "input.h"
#include "optimize.h"

#define DEBUG_OPTIMIZE 0        ///< Macro to debug optimize functions.

/**
 * \def RM
 * \brief Macro to define the shell remove command.
 */
#ifdef G_OS_WIN32
#define RM "del"
#else
#define RM "rm"
#endif

int ntasks;                     ///< Number of tasks.
unsigned int nthreads;          ///< Number of threads.
unsigned int nthreads_direction;
  ///< Number of threads for the direction search method.
GMutex mutex[1];                ///< Mutex struct.
void (*optimize_algorithm) ();
  ///< Pointer to the function to perform a optimization algorithm step.
double (*optimize_estimate_direction) (unsigned int variable,
                                       unsigned int estimate);
  ///< Pointer to the function to estimate the direction.
double (*optimize_norm) (unsigned int simulation);
  ///< Pointer to the error norm function.
Optimize optimize[1];           ///< Optimization data.

/**
 * \fn void optimize_input (unsigned int simulation, char *input, \
 *   GMappedFile *template)
 * \brief Function to write the simulation input file.
 * \param simulation
 * \brief Simulation number.
 * \param input
 * \brief Input file name.
 * \param template
 * \brief Template of the input file name.
 */
void
optimize_input (unsigned int simulation, char *input, GMappedFile * template)
{
  unsigned int i;
  char buffer[32], value[32], *buffer2, *buffer3, *content;
  FILE *file;
  gsize length;
  GRegex *regex;

#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_input: start\n");
#endif

  // Checking the file
  if (!template)
    goto optimize_input_end;

  // Opening template
  content = g_mapped_file_get_contents (template);
  length = g_mapped_file_get_length (template);
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_input: length=%lu\ncontent:\n%s", length, content);
#endif
  file = g_fopen (input, "w");

  // Parsing template
  for (i = 0; i < optimize->nvariables; ++i)
    {
#if DEBUG_OPTIMIZE
      fprintf (stderr, "optimize_input: variable=%u\n", i);
#endif
      snprintf (buffer, 32, "@variable%u@", i + 1);
      regex = g_regex_new (buffer, 0, 0, NULL);
      if (i == 0)
        {
          buffer2 = g_regex_replace_literal (regex, content, length, 0,
                                             optimize->label[i], 0, NULL);
#if DEBUG_OPTIMIZE
          fprintf (stderr, "optimize_input: buffer2\n%s", buffer2);
#endif
        }
      else
        {
          length = strlen (buffer3);
          buffer2 = g_regex_replace_literal (regex, buffer3, length, 0,
                                             optimize->label[i], 0, NULL);
          g_free (buffer3);
        }
      g_regex_unref (regex);
      length = strlen (buffer2);
      snprintf (buffer, 32, "@value%u@", i + 1);
      regex = g_regex_new (buffer, 0, 0, NULL);
      snprintf (value, 32, format[optimize->precision[i]],
                optimize->value[simulation * optimize->nvariables + i]);

#if DEBUG_OPTIMIZE
      fprintf (stderr, "optimize_input: value=%s\n", value);
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

optimize_input_end:
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_input: end\n");
#endif
  return;
}

/**
 * \fn double optimize_parse (unsigned int simulation, unsigned int experiment)
 * \brief Function to parse input files, simulating and calculating the \
 *   objective function.
 * \param simulation
 * \brief Simulation number.
 * \param experiment
 * \brief Experiment number.
 * \return Objective function value.
 */
double
optimize_parse (unsigned int simulation, unsigned int experiment)
{
  unsigned int i;
  double e;
  char buffer[512], input[MAX_NINPUTS][32], output[32], result[32], *buffer2,
    *buffer3, *buffer4;
  FILE *file_result;

#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_parse: start\n");
  fprintf (stderr, "optimize_parse: simulation=%u experiment=%u\n", simulation,
           experiment);
#endif

  // Opening input files
  for (i = 0; i < optimize->ninputs; ++i)
    {
      snprintf (&input[i][0], 32, "input-%u-%u-%u", i, simulation, experiment);
#if DEBUG_OPTIMIZE
      fprintf (stderr, "optimize_parse: i=%u input=%s\n", i, &input[i][0]);
#endif
      optimize_input (simulation, &input[i][0], optimize->file[i][experiment]);
    }
  for (; i < MAX_NINPUTS; ++i)
    strcpy (&input[i][0], "");
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_parse: parsing end\n");
#endif

  // Performing the simulation
  snprintf (output, 32, "output-%u-%u", simulation, experiment);
  buffer2 = g_path_get_dirname (optimize->simulator);
  buffer3 = g_path_get_basename (optimize->simulator);
  buffer4 = g_build_filename (buffer2, buffer3, NULL);
  snprintf (buffer, 512, "\"%s\" %s %s %s %s %s %s %s %s %s",
            buffer4, input[0], input[1], input[2], input[3], input[4], input[5],
            input[6], input[7], output);
  g_free (buffer4);
  g_free (buffer3);
  g_free (buffer2);
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_parse: %s\n", buffer);
#endif
  system (buffer);

  // Checking the objective value function
  if (optimize->evaluator)
    {
      snprintf (result, 32, "result-%u-%u", simulation, experiment);
      buffer2 = g_path_get_dirname (optimize->evaluator);
      buffer3 = g_path_get_basename (optimize->evaluator);
      buffer4 = g_build_filename (buffer2, buffer3, NULL);
      snprintf (buffer, 512, "\"%s\" %s %s %s",
                buffer4, output, optimize->experiment[experiment], result);
      g_free (buffer4);
      g_free (buffer3);
      g_free (buffer2);
#if DEBUG_OPTIMIZE
      fprintf (stderr, "optimize_parse: %s\n", buffer);
#endif
      system (buffer);
      file_result = g_fopen (result, "r");
      e = atof (fgets (buffer, 512, file_result));
      fclose (file_result);
    }
  else
    {
      strcpy (result, "");
      file_result = g_fopen (output, "r");
      e = atof (fgets (buffer, 512, file_result));
      fclose (file_result);
    }

  // Removing files
#if !DEBUG_OPTIMIZE
  for (i = 0; i < optimize->ninputs; ++i)
    {
      if (optimize->file[i][0])
        {
          snprintf (buffer, 512, RM " %s", &input[i][0]);
          system (buffer);
        }
    }
  snprintf (buffer, 512, RM " %s %s", output, result);
  system (buffer);
#endif

  // Processing pending events
  show_pending ();

#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_parse: end\n");
#endif

  // Returning the objective function
  return e * optimize->weight[experiment];
}

/**
 * \fn double optimize_norm_euclidian (unsigned int simulation)
 * \brief Function to calculate the Euclidian error norm.
 * \param simulation
 * \brief simulation number.
 * \return Euclidian error norm.
 */
double
optimize_norm_euclidian (unsigned int simulation)
{
  double e, ei;
  unsigned int i;
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_norm_euclidian: start\n");
#endif
  e = 0.;
  for (i = 0; i < optimize->nexperiments; ++i)
    {
      ei = optimize_parse (simulation, i);
      e += ei * ei;
    }
  e = sqrt (e);
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_norm_euclidian: error=%lg\n", e);
  fprintf (stderr, "optimize_norm_euclidian: end\n");
#endif
  return e;
}

/**
 * \fn double optimize_norm_maximum (unsigned int simulation)
 * \brief Function to calculate the maximum error norm.
 * \param simulation
 * \brief simulation number.
 * \return Maximum error norm.
 */
double
optimize_norm_maximum (unsigned int simulation)
{
  double e, ei;
  unsigned int i;
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_norm_maximum: start\n");
#endif
  e = 0.;
  for (i = 0; i < optimize->nexperiments; ++i)
    {
      ei = fabs (optimize_parse (simulation, i));
      e = fmax (e, ei);
    }
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_norm_maximum: error=%lg\n", e);
  fprintf (stderr, "optimize_norm_maximum: end\n");
#endif
  return e;
}

/**
 * \fn double optimize_norm_p (unsigned int simulation)
 * \brief Function to calculate the P error norm.
 * \param simulation
 * \brief simulation number.
 * \return P error norm.
 */
double
optimize_norm_p (unsigned int simulation)
{
  double e, ei;
  unsigned int i;
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_norm_p: start\n");
#endif
  e = 0.;
  for (i = 0; i < optimize->nexperiments; ++i)
    {
      ei = fabs (optimize_parse (simulation, i));
      e += pow (ei, optimize->p);
    }
  e = pow (e, 1. / optimize->p);
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_norm_p: error=%lg\n", e);
  fprintf (stderr, "optimize_norm_p: end\n");
#endif
  return e;
}

/**
 * \fn double optimize_norm_taxicab (unsigned int simulation)
 * \brief Function to calculate the taxicab error norm.
 * \param simulation
 * \brief simulation number.
 * \return Taxicab error norm.
 */
double
optimize_norm_taxicab (unsigned int simulation)
{
  double e;
  unsigned int i;
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_norm_taxicab: start\n");
#endif
  e = 0.;
  for (i = 0; i < optimize->nexperiments; ++i)
    e += fabs (optimize_parse (simulation, i));
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_norm_taxicab: error=%lg\n", e);
  fprintf (stderr, "optimize_norm_taxicab: end\n");
#endif
  return e;
}

/**
 * \fn void optimize_print ()
 * \brief Function to print the results.
 */
void
optimize_print ()
{
  unsigned int i;
  char buffer[512];
#if HAVE_MPI
  if (optimize->mpi_rank)
    return;
#endif
  printf ("%s\n", gettext ("Best result"));
  fprintf (optimize->file_result, "%s\n", gettext ("Best result"));
  printf ("error = %.15le\n", optimize->error_old[0]);
  fprintf (optimize->file_result, "error = %.15le\n", optimize->error_old[0]);
  for (i = 0; i < optimize->nvariables; ++i)
    {
      snprintf (buffer, 512, "%s = %s\n",
                optimize->label[i], format[optimize->precision[i]]);
      printf (buffer, optimize->value_old[i]);
      fprintf (optimize->file_result, buffer, optimize->value_old[i]);
    }
  fflush (optimize->file_result);
}

/**
 * \fn void optimize_save_variables (unsigned int simulation, double error)
 * \brief Function to save in a file the variables and the error.
 * \param simulation
 * \brief Simulation number.
 * \param error
 * \brief Error value.
 */
void
optimize_save_variables (unsigned int simulation, double error)
{
  unsigned int i;
  char buffer[64];
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_save_variables: start\n");
#endif
  for (i = 0; i < optimize->nvariables; ++i)
    {
      snprintf (buffer, 64, "%s ", format[optimize->precision[i]]);
      fprintf (optimize->file_variables, buffer,
               optimize->value[simulation * optimize->nvariables + i]);
    }
  fprintf (optimize->file_variables, "%.14le\n", error);
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_save_variables: end\n");
#endif
}

/**
 * \fn void optimize_best (unsigned int simulation, double value)
 * \brief Function to save the best simulations.
 * \param simulation
 * \brief Simulation number.
 * \param value
 * \brief Objective function value.
 */
void
optimize_best (unsigned int simulation, double value)
{
  unsigned int i, j;
  double e;
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_best: start\n");
  fprintf (stderr, "optimize_best: nsaveds=%u nbest=%u\n",
           optimize->nsaveds, optimize->nbest);
#endif
  if (optimize->nsaveds < optimize->nbest
      || value < optimize->error_best[optimize->nsaveds - 1])
    {
      if (optimize->nsaveds < optimize->nbest)
        ++optimize->nsaveds;
      optimize->error_best[optimize->nsaveds - 1] = value;
      optimize->simulation_best[optimize->nsaveds - 1] = simulation;
      for (i = optimize->nsaveds; --i;)
        {
          if (optimize->error_best[i] < optimize->error_best[i - 1])
            {
              j = optimize->simulation_best[i];
              e = optimize->error_best[i];
              optimize->simulation_best[i] = optimize->simulation_best[i - 1];
              optimize->error_best[i] = optimize->error_best[i - 1];
              optimize->simulation_best[i - 1] = j;
              optimize->error_best[i - 1] = e;
            }
          else
            break;
        }
    }
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_best: end\n");
#endif
}

/**
 * \fn void optimize_sequential ()
 * \brief Function to optimize sequentially.
 */
void
optimize_sequential ()
{
  unsigned int i;
  double e;
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_sequential: start\n");
  fprintf (stderr, "optimize_sequential: nstart=%u nend=%u\n",
           optimize->nstart, optimize->nend);
#endif
  for (i = optimize->nstart; i < optimize->nend; ++i)
    {
      e = optimize_norm (i);
      optimize_best (i, e);
      optimize_save_variables (i, e);
      if (e < optimize->threshold)
        {
          optimize->stop = 1;
          break;
        }
#if DEBUG_OPTIMIZE
      fprintf (stderr, "optimize_sequential: i=%u e=%lg\n", i, e);
#endif
    }
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_sequential: end\n");
#endif
}

/**
 * \fn void* optimize_thread (ParallelData *data)
 * \brief Function to optimize on a thread.
 * \param data
 * \brief Function data.
 * \return NULL
 */
void *
optimize_thread (ParallelData * data)
{
  unsigned int i, thread;
  double e;
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_thread: start\n");
#endif
  thread = data->thread;
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_thread: thread=%u start=%u end=%u\n", thread,
           optimize->thread[thread], optimize->thread[thread + 1]);
#endif
  for (i = optimize->thread[thread]; i < optimize->thread[thread + 1]; ++i)
    {
      e = optimize_norm (i);
      g_mutex_lock (mutex);
      optimize_best (i, e);
      optimize_save_variables (i, e);
      if (e < optimize->threshold)
        optimize->stop = 1;
      g_mutex_unlock (mutex);
      if (optimize->stop)
        break;
#if DEBUG_OPTIMIZE
      fprintf (stderr, "optimize_thread: i=%u e=%lg\n", i, e);
#endif
    }
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_thread: end\n");
#endif
  g_thread_exit (NULL);
  return NULL;
}

/**
 * \fn void optimize_merge (unsigned int nsaveds, \
 *   unsigned int *simulation_best, double *error_best)
 * \brief Function to merge the 2 optimization results.
 * \param nsaveds
 * \brief Number of saved results.
 * \param simulation_best
 * \brief Array of best simulation numbers.
 * \param error_best
 * \brief Array of best objective function values.
 */
void
optimize_merge (unsigned int nsaveds, unsigned int *simulation_best,
                double *error_best)
{
  unsigned int i, j, k, s[optimize->nbest];
  double e[optimize->nbest];
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_merge: start\n");
#endif
  i = j = k = 0;
  do
    {
      if (i == optimize->nsaveds)
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
          s[k] = optimize->simulation_best[i];
          e[k] = optimize->error_best[i];
          ++i;
          ++k;
          if (i == optimize->nsaveds)
            break;
        }
      else if (optimize->error_best[i] > error_best[j])
        {
          s[k] = simulation_best[j];
          e[k] = error_best[j];
          ++j;
          ++k;
        }
      else
        {
          s[k] = optimize->simulation_best[i];
          e[k] = optimize->error_best[i];
          ++i;
          ++k;
        }
    }
  while (k < optimize->nbest);
  optimize->nsaveds = k;
  memcpy (optimize->simulation_best, s, k * sizeof (unsigned int));
  memcpy (optimize->error_best, e, k * sizeof (double));
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_merge: end\n");
#endif
}

/**
 * \fn void optimize_synchronise ()
 * \brief Function to synchronise the optimization results of MPI tasks.
 */
#if HAVE_MPI
void
optimize_synchronise ()
{
  unsigned int i, nsaveds, simulation_best[optimize->nbest], stop;
  double error_best[optimize->nbest];
  MPI_Status mpi_stat;
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_synchronise: start\n");
#endif
  if (optimize->mpi_rank == 0)
    {
      for (i = 1; i < ntasks; ++i)
        {
          MPI_Recv (&nsaveds, 1, MPI_INT, i, 1, MPI_COMM_WORLD, &mpi_stat);
          MPI_Recv (simulation_best, nsaveds, MPI_INT, i, 1,
                    MPI_COMM_WORLD, &mpi_stat);
          MPI_Recv (error_best, nsaveds, MPI_DOUBLE, i, 1,
                    MPI_COMM_WORLD, &mpi_stat);
          optimize_merge (nsaveds, simulation_best, error_best);
          MPI_Recv (&stop, 1, MPI_UNSIGNED, i, 1, MPI_COMM_WORLD, &mpi_stat);
          if (stop)
            optimize->stop = 1;
        }
      for (i = 1; i < ntasks; ++i)
        MPI_Send (&optimize->stop, 1, MPI_UNSIGNED, i, 1, MPI_COMM_WORLD);
    }
  else
    {
      MPI_Send (&optimize->nsaveds, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
      MPI_Send (optimize->simulation_best, optimize->nsaveds, MPI_INT, 0, 1,
                MPI_COMM_WORLD);
      MPI_Send (optimize->error_best, optimize->nsaveds, MPI_DOUBLE, 0, 1,
                MPI_COMM_WORLD);
      MPI_Send (&optimize->stop, 1, MPI_UNSIGNED, 0, 1, MPI_COMM_WORLD);
      MPI_Recv (&stop, 1, MPI_UNSIGNED, 0, 1, MPI_COMM_WORLD, &mpi_stat);
      if (stop)
        optimize->stop = 1;
    }
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_synchronise: end\n");
#endif
}
#endif

/**
 * \fn void optimize_sweep ()
 * \brief Function to optimize with the sweep algorithm.
 */
void
optimize_sweep ()
{
  unsigned int i, j, k, l;
  double e;
  GThread *thread[nthreads];
  ParallelData data[nthreads];
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_sweep: start\n");
#endif
  for (i = 0; i < optimize->nsimulations; ++i)
    {
      k = i;
      for (j = 0; j < optimize->nvariables; ++j)
        {
          l = k % optimize->nsweeps[j];
          k /= optimize->nsweeps[j];
          e = optimize->rangemin[j];
          if (optimize->nsweeps[j] > 1)
            e += l * (optimize->rangemax[j] - optimize->rangemin[j])
              / (optimize->nsweeps[j] - 1);
          optimize->value[i * optimize->nvariables + j] = e;
        }
    }
  optimize->nsaveds = 0;
  if (nthreads <= 1)
    optimize_sequential ();
  else
    {
      for (i = 0; i < nthreads; ++i)
        {
          data[i].thread = i;
          thread[i] = g_thread_new (NULL, (void (*)) optimize_thread, &data[i]);
        }
      for (i = 0; i < nthreads; ++i)
        g_thread_join (thread[i]);
    }
#if HAVE_MPI
  // Communicating tasks results
  optimize_synchronise ();
#endif
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_sweep: end\n");
#endif
}

/**
 * \fn void optimize_MonteCarlo ()
 * \brief Function to optimize with the Monte-Carlo algorithm.
 */
void
optimize_MonteCarlo ()
{
  unsigned int i, j;
  GThread *thread[nthreads];
  ParallelData data[nthreads];
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_MonteCarlo: start\n");
#endif
  for (i = 0; i < optimize->nsimulations; ++i)
    for (j = 0; j < optimize->nvariables; ++j)
      optimize->value[i * optimize->nvariables + j]
        = optimize->rangemin[j] + gsl_rng_uniform (optimize->rng)
        * (optimize->rangemax[j] - optimize->rangemin[j]);
  optimize->nsaveds = 0;
  if (nthreads <= 1)
    optimize_sequential ();
  else
    {
      for (i = 0; i < nthreads; ++i)
        {
          data[i].thread = i;
          thread[i] = g_thread_new (NULL, (void (*)) optimize_thread, &data[i]);
        }
      for (i = 0; i < nthreads; ++i)
        g_thread_join (thread[i]);
    }
#if HAVE_MPI
  // Communicating tasks results
  optimize_synchronise ();
#endif
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_MonteCarlo: end\n");
#endif
}

/**
 * \fn void optimize_best_direction (unsigned int simulation, \
 *   double value)
 * \brief Function to save the best simulation in a direction search method.
 * \param simulation
 * \brief Simulation number.
 * \param value
 * \brief Objective function value.
 */
void
optimize_best_direction (unsigned int simulation, double value)
{
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_best_direction: start\n");
  fprintf (stderr,
           "optimize_best_direction: simulation=%u value=%.14le best=%.14le\n",
           simulation, value, optimize->error_best[0]);
#endif
  if (value < optimize->error_best[0])
    {
      optimize->error_best[0] = value;
      optimize->simulation_best[0] = simulation;
#if DEBUG_OPTIMIZE
      fprintf (stderr,
               "optimize_best_direction: BEST simulation=%u value=%.14le\n",
               simulation, value);
#endif
    }
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_best_direction: end\n");
#endif
}

/**
 * \fn void optimize_direction_sequential (unsigned int simulation)
 * \brief Function to estimate the direction search sequentially.
 * \param simulation
 * \brief Simulation number.
 */
void
optimize_direction_sequential (unsigned int simulation)
{
  unsigned int i, j;
  double e;
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_direction_sequential: start\n");
  fprintf (stderr, "optimize_direction_sequential: nstart_direction=%u "
           "nend_direction=%u\n",
           optimize->nstart_direction, optimize->nend_direction);
#endif
  for (i = optimize->nstart_direction; i < optimize->nend_direction; ++i)
    {
      j = simulation + i;
      e = optimize_norm (j);
      optimize_best_direction (j, e);
      optimize_save_variables (j, e);
      if (e < optimize->threshold)
        {
          optimize->stop = 1;
          break;
        }
#if DEBUG_OPTIMIZE
      fprintf (stderr, "optimize_direction_sequential: i=%u e=%lg\n", i, e);
#endif
    }
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_direction_sequential: end\n");
#endif
}

/**
 * \fn void* optimize_direction_thread (ParallelData *data)
 * \brief Function to estimate the direction search on a thread.
 * \param data
 * \brief Function data.
 * \return NULL
 */
void *
optimize_direction_thread (ParallelData * data)
{
  unsigned int i, thread;
  double e;
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_direction_thread: start\n");
#endif
  thread = data->thread;
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_direction_thread: thread=%u start=%u end=%u\n",
           thread,
           optimize->thread_direction[thread],
           optimize->thread_direction[thread + 1]);
#endif
  for (i = optimize->thread_direction[thread];
       i < optimize->thread_direction[thread + 1]; ++i)
    {
      e = optimize_norm (i);
      g_mutex_lock (mutex);
      optimize_best_direction (i, e);
      optimize_save_variables (i, e);
      if (e < optimize->threshold)
        optimize->stop = 1;
      g_mutex_unlock (mutex);
      if (optimize->stop)
        break;
#if DEBUG_OPTIMIZE
      fprintf (stderr, "optimize_direction_thread: i=%u e=%lg\n", i, e);
#endif
    }
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_direction_thread: end\n");
#endif
  g_thread_exit (NULL);
  return NULL;
}

/**
 * \fn double optimize_estimate_direction_random (unsigned int variable, \
 *   unsigned int estimate)
 * \brief Function to estimate a component of the direction search vector.
 * \param variable
 * \brief Variable number.
 * \param estimate
 * \brief Estimate number.
 */
double
optimize_estimate_direction_random (unsigned int variable,
                                    unsigned int estimate)
{
  double x;
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_estimate_direction_random: start\n");
#endif
  x = optimize->direction[variable]
    + (1. - 2. * gsl_rng_uniform (optimize->rng)) * optimize->step[variable];
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_estimate_direction_random: direction%u=%lg\n",
           variable, x);
  fprintf (stderr, "optimize_estimate_direction_random: end\n");
#endif
  return x;
}

/**
 * \fn double optimize_estimate_direction_coordinates (unsigned int variable, \
 *   unsigned int estimate)
 * \brief Function to estimate a component of the direction search vector.
 * \param variable
 * \brief Variable number.
 * \param estimate
 * \brief Estimate number.
 */
double
optimize_estimate_direction_coordinates (unsigned int variable,
                                         unsigned int estimate)
{
  double x;
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_estimate_direction_coordinates: start\n");
#endif
  x = optimize->direction[variable];
  if (estimate >= (2 * variable) && estimate < (2 * variable + 2))
    {
      if (estimate & 1)
        x += optimize->step[variable];
      else
        x -= optimize->step[variable];
    }
#if DEBUG_OPTIMIZE
  fprintf (stderr,
           "optimize_estimate_direction_coordinates: direction%u=%lg\n",
           variable, x);
  fprintf (stderr, "optimize_estimate_direction_coordinates: end\n");
#endif
  return x;
}

/**
 * \fn void optimize_step_direction (unsigned int simulation)
 * \brief Function to do a step of the direction search method.
 * \param simulation
 * \brief Simulation number.
 */
void
optimize_step_direction (unsigned int simulation)
{
  GThread *thread[nthreads_direction];
  ParallelData data[nthreads_direction];
  unsigned int i, j, k, b;
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_step_direction: start\n");
#endif
  for (i = 0; i < optimize->nestimates; ++i)
    {
      k = (simulation + i) * optimize->nvariables;
      b = optimize->simulation_best[0] * optimize->nvariables;
#if DEBUG_OPTIMIZE
      fprintf (stderr, "optimize_step_direction: simulation=%u best=%u\n",
               simulation + i, optimize->simulation_best[0]);
#endif
      for (j = 0; j < optimize->nvariables; ++j, ++k, ++b)
        {
#if DEBUG_OPTIMIZE
          fprintf (stderr,
                   "optimize_step_direction: estimate=%u best%u=%.14le\n",
                   i, j, optimize->value[b]);
#endif
          optimize->value[k]
            = optimize->value[b] + optimize_estimate_direction (j, i);
          optimize->value[k] = fmin (fmax (optimize->value[k],
                                           optimize->rangeminabs[j]),
                                     optimize->rangemaxabs[j]);
#if DEBUG_OPTIMIZE
          fprintf (stderr,
                   "optimize_step_direction: estimate=%u variable%u=%.14le\n",
                   i, j, optimize->value[k]);
#endif
        }
    }
  if (nthreads_direction == 1)
    optimize_direction_sequential (simulation);
  else
    {
      for (i = 0; i <= nthreads_direction; ++i)
        {
          optimize->thread_direction[i]
            = simulation + optimize->nstart_direction
            + i * (optimize->nend_direction - optimize->nstart_direction)
            / nthreads_direction;
#if DEBUG_OPTIMIZE
          fprintf (stderr,
                   "optimize_step_direction: i=%u thread_direction=%u\n",
                   i, optimize->thread_direction[i]);
#endif
        }
      for (i = 0; i < nthreads_direction; ++i)
        {
          data[i].thread = i;
          thread[i] = g_thread_new
            (NULL, (void (*)) optimize_direction_thread, &data[i]);
        }
      for (i = 0; i < nthreads_direction; ++i)
        g_thread_join (thread[i]);
    }
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_step_direction: end\n");
#endif
}

/**
 * \fn void optimize_direction ()
 * \brief Function to optimize with a direction search method.
 */
void
optimize_direction ()
{
  unsigned int i, j, k, b, s, adjust;
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_direction: start\n");
#endif
  for (i = 0; i < optimize->nvariables; ++i)
    optimize->direction[i] = 0.;
  b = optimize->simulation_best[0] * optimize->nvariables;
  s = optimize->nsimulations;
  adjust = 1;
  for (i = 0; i < optimize->nsteps; ++i, s += optimize->nestimates, b = k)
    {
#if DEBUG_OPTIMIZE
      fprintf (stderr, "optimize_direction: step=%u old_best=%u\n",
               i, optimize->simulation_best[0]);
#endif
      optimize_step_direction (s);
      k = optimize->simulation_best[0] * optimize->nvariables;
#if DEBUG_OPTIMIZE
      fprintf (stderr, "optimize_direction: step=%u best=%u\n",
               i, optimize->simulation_best[0]);
#endif
      if (k == b)
        {
          if (adjust)
            for (j = 0; j < optimize->nvariables; ++j)
              optimize->step[j] *= 0.5;
          for (j = 0; j < optimize->nvariables; ++j)
            optimize->direction[j] = 0.;
          adjust = 1;
        }
      else
        {
          for (j = 0; j < optimize->nvariables; ++j)
            {
#if DEBUG_OPTIMIZE
              fprintf (stderr,
                       "optimize_direction: best%u=%.14le old%u=%.14le\n",
                       j, optimize->value[k + j], j, optimize->value[b + j]);
#endif
              optimize->direction[j]
                = (1. - optimize->relaxation) * optimize->direction[j]
                + optimize->relaxation
                * (optimize->value[k + j] - optimize->value[b + j]);
#if DEBUG_OPTIMIZE
              fprintf (stderr, "optimize_direction: direction%u=%.14le\n",
                       j, optimize->direction[j]);
#endif
            }
          adjust = 0;
        }
    }
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_direction: end\n");
#endif
}

/**
 * \fn double optimize_genetic_objective (Entity *entity)
 * \brief Function to calculate the objective function of an entity.
 * \param entity
 * \brief entity data.
 * \return objective function value.
 */
double
optimize_genetic_objective (Entity * entity)
{
  unsigned int j;
  double objective;
  char buffer[64];
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_genetic_objective: start\n");
#endif
  for (j = 0; j < optimize->nvariables; ++j)
    {
      optimize->value[entity->id * optimize->nvariables + j]
        = genetic_get_variable (entity, optimize->genetic_variable + j);
    }
  objective = optimize_norm (entity->id);
  g_mutex_lock (mutex);
  for (j = 0; j < optimize->nvariables; ++j)
    {
      snprintf (buffer, 64, "%s ", format[optimize->precision[j]]);
      fprintf (optimize->file_variables, buffer,
               genetic_get_variable (entity, optimize->genetic_variable + j));
    }
  fprintf (optimize->file_variables, "%.14le\n", objective);
  g_mutex_unlock (mutex);
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_genetic_objective: end\n");
#endif
  return objective;
}

/**
 * \fn void optimize_genetic ()
 * \brief Function to optimize with the genetic algorithm.
 */
void
optimize_genetic ()
{
  char *best_genome;
  double best_objective, *best_variable;
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_genetic: start\n");
  fprintf (stderr, "optimize_genetic: ntasks=%u nthreads=%u\n", ntasks,
           nthreads);
  fprintf (stderr,
           "optimize_genetic: nvariables=%u population=%u generations=%u\n",
           optimize->nvariables, optimize->nsimulations, optimize->niterations);
  fprintf (stderr,
           "optimize_genetic: mutation=%lg reproduction=%lg adaptation=%lg\n",
           optimize->mutation_ratio, optimize->reproduction_ratio,
           optimize->adaptation_ratio);
#endif
  genetic_algorithm_default (optimize->nvariables,
                             optimize->genetic_variable,
                             optimize->nsimulations,
                             optimize->niterations,
                             optimize->mutation_ratio,
                             optimize->reproduction_ratio,
                             optimize->adaptation_ratio,
                             optimize->seed,
                             optimize->threshold,
                             &optimize_genetic_objective,
                             &best_genome, &best_variable, &best_objective);
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_genetic: the best\n");
#endif
  optimize->error_old = (double *) g_malloc (sizeof (double));
  optimize->value_old
    = (double *) g_malloc (optimize->nvariables * sizeof (double));
  optimize->error_old[0] = best_objective;
  memcpy (optimize->value_old, best_variable,
          optimize->nvariables * sizeof (double));
  g_free (best_genome);
  g_free (best_variable);
  optimize_print ();
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_genetic: end\n");
#endif
}

/**
 * \fn void optimize_save_old ()
 * \brief Function to save the best results on iterative methods.
 */
void
optimize_save_old ()
{
  unsigned int i, j;
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_save_old: start\n");
  fprintf (stderr, "optimize_save_old: nsaveds=%u\n", optimize->nsaveds);
#endif
  memcpy (optimize->error_old, optimize->error_best,
          optimize->nbest * sizeof (double));
  for (i = 0; i < optimize->nbest; ++i)
    {
      j = optimize->simulation_best[i];
#if DEBUG_OPTIMIZE
      fprintf (stderr, "optimize_save_old: i=%u j=%u\n", i, j);
#endif
      memcpy (optimize->value_old + i * optimize->nvariables,
              optimize->value + j * optimize->nvariables,
              optimize->nvariables * sizeof (double));
    }
#if DEBUG_OPTIMIZE
  for (i = 0; i < optimize->nvariables; ++i)
    fprintf (stderr, "optimize_save_old: best variable %u=%lg\n",
             i, optimize->value_old[i]);
  fprintf (stderr, "optimize_save_old: end\n");
#endif
}

/**
 * \fn void optimize_merge_old ()
 * \brief Function to merge the best results with the previous step best results
 *   on iterative methods.
 */
void
optimize_merge_old ()
{
  unsigned int i, j, k;
  double v[optimize->nbest * optimize->nvariables], e[optimize->nbest],
    *enew, *eold;
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_merge_old: start\n");
#endif
  enew = optimize->error_best;
  eold = optimize->error_old;
  i = j = k = 0;
  do
    {
      if (*enew < *eold)
        {
          memcpy (v + k * optimize->nvariables,
                  optimize->value
                  + optimize->simulation_best[i] * optimize->nvariables,
                  optimize->nvariables * sizeof (double));
          e[k] = *enew;
          ++k;
          ++enew;
          ++i;
        }
      else
        {
          memcpy (v + k * optimize->nvariables,
                  optimize->value_old + j * optimize->nvariables,
                  optimize->nvariables * sizeof (double));
          e[k] = *eold;
          ++k;
          ++eold;
          ++j;
        }
    }
  while (k < optimize->nbest);
  memcpy (optimize->value_old, v, k * optimize->nvariables * sizeof (double));
  memcpy (optimize->error_old, e, k * sizeof (double));
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_merge_old: end\n");
#endif
}

/**
 * \fn void optimize_refine ()
 * \brief Function to refine the search ranges of the variables in iterative
 *   algorithms.
 */
void
optimize_refine ()
{
  unsigned int i, j;
  double d;
#if HAVE_MPI
  MPI_Status mpi_stat;
#endif
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_refine: start\n");
#endif
#if HAVE_MPI
  if (!optimize->mpi_rank)
    {
#endif
      for (j = 0; j < optimize->nvariables; ++j)
        {
          optimize->rangemin[j] = optimize->rangemax[j]
            = optimize->value_old[j];
        }
      for (i = 0; ++i < optimize->nbest;)
        {
          for (j = 0; j < optimize->nvariables; ++j)
            {
              optimize->rangemin[j]
                = fmin (optimize->rangemin[j],
                        optimize->value_old[i * optimize->nvariables + j]);
              optimize->rangemax[j]
                = fmax (optimize->rangemax[j],
                        optimize->value_old[i * optimize->nvariables + j]);
            }
        }
      for (j = 0; j < optimize->nvariables; ++j)
        {
          d = optimize->tolerance
            * (optimize->rangemax[j] - optimize->rangemin[j]);
          switch (optimize->algorithm)
            {
            case ALGORITHM_MONTE_CARLO:
              d *= 0.5;
              break;
            default:
              if (optimize->nsweeps[j] > 1)
                d /= optimize->nsweeps[j] - 1;
              else
                d = 0.;
            }
          optimize->rangemin[j] -= d;
          optimize->rangemin[j]
            = fmax (optimize->rangemin[j], optimize->rangeminabs[j]);
          optimize->rangemax[j] += d;
          optimize->rangemax[j]
            = fmin (optimize->rangemax[j], optimize->rangemaxabs[j]);
          printf ("%s min=%lg max=%lg\n", optimize->label[j],
                  optimize->rangemin[j], optimize->rangemax[j]);
          fprintf (optimize->file_result, "%s min=%lg max=%lg\n",
                   optimize->label[j], optimize->rangemin[j],
                   optimize->rangemax[j]);
        }
#if HAVE_MPI
      for (i = 1; i < ntasks; ++i)
        {
          MPI_Send (optimize->rangemin, optimize->nvariables, MPI_DOUBLE, i,
                    1, MPI_COMM_WORLD);
          MPI_Send (optimize->rangemax, optimize->nvariables, MPI_DOUBLE, i,
                    1, MPI_COMM_WORLD);
        }
    }
  else
    {
      MPI_Recv (optimize->rangemin, optimize->nvariables, MPI_DOUBLE, 0, 1,
                MPI_COMM_WORLD, &mpi_stat);
      MPI_Recv (optimize->rangemax, optimize->nvariables, MPI_DOUBLE, 0, 1,
                MPI_COMM_WORLD, &mpi_stat);
    }
#endif
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_refine: end\n");
#endif
}

/**
 * \fn void optimize_step ()
 * \brief Function to do a step of the iterative algorithm.
 */
void
optimize_step ()
{
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_step: start\n");
#endif
  optimize_algorithm ();
  if (optimize->nsteps)
    optimize_direction ();
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_step: end\n");
#endif
}

/**
 * \fn void optimize_iterate ()
 * \brief Function to iterate the algorithm.
 */
void
optimize_iterate ()
{
  unsigned int i;
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_iterate: start\n");
#endif
  optimize->error_old = (double *) g_malloc (optimize->nbest * sizeof (double));
  optimize->value_old = (double *)
    g_malloc (optimize->nbest * optimize->nvariables * sizeof (double));
  optimize_step ();
  optimize_save_old ();
  optimize_refine ();
  optimize_print ();
  for (i = 1; i < optimize->niterations && !optimize->stop; ++i)
    {
      optimize_step ();
      optimize_merge_old ();
      optimize_refine ();
      optimize_print ();
    }
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_iterate: end\n");
#endif
}

/**
 * \fn void optimize_free ()
 * \brief Function to free the memory used by the Optimize struct.
 */
void
optimize_free ()
{
  unsigned int i, j;
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_free: start\n");
#endif
  for (j = 0; j < optimize->ninputs; ++j)
    {
      for (i = 0; i < optimize->nexperiments; ++i)
        g_mapped_file_unref (optimize->file[j][i]);
      g_free (optimize->file[j]);
    }
  g_free (optimize->error_old);
  g_free (optimize->value_old);
  g_free (optimize->value);
  g_free (optimize->genetic_variable);
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_free: end\n");
#endif
}

/**
 * \fn void optimize_open ()
 * \brief Function to open and perform a optimization.
 */
void
optimize_open ()
{
  GTimeZone *tz;
  GDateTime *t0, *t;
  unsigned int i, j;

#if DEBUG_OPTIMIZE
  char *buffer;
  fprintf (stderr, "optimize_open: start\n");
#endif

  // Getting initial time
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_open: getting initial time\n");
#endif
  tz = g_time_zone_new_utc ();
  t0 = g_date_time_new_now (tz);

  // Obtaining and initing the pseudo-random numbers generator seed
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_open: getting initial seed\n");
#endif
  if (optimize->seed == DEFAULT_RANDOM_SEED)
    optimize->seed = input->seed;
  gsl_rng_set (optimize->rng, optimize->seed);

  // Replacing the working directory
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_open: replacing the working directory\n");
#endif
  g_chdir (input->directory);

  // Getting results file names
  optimize->result = input->result;
  optimize->variables = input->variables;

  // Obtaining the simulator file
  optimize->simulator = input->simulator;

  // Obtaining the evaluator file
  optimize->evaluator = input->evaluator;

  // Reading the algorithm
  optimize->algorithm = input->algorithm;
  switch (optimize->algorithm)
    {
    case ALGORITHM_MONTE_CARLO:
      optimize_algorithm = optimize_MonteCarlo;
      break;
    case ALGORITHM_SWEEP:
      optimize_algorithm = optimize_sweep;
      break;
    default:
      optimize_algorithm = optimize_genetic;
      optimize->mutation_ratio = input->mutation_ratio;
      optimize->reproduction_ratio = input->reproduction_ratio;
      optimize->adaptation_ratio = input->adaptation_ratio;
    }
  optimize->nvariables = input->nvariables;
  optimize->nsimulations = input->nsimulations;
  optimize->niterations = input->niterations;
  optimize->nbest = input->nbest;
  optimize->tolerance = input->tolerance;
  optimize->nsteps = input->nsteps;
  optimize->nestimates = 0;
  optimize->threshold = input->threshold;
  optimize->stop = 0;
  if (input->nsteps)
    {
      optimize->relaxation = input->relaxation;
      switch (input->direction)
        {
        case DIRECTION_METHOD_COORDINATES:
          optimize->nestimates = 2 * optimize->nvariables;
          optimize_estimate_direction = optimize_estimate_direction_coordinates;
          break;
        default:
          optimize->nestimates = input->nestimates;
          optimize_estimate_direction = optimize_estimate_direction_random;
        }
    }

#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_open: nbest=%u\n", optimize->nbest);
#endif
  optimize->simulation_best
    = (unsigned int *) alloca (optimize->nbest * sizeof (unsigned int));
  optimize->error_best = (double *) alloca (optimize->nbest * sizeof (double));

  // Reading the experimental data
#if DEBUG_OPTIMIZE
  buffer = g_get_current_dir ();
  fprintf (stderr, "optimize_open: current directory=%s\n", buffer);
  g_free (buffer);
#endif
  optimize->nexperiments = input->nexperiments;
  optimize->ninputs = input->experiment->ninputs;
  optimize->experiment
    = (char **) alloca (input->nexperiments * sizeof (char *));
  optimize->weight = (double *) alloca (input->nexperiments * sizeof (double));
  for (i = 0; i < input->experiment->ninputs; ++i)
    optimize->file[i] = (GMappedFile **)
      g_malloc (input->nexperiments * sizeof (GMappedFile *));
  for (i = 0; i < input->nexperiments; ++i)
    {
#if DEBUG_OPTIMIZE
      fprintf (stderr, "optimize_open: i=%u\n", i);
#endif
      optimize->experiment[i] = input->experiment[i].name;
      optimize->weight[i] = input->experiment[i].weight;
#if DEBUG_OPTIMIZE
      fprintf (stderr, "optimize_open: experiment=%s weight=%lg\n",
               optimize->experiment[i], optimize->weight[i]);
#endif
      for (j = 0; j < input->experiment->ninputs; ++j)
        {
#if DEBUG_OPTIMIZE
          fprintf (stderr, "optimize_open: template%u\n", j + 1);
#endif
          optimize->file[j][i]
            = g_mapped_file_new (input->experiment[i].template[j], 0, NULL);
        }
    }

  // Reading the variables data
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_open: reading variables\n");
#endif
  optimize->label = (char **) alloca (input->nvariables * sizeof (char *));
  j = input->nvariables * sizeof (double);
  optimize->rangemin = (double *) alloca (j);
  optimize->rangeminabs = (double *) alloca (j);
  optimize->rangemax = (double *) alloca (j);
  optimize->rangemaxabs = (double *) alloca (j);
  optimize->step = (double *) alloca (j);
  j = input->nvariables * sizeof (unsigned int);
  optimize->precision = (unsigned int *) alloca (j);
  optimize->nsweeps = (unsigned int *) alloca (j);
  optimize->nbits = (unsigned int *) alloca (j);
  for (i = 0; i < input->nvariables; ++i)
    {
      optimize->label[i] = input->variable[i].name;
      optimize->rangemin[i] = input->variable[i].rangemin;
      optimize->rangeminabs[i] = input->variable[i].rangeminabs;
      optimize->rangemax[i] = input->variable[i].rangemax;
      optimize->rangemaxabs[i] = input->variable[i].rangemaxabs;
      optimize->precision[i] = input->variable[i].precision;
      optimize->step[i] = input->variable[i].step;
      optimize->nsweeps[i] = input->variable[i].nsweeps;
      optimize->nbits[i] = input->variable[i].nbits;
    }
  if (input->algorithm == ALGORITHM_SWEEP)
    {
      optimize->nsimulations = 1;
      for (i = 0; i < input->nvariables; ++i)
        {
          if (input->algorithm == ALGORITHM_SWEEP)
            {
              optimize->nsimulations *= optimize->nsweeps[i];
#if DEBUG_OPTIMIZE
              fprintf (stderr, "optimize_open: nsweeps=%u nsimulations=%u\n",
                       optimize->nsweeps[i], optimize->nsimulations);
#endif
            }
        }
    }
  if (optimize->nsteps)
    optimize->direction
      = (double *) alloca (optimize->nvariables * sizeof (double));

  // Setting error norm
  switch (input->norm)
    {
    case ERROR_NORM_EUCLIDIAN:
      optimize_norm = optimize_norm_euclidian;
      break;
    case ERROR_NORM_MAXIMUM:
      optimize_norm = optimize_norm_maximum;
      break;
    case ERROR_NORM_P:
      optimize_norm = optimize_norm_p;
      optimize->p = input->p;
      break;
    default:
      optimize_norm = optimize_norm_taxicab;
    }

  // Allocating values
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_open: allocating variables\n");
  fprintf (stderr, "optimize_open: nvariables=%u algorithm=%u\n",
           optimize->nvariables, optimize->algorithm);
#endif
  optimize->genetic_variable = NULL;
  if (optimize->algorithm == ALGORITHM_GENETIC)
    {
      optimize->genetic_variable = (GeneticVariable *)
        g_malloc (optimize->nvariables * sizeof (GeneticVariable));
      for (i = 0; i < optimize->nvariables; ++i)
        {
#if DEBUG_OPTIMIZE
          fprintf (stderr, "optimize_open: i=%u min=%lg max=%lg nbits=%u\n",
                   i, optimize->rangemin[i], optimize->rangemax[i],
                   optimize->nbits[i]);
#endif
          optimize->genetic_variable[i].minimum = optimize->rangemin[i];
          optimize->genetic_variable[i].maximum = optimize->rangemax[i];
          optimize->genetic_variable[i].nbits = optimize->nbits[i];
        }
    }
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_open: nvariables=%u nsimulations=%u\n",
           optimize->nvariables, optimize->nsimulations);
#endif
  optimize->value = (double *)
    g_malloc ((optimize->nsimulations
               + optimize->nestimates * optimize->nsteps)
              * optimize->nvariables * sizeof (double));

  // Calculating simulations to perform for each task
#if HAVE_MPI
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_open: rank=%u ntasks=%u\n",
           optimize->mpi_rank, ntasks);
#endif
  optimize->nstart = optimize->mpi_rank * optimize->nsimulations / ntasks;
  optimize->nend = (1 + optimize->mpi_rank) * optimize->nsimulations / ntasks;
  if (optimize->nsteps)
    {
      optimize->nstart_direction
        = optimize->mpi_rank * optimize->nestimates / ntasks;
      optimize->nend_direction
        = (1 + optimize->mpi_rank) * optimize->nestimates / ntasks;
    }
#else
  optimize->nstart = 0;
  optimize->nend = optimize->nsimulations;
  if (optimize->nsteps)
    {
      optimize->nstart_direction = 0;
      optimize->nend_direction = optimize->nestimates;
    }
#endif
#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_open: nstart=%u nend=%u\n", optimize->nstart,
           optimize->nend);
#endif

  // Calculating simulations to perform for each thread
  optimize->thread
    = (unsigned int *) alloca ((1 + nthreads) * sizeof (unsigned int));
  for (i = 0; i <= nthreads; ++i)
    {
      optimize->thread[i] = optimize->nstart
        + i * (optimize->nend - optimize->nstart) / nthreads;
#if DEBUG_OPTIMIZE
      fprintf (stderr, "optimize_open: i=%u thread=%u\n", i,
               optimize->thread[i]);
#endif
    }
  if (optimize->nsteps)
    optimize->thread_direction = (unsigned int *)
      alloca ((1 + nthreads_direction) * sizeof (unsigned int));

  // Opening result files
  optimize->file_result = g_fopen (optimize->result, "w");
  optimize->file_variables = g_fopen (optimize->variables, "w");

  // Performing the algorithm
  switch (optimize->algorithm)
    {
      // Genetic algorithm
    case ALGORITHM_GENETIC:
      optimize_genetic ();
      break;

      // Iterative algorithm
    default:
      optimize_iterate ();
    }

  // Getting calculation time
  t = g_date_time_new_now (tz);
  optimize->calculation_time = 0.000001 * g_date_time_difference (t, t0);
  g_date_time_unref (t);
  g_date_time_unref (t0);
  g_time_zone_unref (tz);
  printf ("%s = %.6lg s\n",
          gettext ("Calculation time"), optimize->calculation_time);
  fprintf (optimize->file_result, "%s = %.6lg s\n",
           gettext ("Calculation time"), optimize->calculation_time);

  // Closing result files
  fclose (optimize->file_variables);
  fclose (optimize->file_result);

#if DEBUG_OPTIMIZE
  fprintf (stderr, "optimize_open: end\n");
#endif
}