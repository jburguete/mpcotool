#include <stdio.h>
#include <string.h>

#define NTESTS 6
#define NMETHODS 41

const char *test[NTESTS] = { "Sphere", "Ackley", "Booth", "Rosenbrock", "Easom",
	"Beale" };
const char *method[NMETHODS] = {
	"GE-1", "GE-2", "GE-3", "GE-4", "GE-5", "GE-6",
	"GE-7", "GE-8", "GE-9", "GE-10", "GE-11",
	"MC-IT-1", "MC-1", "MC-CD-1", "MC-CD-2",
	"MC-RA-1", "MC-RA-2", "MC-RA-3", "MC-RA-4", "MC-RA-5", "MC-RA-6",
	"SW-RA-1", "SW-RA-2", "SW-RA-3", "SW-RA-4", "SW-RA-5", "SW-RA-6",
	"SW-IT-1", "SW-1", "SW-CD-1", "SW-CD-2",
	"OS-RA-1", "OS-RA-2", "OS-RA-3", "OS-RA-4", "OS-RA-5", "OS-RA-6",
	"OS-IT-1", "OS-1", "OS-CD-1", "OS-CD-2"};
const char *label[NMETHODS] = {
  "ge-100-33-0-0-0.75-32-14-*",
  "ge-100-33-0-0.75-0-32-14-*",
  "ge-100-33-0.25-0.25-0.25-32-14-*",
  "ge-100-33-0.75-0-0-32-14-*",
  "ge-250-16-0.2-0.2-0.2-32-14-*",
  "ge-250-31-0.1-0.1-0.1-32-14-*",
  "ge-500-21-0-0-0.2-32-14-*",
  "ge-500-21-0-0.2-0-32-14-*",
  "ge-500-21-0.2-0-0-32-14-*",
  "ge-625-11-0.1-0.1-0.1-32-14-*",
  "ge-625-6-0.2-0.2-0.2-32-14-*",
  "mc-100-25-10-0.1-14-*",
  "mc-2500-1-14-*",
  "mc-cd-100-1-600-0.1-1-14-*",
  "mc-cd-1600-1-225-0.01-1-14-*",
  "mc-ra-100-1-1200-2-0.1-1-14-*",
  "mc-ra-100-1-600-4-0.1-1-14-*",
  "mc-ra-100-1-240-10-0.1-1-14-*",
  "mc-ra-1600-1-450-2-0.01-1-14-*",
  "mc-ra-1600-1-225-4-0.01-1-14-*",
  "mc-ra-1600-1-90-10-0.01-1-14-*",
  "sw-ra-10-10-1-1200-2-0.1-1-14-*",
  "sw-ra-10-10-1-600-4-0.1-1-14-*",
  "sw-ra-10-10-1-240-10-0.1-1-14-*",
  "sw-ra-40-40-1-450-2-0.01-1-14-*",
  "sw-ra-40-40-1-225-4-0.01-1-14-*",
  "sw-ra-40-40-1-90-10-0.01-1-14-*",
  "sw-10-10-25-10-0.5-14",
  "sw-50-50-1-14",
  "sw-cd-10-10-1-600-0.1-1-14",
  "sw-cd-40-40-1-225-0.01-1-14",
  "os-10-10-25-10-0.5-14-*",
  "os-50-50-1-14-*",
  "os-cd-10-10-1-600-0.1-1-14-*",
  "os-cd-40-40-1-225-0.01-1-14-*",
  "os-ra-10-10-1-1200-2-0.1-1-14-*",
  "os-ra-10-10-1-600-4-0.1-1-14-*",
  "os-ra-10-10-1-240-10-0.1-1-14-*",
  "os-ra-40-40-1-450-2-0.01-1-14-*",
  "os-ra-40-40-1-225-4-0.01-1-14-*",
  "os-ra-40-40-1-90-10-0.01-1-14-*"};

int main()
{
  char buffer[6][512];
  double x[NMETHODS][3];
  FILE *filein, *fileout;
  unsigned int i, j, k;
  filein = fopen ("result", "r");
  for (i = 0; i < NTESTS; ++i)
	{
	  for (j = 0; j < NMETHODS; ++j)
		{
		  fscanf (filein, "%s%s%s%s%s", buffer[0], buffer[1], buffer[2],
				  buffer[3], buffer[4]);
			for (k = 0; k < NMETHODS && strcmp (label[k], buffer[1] + 7);)
				++k;
printf ("k=%u\n", k);
			if (k == NMETHODS)
				break;
		  sscanf (buffer[2] + 4, "%lg", &x[k][0]);
		  sscanf (buffer[3] + 5, "%lg", &x[k][1]);
		  sscanf (buffer[4] + 4, "%lg", &x[k][2]);
		}
	  snprintf (buffer[0], 512, "%s.dat", test[i]);
    fileout = fopen (buffer[0], "w");
	  for (j = 0; j < NMETHODS; ++j)
      fprintf (fileout, "%s,%u,%lg,%lg,%lg\n",
					     method[j], j, x[j][0], x[j][1], x[j][2]);
    fclose (fileout);
	}
	fclose (filein);
  return 0;
}
