#include <stdio.h>
#include <math.h>

int main(int argn, char **argc)
{
	double value1, value2;
	FILE *fp;

	fp = fopen(argc[1], "r");
	fscanf(fp, "%*s %*s %lf", &value1);
	fscanf(fp, "%*s %*s %lf", &value2);
	fclose(fp);

	fp = fopen(argc[2], "w");
	fprintf(fp, "%e\n", value1);
	fprintf(fp, "%e\n", value2);
	fclose(fp);

	return 0;
}
