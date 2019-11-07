#include "diff.h"

int main(int argc, char **argv)
{
	if (argc != 3) return 1;
	if (!diff(argv[1], argv[2])) return 1;
	return 0;
}
