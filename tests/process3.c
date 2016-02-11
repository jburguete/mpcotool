#include <stdio.h>

#define NTESTS 6
#define NMETHODS 31

char *test[NTESTS] = { "Sphere", "Ackley", "Booth", "Rosenbrock", "Easom",
	"Beale" };
char *method[NMETHODS] = {
	"GE-1", "GE-2", "GE-3", "GE-4", "GE-5", "GE-6",
	"GE-7", "GE-8", "GE-9", "GE-10", "GE-11",
	"MC-IT-1", "MC-1", "MC-CD-1", "MC-CD-2",
	"MC-RA-1", "MC-RA-2", "MC-RA-3", "MC-RA-4", "MC-RA-5", "MC-RA-6",
	"SW-RA-1", "SW-RA-2", "SW-RA-3", "SW-RA-4", "SW-RA-5", "SW-RA-6",
	"SW-IT-1", "SW-1", "SW-CD-1", "SW-CD-2"};

int main()
{
  char buffer[6][512];
  double x[3];
  FILE *filein, *fileout;
  unsigned int i, j;
  filein = fopen ("result", "r");
  for (i = 0; i < NTESTS; ++i)
	{
	  snprintf (buffer[0], 512, "%s.dat", test[i]);
      fileout = fopen (buffer[0], "w");
	  for (j = 0; j < NMETHODS; ++j)
		{
		  fscanf (filein, "%s%s%s%s%s", buffer[0], buffer[1], buffer[2],
				  buffer[3], buffer[4]);
		  sscanf (buffer[2]+4, "%lg", x);
		  sscanf (buffer[3]+5, "%lg", x + 1);
		  sscanf (buffer[4]+4, "%lg", x + 2);
          fprintf (fileout, "%s,%u,%lg,%lg,%lg\n", method[j], j, x[0], x[1],
				   x[2]);
		}
      fclose (fileout);
	}
  return 0;
}
