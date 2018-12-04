#include <stdio.h>

#define NTESTS 6
#define NMETHODSA1 37
#define NMETHODSB1 6
#define NMETHODSA2 4
#define NMETHODSB2 1
#define NSEEDS 10

char *test[NTESTS] = { "Sphere", "Ackley", "Booth", "Rosenbrock", "Easom",
  "Beale"
};

char *arguments[NTESTS] = { "", "", "1 3", "1 1", "0", "3 0.5" };

char *methodA1[NMETHODSA1] = {
  "ge-100-33-0-0-0.75-32-14",
  "ge-100-33-0-0.75-0-32-14",
  "ge-100-33-0.25-0.25-0.25-32-14",
  "ge-100-33-0.75-0-0-32-14",
  "ge-250-16-0.2-0.2-0.2-32-14",
  "ge-250-31-0.1-0.1-0.1-32-14",
  "ge-500-21-0-0-0.2-32-14",
  "ge-500-21-0-0.2-0-32-14",
  "ge-500-21-0.2-0-0-32-14",
  "ge-625-11-0.1-0.1-0.1-32-14",
  "ge-625-6-0.2-0.2-0.2-32-14",
  "mc-100-25-10-0.1-14",
  "mc-2500-1-14",
  "mc-cd-100-1-600-0.1-1-14",
  "mc-cd-1600-1-225-0.01-1-14",
  "mc-ra-100-1-1200-2-0.1-1-14",
  "mc-ra-100-1-600-4-0.1-1-14",
  "mc-ra-100-1-240-10-0.1-1-14",
  "mc-ra-1600-1-450-2-0.01-1-14",
  "mc-ra-1600-1-225-4-0.01-1-14",
  "mc-ra-1600-1-90-10-0.01-1-14",
  "sw-ra-10-10-1-1200-2-0.1-1-14",
  "sw-ra-10-10-1-600-4-0.1-1-14",
  "sw-ra-10-10-1-240-10-0.1-1-14",
  "sw-ra-40-40-1-450-2-0.01-1-14",
  "sw-ra-40-40-1-225-4-0.01-1-14",
  "sw-ra-40-40-1-90-10-0.01-1-14",
  "os-10-10-25-10-0.5-14",
  "os-50-50-1-14",
  "os-cd-10-10-1-600-0.1-1-14",
  "os-cd-40-40-1-225-0.01-1-14",
  "os-ra-10-10-1-1200-2-0.1-1-14",
  "os-ra-10-10-1-600-4-0.1-1-14",
  "os-ra-10-10-1-240-10-0.1-1-14",
  "os-ra-40-40-1-450-2-0.01-1-14",
  "os-ra-40-40-1-225-4-0.01-1-14",
  "os-ra-40-40-1-90-10-0.01-1-14"
};

char *methodB1[NMETHODSB1] = {
  "mc-100-25-4-0-14",
  "mc-cd-100-1-600-0.01-0-14",
  "mc-cd-100-1-600-0.01-1-14",
  "mc-cd-100-1-600-0.01-2-14",
  "mc-cd-100-1-600-0.1-0-14",
  "mc-cd-100-1-600-1-0-14"
};

char *methodA2[NMETHODSA2] = {
  "sw-10-10-25-10-0.5-14",
  "sw-50-50-1-14",
  "sw-cd-10-10-1-600-0.1-1-14",
  "sw-cd-40-40-1-225-0.01-1-14"
};

char *methodB2[NMETHODSB2] = {
  "sw-10-10-25-4-0-14"
};

unsigned int seed[NSEEDS] = { 777, 7077, 7707, 7770, 70077, 70707, 70770, 77007,
  77070, 77700
};

int
main (int argn, char **argc)
{
  FILE *file;
  unsigned int i, j, k;
  if (argn != 2)
    return 1;
  file = fopen (argc[1], "w");
  if (!file)
    return 2;
  for (j = 0; j < NMETHODSB1; ++j)
    for (k = 0; k < NSEEDS; ++k)
      fprintf (file, "./mpcotoolbin -seed %u ../tests/testSphere/test-%s.xml"
               " result-%s-%u variables-%s-%u\n",
               seed[k], methodB1[j], methodB1[j], seed[k], methodB1[j],
               seed[k]);
  for (j = 0; j < NMETHODSB2; ++j)
    fprintf (file, "./mpcotoolbin ../tests/testSphere/test-%s.xml result-%s "
             "variables-%s\n", methodB2[j], methodB2[j], methodB2[j]);
  for (i = 0; i < NTESTS; ++i)
    {
      for (j = 0; j < NMETHODSA1; ++j)
        for (k = 0; k < NSEEDS; ++k)
          fprintf (file, "./mpcotoolbin -seed %u ../tests/test%s/test-%s.xml"
                   " result-%s-%u variables-%s-%u\n",
                   seed[k], test[i], methodA1[j], methodA1[j], seed[k],
                   methodA1[j], seed[k]);
      for (j = 0; j < NMETHODSA2; ++j)
        fprintf (file, "./mpcotoolbin ../tests/test%s/test-%s.xml result-%s "
                 "variables-%s\n",
                 test[i], methodA2[j], methodA2[j], methodA2[j]);
    }
  fprintf (file, "cd ../tests\n");
  for (j = 0; j < NMETHODSB1; ++j)
    for (k = 0; k < NSEEDS; ++k)
      fprintf (file, "./process test%s/variables-%s-%u test%s/v-%s-%u\n",
               test[0], methodB1[j], seed[k], test[0], methodB1[j], seed[k]);
  for (j = 0; j < NMETHODSB2; ++j)
    fprintf (file, "./process test%s/variables-%s test%s/v-%s\n",
             test[0], methodB2[j], test[0], methodB2[j]);
  for (i = 0; i < NTESTS; ++i)
    {
      for (j = 0; j < NMETHODSA1; ++j)
        for (k = 0; k < NSEEDS; ++k)
          fprintf (file,
                   "./process test%s/variables-%s-%u test%s/v-%s-%u %s\n",
                   test[i], methodA1[j], seed[k], test[i], methodA1[j],
                   seed[k], arguments[i]);
      for (j = 0; j < NMETHODSA2; ++j)
        fprintf (file, "./process test%s/variables-%s test%s/v-%s %s\n",
                 test[i], methodA2[j], test[i], methodA2[j], arguments[i]);
    }
  for (i = 0; i < NTESTS; ++i)
    {
      for (j = 0; j < NMETHODSA1; ++j)
        fprintf (file, "./process2 %s %s-* 10 >> result\n",
                 test[i], methodA1[j]);
      for (j = 0; j < NMETHODSA2; ++j)
        fprintf (file, "./process2 %s %s 1 >> result\n", test[i], methodA2[j]);
    }
  fclose (file);
  return 0;
}
