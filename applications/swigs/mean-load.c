#include <stdio.h>
#include <stdlib.h>

int main (int argn, char **argc)
{
  FILE *file;
  double x, r[5];
  int i, n, imin, imax;
  if (argn != 4)
	return 1;
  file = fopen (argc[1], "r");
  if (!file)
	return 1;
  imin = atoi (argc[2]);
  imax = atoi (argc[3]);
  for (x = 0., i = n = 0;
	   fscanf (file, "%lf%lf%lf%lf%lf", r, r + 1, r + 2, r + 3, r + 4) == 5;)
	{
	  ++i;
	  if (i > imax)
		break;
	  if (i >= imin)
		{
		  x += r[4];
		  ++n;
		}
	}
  fclose (file);
  printf ("Mean load=%lf\n", x / n);
  return 0;
}
