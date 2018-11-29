#include <stdio.h>
#include <math.h>

static inline double
Beale (double x, double y)
{
  double k1, k2, k3;
  k1 = 1.5 - x + x * y;
  k2 = 2.25 - x + x * y * y;
  k3 = 2.625 - x + x * y * y * y;
  return k1 * k1 + k2 * k2 + k3 * k3;
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
  fprintf (file, "%.14le", Beale (x - M_PI_4, y - M_PI_4));
  fclose (file);
  return 0;
}
