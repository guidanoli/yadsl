#include <stdio.h>
#include "diff.h"

int main(int argc, char **argv)
{
    size_t cost;
    if (argc != 3) return 1;
    cost = diff(argv[1], argv[2]);
    if (cost == DIFFERR) return 1;
    printf("difference = %d\n", cost);
    return 0;
}
