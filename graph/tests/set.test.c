#include <stdio.h>
#include <string.h>
#include "set.h"

Set *s;

char *test(SetReturnID *pid, int *nid, char **numbers, int count)
{
    size_t number;
    char cmd;
    int i, tokens;
    if (*pid = setCreate(&s)) {
        if (*pid == SET_RETURN_MEMORY)
            s = NULL;
        return "Could not create set";
    }
    if ((*pid = setContains(s, 0)) != SET_RETURN_DOES_NOT_CONTAIN)
        return "Found number in empty set";
    for (i = 0; i < count; ++i) {
        *nid = i+1;
        if ((tokens = sscanf(numbers[i], "%d%c", &number, &cmd)) != 2 &&
            (tokens = sscanf(numbers[i], "%c", &cmd)) != 1)
            return "Parsing error";
        if (tokens == 1) {
            switch (cmd) {
                case 'p':
                    if (*pid = setPreviousValue(s))
                        return "Could not go to the previous value";
                    break;
                case 'n':
                    if (*pid = setNextValue(s))
                        return "Could not go to the next value";
                    break;
                case 'c':
                    if (*pid = setGetCurrent(s, &number))
                        return "Could not get current value";
                    fprintf(stdout, "[%d] %d\n", *nid, number);
                    break;
                case 'f':
                    if (*pid = setFirstValue(s))
                        return "Could not go to the first value";
                    break;
                case 'l':
                    if (*pid = setLastValue(s))
                        return "Could not go to the last value";
                    break;
                case 's':
                    if (*pid = setGetSize(s, &number))
                        return "Could not get set size";
                    fprintf(stdout, "[%d] %d\n", *nid, number);
                    break;
                default:
                    return "Unknown command";
            }
        } else if (tokens == 2) {
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
    }
    return NULL;
}

int main(int argc, char **argv)
{
    SetReturnID id;
    int nid = 0;
    char **numbers;
    char *str;
    if (argc >= 2 && (strcmp(argv[1],"help") == 0 || strcmp(argv[1],"HELP") == 0)) {
        printf("This is an interactive module of the set library\n");
        printf("You interact with a single set object at all times\n");
        printf("Actions to the set are parsed by the command line arguments\n");
        printf("The registered actions are the following:\n");
        printf("s\tprints current set size\n");
        printf("c\tprints current number pointed by the cursor\n");
        printf("f\tmake cursor point to smallest number in set\n");
        printf("l\tmake cursor point to biggest number in set\n");
        printf("p\tmake cursor point to number directly smaller (previous)\n");
        printf("n\tmake cursor point to number directly bigger (next)\n");
        printf("X+\tadd number X to set\n");
        printf("X-\tremove number X from set\n");
        printf("X?\tcheck if number X is contained in the set\n");
        return 0;
    }
    numbers = argc >= 2 ? argv + 1 : NULL;
    str = test(&id, &nid, numbers, argc - 1);
    if (s)
        setDestroy(s);
    s = NULL;
    if (str) {
        if (id) {
            fprintf(stderr, "Error #%d on action #%d (%s): %s\n", id, nid, argv[nid], str);
        } else {
            fprintf(stderr, "Error on action #%d (%s): %s\n", nid, argv[nid], str);
        }
        return 1;
    } else {
        fprintf(stdout, "No errors.\n");
    }
    return 0;
}
