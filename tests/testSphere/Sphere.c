#include <stdio.h>
#include <math.h>

static inline double
Sphere (double x, double y)
{
  return x * x + y * y;
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
  fprintf (file, "%.14le", Sphere (x - M_PI_4, y - M_PI_4));
  fclose (file);
  return 0;
}
