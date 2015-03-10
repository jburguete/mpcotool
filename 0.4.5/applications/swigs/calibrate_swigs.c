#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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
	unsigned int i, end_simulation, ndata;
	double e, ey, maxy, rs1[nsimulation], rs2[nsimulation], re[nexperiment];
	e = 0.;
	maxy = 0.;
	end_simulation = ndata = 0;
	for (i = 0; i < nsimulation; ++i)
		if (fscanf(file_simulation, "%lf", rs2 + i) != 1) goto exit_evaluate;
	do
	{
		for (i = 0; i < nexperiment; ++i)
			if (fscanf(file_experiment, "%lf", re + i) != 1) goto exit_evaluate;
		++ndata;
		maxy = fmax(maxy, re[yexperiment]);
		for (i = 0; i < nsimulation; ++i) rs1[i] = rs2[i];
		if (rs1[xsimulation] > re[xexperiment])
			ey = rs1[ysimulation] - re[yexperiment];
		else if (!end_simulation)
		{
			do
			{
				for (i = 0; i < nsimulation; ++i) rs1[i] = rs2[i];
				for (i = 0; i < nsimulation; ++i)
				{
					if (fscanf(file_simulation, "%lf", rs2 + i) != 1)
					{
						end_simulation = 1;
						goto exit_simulation;
					}
				}
			}
			while (rs2[xsimulation] < re[xsimulation]);
			ey = rs1[ysimulation] + (rs2[ysimulation] - rs1[ysimulation])
				* (re[xexperiment] - rs1[xsimulation])
				/ (rs2[xsimulation] - rs1[xsimulation])
				- re[yexperiment];
		}
exit_simulation:
		if (end_simulation) ey = rs1[ysimulation] - re[yexperiment];
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
	snprintf(buffer, 512, "mkdir %s", argc[3]);
	system(buffer);
	snprintf(buffer, 512, "cp %s %s/case.xml", argc[1], argc[3]);
	system(buffer);
	snprintf(buffer, 512, "cp simulate.xml %s", argc[3]);
	system(buffer);
	snprintf(buffer, 512, "cd %s; ../guad1dbin simulate.xml", argc[3]);
	system(buffer);
	e = 0.;
	snprintf(buffer, 512, "%s/contributions", argc[3]);
	file_simulation = fopen(buffer, "r");
	file_experiment = fopen(argc[2], "r");
	e += evaluation(file_simulation, 1, 0, 1, file_experiment, 1, 0, 1);
	e += evaluation(file_simulation, 2, 0, 1, file_experiment, 2, 0, 1);
	e += evaluation(file_simulation, 3, 0, 1, file_experiment, 3, 0, 1);
	e += evaluation(file_simulation, 4, 0, 1, file_experiment, 4, 0, 1);
	fclose(file_simulation);
	fclose(file_experiment);
	file_overflow = fopen(buffer, "r");
	snprintf(buffer, 512, "%s/overflow", argc[3]);
	fscanf(file_overflow, "%d", &i);
	fclose(file_overflow);
	if (i) e *= 100.;
	e = sqrt(e);
	printf("total error: %lg\n", e);
	snprintf(buffer, 512, "rm -rf %s", argc[3]);
	system(buffer);
	file_evaluation = fopen(argc[3], "w");
	fprintf(file_evaluation, "%.14le", e);
	fclose(file_evaluation);
	return 0;
}
