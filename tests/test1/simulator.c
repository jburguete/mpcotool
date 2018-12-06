#include <stdio.h>
#include <math.h>

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
  fprintf (fp, "%e %e\n", value1, value2);
  fclose (fp);

  return 0;
}
