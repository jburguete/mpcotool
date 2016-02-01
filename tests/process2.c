#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int
main (int argn, char **argc)
{
  char buffer[512];
  FILE *file;
  char *line;
  double x[5], xmin, xm, xmax;
  size_t n;
  unsigned int i;
  if (argn != 3)
	return 1;
  snprintf (buffer, 512, "tail -n1 test%s/v-%s-* > out",
		    argc[1], argc[2]);
  system (buffer);
  file = fopen ("out", "r");
  xmin = INFINITY;
  xmax = -INFINITY;
  xm = 0.;
  for (i = 0; i < 10; ++i)
	{
	  line = NULL;
	  n = 0;
	  getline (&line, &n, file);
	  free (line);
	  fscanf (file, "%lf%lf%lf%lf%lf", x, x + 1, x + 2, x + 3, x + 4);
	  x[0] = -log10 (x[4]);
	  if (x[0] < xmin)
		xmin = x[0];
	  if (x[0] > xmax)
		xmax = x[0];
	  xm += x[0];
	}
  xm /= 10;
  printf ("min=%lg mean=%lg max=%lg\n", xmin, xm, xmax);
  fclose (file);
  system ("rm out");
  return 0;
}
