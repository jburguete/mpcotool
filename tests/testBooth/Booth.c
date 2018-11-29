#include <stdio.h>
#include <math.h>

static inline double
Booth (double x, double y)
{
  double k1, k2;
  k1 = x + 2. * y - 7.;
  k2 = 2. * x + y - 5.;
  return k1 * k1 + k2 * k2;
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
  fprintf (file, "%.14le", Booth (x - M_PI_4, y - M_PI_4));
  fclose (file);
  return 0;
}
