#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

double evaluation(
	FILE *file_simulation,
	unsigned int nsimulation,
	unsigned int xsimulation,
	unsigned int ysimulation,
	FILE *file_experiment,
	unsigned int nexperiment,
	unsigned int xexperiment,
	unsigned int yexperiment)
{
	unsigned int i, end_experiment, ndata;
	double e, ey, maxy, re1[nexperiment], re2[nexperiment], rs[nsimulation];
	e = 0.;
	maxy = 0.;
	end_experiment = ndata = 0;
	for (i = 0; i < nexperiment; ++i)
		if (fscanf(file_experiment, "%lf", re2 + i) != 1) goto exit_evaluate;
	maxy = fmax(maxy, fabs(re2[yexperiment]));
	memcpy(re1, re2, nexperiment * sizeof(double));
	do
	{
		for (i = 0; i < nsimulation; ++i)
			if (fscanf(file_simulation, "%lf", rs + i) != 1) goto exit_evaluate;
		++ndata;
		if (re1[xexperiment] >= rs[xsimulation])
			ey = re1[yexperiment] - rs[ysimulation];
		else if (!end_experiment)
		{
			while (re2[xexperiment] < rs[xsimulation])
			{
				memcpy(re1, re2, nexperiment * sizeof(double));
				for (i = 0; i < nexperiment; ++i)
				{
					if (fscanf(file_experiment, "%lf", re2 + i) != 1)
					{
						end_experiment = 1;
						goto exit_experiment;
					}
				}
				maxy = fmax(maxy, fabs(re2[yexperiment]));
			}
			ey = re1[yexperiment] + (re2[yexperiment] - re1[yexperiment])
				* (rs[xsimulation] - re1[xexperiment])
				/ (re2[xexperiment] - re1[xexperiment])
				- rs[ysimulation];
		}
exit_experiment:
		if (end_experiment) ey = re1[yexperiment] - rs[ysimulation];
		e += ey * ey;
	}
	while (1);
exit_evaluate:
	e /= ndata;
	return e;
}

int main(int argn, char **argc)
{
	int i;
	double e;
	char buffer[512];
	FILE *file_simulation, *file_experiment, *file_evaluation, *file_overflow;
	printf("calibrate_swigs %s %s\n", argc[1], argc[2]);
	snprintf(buffer, 512, "mkdir %s", argc[2]);
	system(buffer);
	snprintf(buffer, 512, "cp %s %s/case.xml", argc[1], argc[2]);
	system(buffer);
	snprintf(buffer, 512, "cp simulate.xml %s", argc[2]);
	system(buffer);
	snprintf(buffer, 512, "cd %s; ../swigsbin simulate.xml", argc[2]);
	system(buffer);
	snprintf(buffer, 512, "%s/contributions", argc[2]);
	file_simulation = fopen(buffer, "r");
	file_experiment = fopen("demands", "r");
	e = evaluation(file_simulation, 6, 0, 2, file_experiment, 5, 0, 1);
	printf("e1=%lg\n", e);
	fseek(file_simulation, 0L, SEEK_SET);
	fseek(file_experiment, 0L, SEEK_SET);
	e += evaluation(file_simulation, 6, 0, 3, file_experiment, 5, 0, 2);
	printf("e2=%lg\n", e);
	fseek(file_simulation, 0L, SEEK_SET);
	fseek(file_experiment, 0L, SEEK_SET);
	e += evaluation(file_simulation, 6, 0, 4, file_experiment, 5, 0, 3);
	printf("e3=%lg\n", e);
	fseek(file_simulation, 0L, SEEK_SET);
	fseek(file_experiment, 0L, SEEK_SET);
	e += evaluation(file_simulation, 6, 0, 5, file_experiment, 5, 0, 4);
	printf("e4=%lg\n", e);
	fclose(file_simulation);
	fclose(file_experiment);
	snprintf(buffer, 512, "ls %s", argc[2]);
	system(buffer);
	snprintf(buffer, 512, "%s/overflow", argc[2]);
	file_overflow = fopen(buffer, "r");
	fscanf(file_overflow, "%d", &i);
	fclose(file_overflow);
	printf("overflow=%d\n", i);
	if (i) e *= 100.;
	e = sqrt(e);
	printf("total error: %lg\n", e);
	snprintf(buffer, 512, "rm -rf %s", argc[2]);
	system(buffer);
	file_evaluation = fopen(argc[2], "w");
	fprintf(file_evaluation, "%.14le", e);
	fclose(file_evaluation);
	return 0;
}
