#include <stdio.h>
#include "diff.h"

int main(int argc, char **argv)
{
	if (argc != 3) return 1;
	int cost = diff(argv[1], argv[2]);
	if (cost == -1) return 1;
	printf("difference = %d\n", cost);
	return 0;
}
