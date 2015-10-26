#include <stdio.h>
#include <math.h>

double objective(double x, double y)
{
	double k;
	k = x - 0.5;
	return 1. + 0.001 * k * k + sin(M_PI * y);
}

int main(int argn, char **argc)
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
