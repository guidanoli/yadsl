#include <stdio.h>
#include <string.h>
#include "diff.h"

int main(int argc, char **argv)
{
    size_t cost;
    if (argc < 3) return 1;
    cost = diff(argv[1], argv[2]);
    if (cost == DIFFERR) return 0;
    if (argc > 3) {
        // Automated testing
        if (strcmp(argv[3], "EQUAL") == 0) {
            return !(cost == 0);
        } else if (strcmp(argv[3], "DIFFERENT") == 0) {
            return !(cost != 0);
        } else {
            fprintf(stderr, "Unknown parameter '%s'\n", argv[3]);
            return 1;
        }
    }
    printf("difference = %d\n", cost);
    return 0;
}
