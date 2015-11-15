#include <stdio.h>
#include <math.h>
#include <float.h>

int main (int argn, char **argc)
{
  FILE *filein, *fileout;
  double x, y, z, m, dx, dy;
  unsigned int i;
  filein = fopen (argc[1], "r");
  fileout = fopen (argc[2], "w");
  m = DBL_MAX;
  for (i = 1; fscanf (filein, "%lf%lf%lf", &x, &y, &z) == 3; ++i)
    {
	  dx = x - M_PI_4;
	  dy = y - M_PI_4;
	  z = sqrt (dx * dx + dy * dy);
      m = fmin (m, z);
      fprintf (fileout, "%u %le %le %le %le\n", i, x, y, z, m);
	}
  fclose (fileout);
  fclose (filein);
  return 0;
}
