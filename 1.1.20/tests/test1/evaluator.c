#include <stdio.h>
#include <math.h>

int main(int argn, char **argc)
{
	double ref1, sol1;
	double ref2, sol2;
	double error;
	FILE *fp;

	fp = fopen(argc[1], "r");
	fscanf(fp, "%lf", &ref1);
	fscanf(fp, "%lf", &ref2);
	fclose(fp);

	fp = fopen(argc[2], "r");
	fscanf(fp, "%*s %*s %lf", &sol1);
	fscanf(fp, "%*s %*s %lf", &sol2);
	fclose(fp);

	error  = ( ref1 - sol1 ) * ( ref1 - sol1 );
	error += ( ref2 - sol2 ) * ( ref2 - sol2 );

	fp = fopen(argc[3], "w");
	fprintf(fp, "%e", error);
	fclose(fp);

	return 0;
}
