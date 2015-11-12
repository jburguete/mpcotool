#include <stdio.h>
#include <math.h>

double Sphere(double x, double y)
{
  return x * x + y * y;
}

int main(int argn, char **argc)
{
  double x, y;
  FILE *file;
  file = fopen (argc[1], "r");
  fscanf (file, "%*s%lf%*s%lf", &x, &y);
  fclose (file);
  file = fopen (argc[2], "w");
  fprintf (file, "%.14le", Sphere (x - M_PI_4, y - M_PI_4));
  fclose (file);
  return 0;
}
