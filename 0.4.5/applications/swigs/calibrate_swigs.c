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
		printf("xs=%lg ys=%lg xe=%lg ye=%lg e=%lg\n",
			rs1[xsimulation], rs1[ysimulation],
			re[xexperiment], re[yexperiment], e);
	}
	while (1);
exit_evaluate:
	e /= ndata;
	printf("e=%lg\n", e);
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
	e = evaluation(file_experiment, 5, 0, 1, file_simulation, 6, 0, 2);
	printf("e1=%lg\n", e);
	fseek(file_simulation, 0L, SEEK_SET);
	fseek(file_experiment, 0L, SEEK_SET);
	e += evaluation(file_experiment, 5, 0, 2, file_simulation, 6, 0, 3);
	printf("e2=%lg\n", e);
	fseek(file_simulation, 0L, SEEK_SET);
	fseek(file_experiment, 0L, SEEK_SET);
	e += evaluation(file_experiment, 5, 0, 3, file_simulation, 6, 0, 4);
	printf("e3=%lg\n", e);
	fseek(file_simulation, 0L, SEEK_SET);
	fseek(file_experiment, 0L, SEEK_SET);
	e += evaluation(file_experiment, 5, 0, 4, file_simulation, 6, 0, 5);
	printf("e4=%lg\n", e);
	fclose(file_simulation);
	fclose(file_experiment);
	file_overflow = fopen(buffer, "r");
	snprintf(buffer, 512, "%s/overflow", argc[2]);
	fscanf(file_overflow, "%d", &i);
	fclose(file_overflow);
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
