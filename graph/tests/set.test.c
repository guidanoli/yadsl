#include <stdio.h>
#include <string.h>
#include "set.h"

/* Help */

static const char *helpStrings[] = {
    "This is an interactive module of the set library",
    "You interact with a single set object at all times",
    "Actions to the set are parsed by the command line arguments",
    "",
    "The registered actions are the following:",
    "s\tprints current set size",
    "c\tprints current number pointed by the cursor",
    "f\tmake cursor point to smallest number in set",
    "l\tmake cursor point to biggest number in set",
    "p\tmake cursor point to number directly smaller (previous)",
    "n\tmake cursor point to number directly bigger (next)",
    "X+\tadd number X to set",
    "X-\tremove number X from set",
    "X?\tcheck if number X is contained in the set",
    NULL,
};

/* Set */

static Set *s;

/* Private functions prototypes */

static char *test(SetReturnID *pid, int *nid, char **numbers, int count);

static void print_help();

/* Main function */

int main(int argc, char **argv)
{
    SetReturnID id;
    int nid = 0;
    char **numbers;
    char *str;
    if (argc == 1) {
        print_help();
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

char *test(SetReturnID *pid, int *nid, char **numbers, int count)
{
    unsigned long number;
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
        if ((tokens = sscanf(numbers[i], "%lu%c", &number, &cmd)) != 2 &&
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
                    fprintf(stdout, "[%d] %lu\n", *nid, number);
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
                    fprintf(stdout, "[%d] %lu\n", *nid, number);
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
                            fprintf(stdout, "[%d] Contains %lu\n", *nid, number);
                            break;
                        case SET_RETURN_DOES_NOT_CONTAIN:
                            fprintf(stdout, "[%d] Does not contain %lu\n", *nid, number);
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

static void print_help() {
    const char **str = helpStrings;
    for (; *str; ++str) puts(*str);
}