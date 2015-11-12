#include <stdio.h>
#include <math.h>

double
Booth (double x, double y)
{
  double k1, k2;
  k1 = x + 2. * y - 7.;
  k2 = 2. * x + y - 5.;
  return k1 * k1 + k2 * k2;
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
  fprintf (file, "%.14le", Booth (x - M_PI_4, y - M_PI_4));
  fclose (file);
  return 0;
}
