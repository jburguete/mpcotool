#include <stdio.h>
#include <math.h>

static inline double
Ackley (double x, double y)
{
  return 20. * (1. - exp (-0.2 * sqrt (0.5 * (x * x + y * y)))) + M_E
    - exp (0.5 * (cos (2. * M_PI * x) + cos (2. * M_PI * y)));
}

int
main (int argn __attribute__ ((unused)), char **argc)
{
  FILE *file;
  double x, y;
  file = fopen (argc[1], "r");
  if (fscanf (file, "%*s%lf%*s%lf", &x, &y) != 2)
    return 1;
  fclose (file);
  file = fopen (argc[2], "w");
  fprintf (file, "%.14le", Ackley (x - M_PI_4, y - M_PI_4));
  fclose (file);
  return 0;
}
