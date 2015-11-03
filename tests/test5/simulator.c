#include <stdio.h>
#include <math.h>

inline double sqr (double x)
{
	return x * x;
}

inline double objective (double x, double y)
{
	return 1. + 0.001 * (x * x + y * y) + sin(M_PI * y - M_PI_2);
}

int main (int argn, char **argc)
{
	double value1, value2;
	FILE *fp;

	fp = fopen(argc[1], "r");
	fscanf(fp, "%*s %*s %lf", &value1);
	fscanf(fp, "%*s %*s %lf", &value2);
	fclose(fp);

	fp = fopen(argc[2], "w");
	fprintf(fp, "%.14le\n", objective(value1, value2));
	fclose(fp);

	return 0;
}
