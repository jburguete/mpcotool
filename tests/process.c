#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

int main (int argn, char **argc)
{
  FILE *filein, *fileout;
  double x, y, x0, y0, z, m, dx, dy;
  unsigned int i;
  filein = fopen (argc[1], "r");
  fileout = fopen (argc[2], "w");
  if (argn == 3)
	x0 = y0 = M_PI_4;
  else if (argn == 4)
	x0 = y0 = M_PI;
  else
	{
	  x0 = atof (argc[3]) + M_PI_4;
	  y0 = atof (argc[4]) + M_PI_4;
	}
  m = DBL_MAX;
  for (i = 1; fscanf (filein, "%lf%lf%lf", &x, &y, &z) == 3; ++i)
    {
	  dx = x - x0;
	  dy = y - y0;
	  z = sqrt (dx * dx + dy * dy);
      m = fmin (m, z);
      fprintf (fileout, "%u %le %le %le %le\n", i, x, y, z, m);
	}
  fclose (fileout);
  fclose (filein);
  return 0;
}
