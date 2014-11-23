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
	unsigned int yexperiment,
	double weight)
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
	if (weight != 0.) maxy = weight;
	return e / (maxy * maxy * ndata);
}

int main(int argn, char **argc)
{
	double e, ea, eh, es;
	char buffer[512];
	FILE *file_simulation, *file_experiment, *file_evaluation;
	snprintf(buffer, 512, "mkdir %s", argc[9]);
	system(buffer);
	snprintf(buffer, 512, "cp mesh.in model.in probe.in times.in %s", argc[9]);
	system(buffer);
	snprintf(buffer, 512, "cp %s %s/field.in", argc[1], argc[9]);
	system(buffer);
	snprintf(buffer, 512, "cp %s %s/input.in", argc[2], argc[9]);
	system(buffer);
	snprintf(buffer, 512, "./surcos %s", argc[9]);
	system(buffer);
	e = 0.;
	snprintf(buffer, 512, "%s/00b.out", argc[9]);
	file_simulation = fopen(buffer, "r");
	file_experiment = fopen(argc[3], "r");
	ea = sqrt
		(evaluation(file_simulation, 3, 0, 1, file_experiment, 2, 0, 1, 0.));
	fclose(file_simulation);
	fclose(file_experiment);
	snprintf(buffer, 512, "%s/probes.out", argc[9]);
	file_simulation = fopen(buffer, "r");
	file_experiment = fopen(argc[4], "r");
	eh = sqrt
		(evaluation(file_simulation, 11, 0, 1, file_experiment, 2, 0, 1, 0.));
	fclose(file_simulation);
	fclose(file_experiment);
	snprintf(buffer, 512, "%s/probes.out", argc[9]);
	file_simulation = fopen(buffer, "r");
	file_experiment = fopen(argc[5], "r");
	es = evaluation(file_simulation, 11, 0, 4, file_experiment, 2, 0, 1, 10.6);
	fclose(file_simulation);
	fclose(file_experiment);
	snprintf(buffer, 512, "%s/probes.out", argc[9]);
	file_simulation = fopen(buffer, "r");
	file_experiment = fopen(argc[6], "r");
	es += evaluation(file_simulation, 11, 0, 6, file_experiment, 2, 0, 1, 10.6);
	fclose(file_simulation);
	fclose(file_experiment);
	snprintf(buffer, 512, "%s/probes.out", argc[9]);
	file_simulation = fopen(buffer, "r");
	file_experiment = fopen(argc[7], "r");
	es += evaluation(file_simulation, 11, 0, 8, file_experiment, 2, 0, 1, 10.6);
	fclose(file_simulation);
	fclose(file_experiment);
	snprintf(buffer, 512, "%s/probes.out", argc[9]);
	file_simulation = fopen(buffer, "r");
	file_experiment = fopen(argc[8], "r");
	es += evaluation(file_simulation, 10, 0, 8, file_experiment, 2, 0, 1, 10.6);
	fclose(file_simulation);
	fclose(file_experiment);
	es = sqrt(es / 4);
	printf("error in advance: %lg\n", ea);
	printf("error in depth: %lg\n", eh);
	printf("error in concentration: %lg\n", es);
	e = ea + 0.5 * (eh + es);
	printf("total error: %lg\n", e);
	snprintf(buffer, 512, "rm -rf %s", argc[9]);
	system(buffer);
	file_evaluation = fopen(argc[9], "w");
	fprintf(file_evaluation, "%.14le", e);
	fclose(file_evaluation);
	return 0;
}
