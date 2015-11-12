#include <stdio.h>
#include <math.h>

double
Rosenbrock (double x, double y)
{
  double k1, k2;
  k1 = y - x * x;
  k2 = x - 1.;
  return 100. * k1 * k1 + k2 * k2;
}

int
main (int argn, char **argc)
{
  double x, y;
  FILE *file;
  file = fopen (argc[1], "r");
  fscanf (file, "%*s%lf%*s%lf", &x, &y);
  fclose (file);
  file = fopen (argc[2], "w");
  fprintf (file, "%.14le", Rosenbrock (x - M_PI_4, y - M_PI_4));
  fclose (file);
  return 0;
}
