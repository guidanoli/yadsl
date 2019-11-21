#include <stdio.h>
#include "set.h"

Set *s;

#define INTERNAL_ERROR 99

char *test(SetReturnID *pid, int *nid, char **numbers, int count)
{
    if (*pid = setCreate(&s))
        return "Could not create set";
    if ((*pid = setContains(s, 0)) != SET_RETURN_DOES_NOT_CONTAIN)
        return "Found number in empty set";
    size_t number;
    for (int i = 0; i < count; ++i) {
        *nid = i+1;
        if (sscanf(numbers[i], "-%zu", &number) == 1) {
            if (*pid = setRemove(s, number))
                return "Could not remove number from set";
        } else {
            if (sscanf(numbers[i], "%zu", &number) != 1) {
                *pid = INTERNAL_ERROR;
                return "Invalid number";
            }
            if (*pid = setAdd(s, number))
                return "Could not add number to set";
        }
    }
    return NULL;
}

int main(int argc, char **argv)
{
    SetReturnID id;
    int nid;
    char **numbers = argc >= 2 ? argv + 1 : NULL;
    char *str = test(&id, &nid, numbers, argc - 1);
    setDestroy(s);
    s = NULL;
    if (str) {
        if (id) {
            fprintf(stderr, "Error #%d on %d-th number (%s): %s\n", id, nid, argv[nid], str);
        } else {
            fprintf(stderr, "Error on %d-th number (%s): %s\n", nid, argv[nid], str);
        }
        return 1;
    } else {
        fprintf(stdout, "No errors.\n");
    }
    return 0;
}
