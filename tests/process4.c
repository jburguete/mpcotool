#include <stdio.h>
#include <stdlib.h>

#define NTESTS 6
#define NMETHODS 41

const char *test[NTESTS] = { "Sphere", "Ackley", "Booth", "Rosenbrock", "Easom",
  "Beale"
};

int
main ()
{
  char label[NMETHODS][512];
  char buffer[512];
  float x[NMETHODS][3], y[NMETHODS][3];
  FILE *file;
  unsigned int i, j;
  for (j = 0; j < NMETHODS; ++j)
    y[j][0] = y[j][1] = y[j][2] = 0.;
  for (i = 0; i < NTESTS; ++i)
    {
      snprintf (buffer, 512, "sed 's/,/ /g' %s.dat > %s2.dat",
                test[i], test[i]);
      system (buffer);
      snprintf (buffer, 512, "%s2.dat", test[i]);
      file = fopen (buffer, "r");
      for (j = 0; j < NMETHODS; ++j)
        {
          fscanf (file, "%s%*u%f%f%f",
                  &label[j][0], &x[j][0], &x[j][1], &x[j][2]);
          y[j][0] += x[j][0];
          y[j][1] += x[j][1];
          y[j][2] += x[j][2];
        }
      fclose (file);
    }
  file = fopen ("out", "w");
  for (j = 0; j < NMETHODS; ++j)
    fprintf (file, "%s %u %g %g %g\n", &label[j][0], j,
             y[j][0] / NTESTS, y[j][1] / NTESTS, y[j][2] / NTESTS);
  fclose (file);
  return 0;
}
