#include <stdio.h>
#include <math.h>

double Ackley(double x, double y)
{
  return 20. * (1. - exp (-0.2 * sqrt (0.5 * (x * x + y * y)))) + M_E
    - exp(0.5 * (cos (2. * M_PI * x) + cos (2. * M_PI * y)));
}

int main(int argn, char **argc)
{
  double x, y;
  FILE *file;
  file = fopen (argc[1], "r");
  fscanf (file, "%*s%lf%*s%lf", &x, &y);
  fclose (file);
  file = fopen (argc[2], "w");
  fprintf (file, "%.14le", Ackley (x - M_PI, y - M_PI));
}
