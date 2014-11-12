#include <stdio.h>
#include <stdlib.h>

int main(int argn, char **argc)
{
	char buffer[512];
	snprintf(buffer, 512, "cp %s %s", argc[1], argc[3]);
	system(buffer);
	return 0;
}
