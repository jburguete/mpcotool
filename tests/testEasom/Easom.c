#include <stdio.h>
#include <math.h>

double
Easom (double x, double y)
{
  double k1, k2;
  k1 = x - M_PI;
  k2 = y - M_PI;
  return -cos (x) * cos (y) * exp (-(k1 * k1 + k2 * k2));
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
  fprintf (file, "%.14le", Easom (x, y));
  fclose (file);
  return 0;
}
