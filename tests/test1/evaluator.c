#include <stdio.h>
#include <math.h>

int
main (int argn __attribute__ ((unused)), char **argc)
{
  FILE *fp;
  double ref1, sol1, ref2, sol2, error;

  fp = fopen (argc[1], "r");
  if (fscanf (fp, "%lf%lf", &ref1, &ref2) != 2)
    return 1;
  fclose (fp);

  fp = fopen (argc[2], "r");
  if (fscanf (fp, "%*s%*s%lf%*s%*s%lf", &sol1, &sol2) != 2)
    return 2;
  fclose (fp);

  error = (ref1 - sol1) * (ref1 - sol1);
  error += (ref2 - sol2) * (ref2 - sol2);

  fp = fopen (argc[3], "w");
  fprintf (fp, "%e", error);
  fclose (fp);

  return 0;
}
