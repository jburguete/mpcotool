#include <stdio.h>
#include <math.h>

int
main (int argn __attribute__ ((unused)), char **argc)
{
  FILE *fp;
  double ref, sol1, sol2, e, error;
  unsigned int n;

  fp = fopen (argc[1], "r");
  if (fscanf (fp, "%lf%lf", &sol1, &sol2) != 2)
    return 2;
  fclose (fp);

  fp = fopen (argc[2], "r");
  if (fscanf (fp, "%u%lf%lf", &n, &e, &ref) != 3)
    return 1;
  fclose (fp);

  ref = pow (ref, 1. / e);
  if (!n)
    error = sol1;
  else
    error = sol2;
  error = ref - error;
  error *= error;

  fp = fopen (argc[3], "w");
  fprintf (fp, "%e", error);
  fclose (fp);

  return 0;
}
