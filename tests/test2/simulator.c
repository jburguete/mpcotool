#define _GNU_SOURCE
#include <stdio.h>
#include <math.h>
#include <gsl/gsl_rng.h>
#include <glib.h>
#if HAVE_MPI
#include <mpi.h>
#endif
#include "genetic/genetic.h"

#define N_SIMULATIONS 10000
#define SEED 707l
int ntasks = 1;
unsigned int nthreads = 1;
GMutex mutex[1];
GeneticVariable v[2];

double
evaluate (Entity * entity)
{
  double x, y, e1, e2;
  x = genetic_get_variable (entity, v);
  y = genetic_get_variable (entity, v + 1);
  e1 = x + y - 3.;
  e2 = x - y - 1.;
  e1 = e1 * e1 + e2 * e2;
  return e1;
}

int
main (int argn, char **argc)
{
  FILE *file;
  double *best_variables;
  char *best_genome;
  double xgenerations, mutation_ratio, reproduction_ratio, adaptation_ratio,
    evolution_ratio, best_objective;
  int rank;
  unsigned int ngenerations;
#if HAVE_MPI
  MPI_Init (&argn, &argc);
  MPI_Comm_size (MPI_COMM_WORLD, &ntasks);
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);
#else
  rank = 0;
#endif
  nthreads = 4;
  v[0].maximum = 10.;
  v[0].minimum = -10.;
  v[0].nbits = 30;
  v[1].maximum = 10.;
  v[1].minimum = -10.;
  v[1].nbits = 30;
  file = fopen (argc[1], "r");
  if (fscanf (file, "%*s%lf%*s%lf%*s%lf%*s%lf", &xgenerations,
              &mutation_ratio, &reproduction_ratio, &adaptation_ratio) != 4)
    return 1;
  fclose (file);
  ngenerations = xgenerations;
  evolution_ratio = mutation_ratio + reproduction_ratio + adaptation_ratio;
  genetic_algorithm_default (2, v,
                             N_SIMULATIONS
                             / (1 + (ngenerations - 1) * evolution_ratio),
                             ngenerations, mutation_ratio, reproduction_ratio,
                             adaptation_ratio, SEED, 0., &evaluate,
                             &best_genome, &best_variables, &best_objective);
  if (rank == 0)
    {
      file = fopen (argc[2], "w");
      fprintf (file, "%.14le", best_objective);
      printf ("objective=%.14le\n", best_objective);
      fclose (file);
      g_free (best_genome);
      g_free (best_variables);
    }
#if HAVE_MPI
  MPI_Finalize ();
#endif
  return 0;
}
