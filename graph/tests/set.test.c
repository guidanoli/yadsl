#include <stdio.h>
#include "set.h"

Set *s;

#define INTERNAL_ERROR 99

char *test(SetReturnID *pid, int *nid, char **numbers, int count)
{
    size_t number;
    char cmd;
    int i;
    if (*pid = setCreate(&s)) {
        if (*pid == SET_RETURN_MEMORY)
            s = NULL;
        return "Could not create set";
    }
    if ((*pid = setContains(s, 0)) != SET_RETURN_DOES_NOT_CONTAIN)
        return "Found number in empty set";
    for (i = 0; i < count; ++i) {
        *nid = i+1;
        if (sscanf(numbers[i], "%d%c", &number, &cmd) != 2)
            return "Parsing error";
        switch (cmd) {
            case '-':
                if (*pid = setRemove(s, number))
                    return "Could not remove number from set";
                break;
            case '+':
                if (*pid = setAdd(s, number))
                    return "Could not add number to set";
                break;
            case '?':
                *pid = setContains(s, number);
                switch (*pid) {
                    case SET_RETURN_CONTAINS:
                        fprintf(stdout, "[%d] Contains %d\n", *nid, number);
                        break;
                    case SET_RETURN_DOES_NOT_CONTAIN:
                        fprintf(stdout, "[%d] Does not contain %d\n", *nid, number);
                        break;
                    default:
                        return "Could not assert if set contained number or not";
                }
                break;
            default:
                return "Unknown command";
        }
    }
    return NULL;
}

int main(int argc, char **argv)
{
    SetReturnID id;
    int nid = 0;
    char **numbers = argc >= 2 ? argv + 1 : NULL;
    char *str = test(&id, &nid, numbers, argc - 1);
    if (s)
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
