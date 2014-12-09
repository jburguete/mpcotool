#include <stdio.h>
#include <stdlib.h>

int main(int argn, char **argc)
{
	int i, j, nt, nm, n1[20], n2[20];
	float t1, t2, t3;
	FILE *filein, *fileout;
	filein = fopen(argc[1], "r");
	fileout = fopen(argc[2], "w");
	fscanf(filein, "%d", &nt);
	nm = atoi(argc[3]);
	fprintf(fileout, "%s ", argc[4]);
	for (i = 0; i < nt; ++i)
		fprintf(fileout, "%s %s ", argc[5 +  i], argc[5 + i]);
	fprintf(fileout, "\n");
	t3 = 0.;
	nt *= 2;
	do
	{
		if (fscanf(filein, "%f", &t1) != 1) goto end;
		for (i = 0; i < nt; ++i)
			if (fscanf(filein, "%d", n1 + i) != 1) goto end;
		for (j = 1; j < nm; ++j)
		{
			if (fscanf(filein, "%f", &t2) != 1) goto end;
			for (i = 0; i < nt; ++i)
				if (fscanf(filein, "%d", n2 + i) != 1) goto end;
			t1 = 2 * t2 - t1;
			for (i = 0; i < nt; ++i) n1[i] += n2[i];
		}
		fprintf(fileout, "%g-%g ", t3, t1);
		for (i = 0; i < nt; ++i) fprintf(fileout, "%d ", n1[i]);
		fprintf(fileout, "\n");
		t3 = t1;
	}
	while (1);
end:
	fclose(filein);
	fclose(fileout);
	return 0;
}
