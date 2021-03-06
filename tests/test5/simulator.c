/*
This is a test case to check the performance of optimization algorithms in an
ill-conditioned problem

The objective function is:
f = 1 + (x - pi)^2 / 1000 + (y - pi)^2 / 1000 + sin(pi * (y - 0.5))

The problem has 10 local minimums, located in very flat regions, on the interval
x \in [-10, 10] and y \in [-10, 10]

Absolute minimum is: x = pi, y = pi
*/

#include <stdio.h>
#include <math.h>

static inline double
objective (double x, double y)
{
  x -= M_PI;
  y -= M_PI;
  return 1. + 0.001 * (x * x + y * y) + sin (M_PI * y - M_PI_2);
}

int
main (int argn __attribute__ ((unused)), char **argc)
{
  FILE *fp;
  double value1, value2;

  fp = fopen (argc[1], "r");
  if (fscanf (fp, "%*s%*s%lf%*s%*s%lf", &value1, &value2) != 2)
    return 1;
  fclose (fp);

  fp = fopen (argc[2], "w");
  fprintf (fp, "%.14le\n", objective (value1, value2));
  fclose (fp);

  return 0;
}
