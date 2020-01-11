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

/* Private functions prototypes */

static char *parseCmds(Set **ppSet, SetReturnID *pSetId, unsigned int *pActionIndex, char **actions, int actionCount);

static void printHelpMessage();

/* Main function */

int main(int argc, char **argv)
{
    Set *testSet = NULL;
    SetReturnID setId = SET_RETURN_OK;
    unsigned int actionIndex = 0;
    char *errorString = NULL;
    if (argc == 1) {
        printHelpMessage();
        return 0;
    }
    /* ignore program name */
    errorString = parseCmds(&testSet, &setId, &actionIndex, argv + 1, argc - 1);
    if (testSet != NULL)
        setDestroy(testSet);
    testSet = NULL;
    /* print error message, if there is */
    if (errorString != NULL) {
        if (setId != SET_RETURN_OK) {
            fprintf(stderr, "Error #%d on action #%d (%s): %s\n",
                setId, actionIndex, argv[actionIndex], errorString);
        } else {
            fprintf(stderr, "Error on action #%d (%s): %s\n",
                actionIndex, argv[actionIndex], errorString);
        }
        return 1;
    } else {
        fprintf(stdout, "No errors.\n");
    }
    return 0;
}

static char *parseCmds(Set **ppSet, SetReturnID *pSetId, unsigned int *pActionIndex, char **actions, int actionCount)
{
    unsigned long number;
    char c;
    int i, tokenCount;
    if (*pSetId = setCreate(ppSet)) {
        if (*pSetId == SET_RETURN_MEMORY)
            *ppSet = NULL;
        return "Could not create set";
    }
    if ((*pSetId = setContains(*ppSet, 0)) != SET_RETURN_DOES_NOT_CONTAIN)
        return "Found number in empty set";
    for (i = 0; i < actionCount; ++i) {
        *pActionIndex = i+1;
        if ((tokenCount = sscanf(actions[i], "%lu%c", &number, &c)) != 2 &&
            (tokenCount = sscanf(actions[i], "%c", &c)) != 1)
            return "Parsing error";
        if (tokenCount == 1) {
            switch (c) {
                case 'p':
                    if (*pSetId = setPreviousValue(*ppSet))
                        return "Could not go to the previous value";
                    break;
                case 'n':
                    if (*pSetId = setNextValue(*ppSet))
                        return "Could not go to the next value";
                    break;
                case 'c':
                    if (*pSetId = setGetCurrent(*ppSet, &number))
                        return "Could not get current value";
                    fprintf(stdout, "[%d] %lu\n", *pActionIndex, number);
                    break;
                case 'f':
                    if (*pSetId = setFirstValue(*ppSet))
                        return "Could not go to the first value";
                    break;
                case 'l':
                    if (*pSetId = setLastValue(*ppSet))
                        return "Could not go to the last value";
                    break;
                case 's':
                    if (*pSetId = setGetSize(*ppSet, &number))
                        return "Could not get set size";
                    fprintf(stdout, "[%d] %lu\n", *pActionIndex, number);
                    break;
                default:
                    return "Unknown command";
            }
        } else if (tokenCount == 2) {
            switch (c) {
                case '-':
                    if (*pSetId = setRemove(*ppSet, number))
                        return "Could not remove number from set";
                    break;
                case '+':
                    if (*pSetId = setAdd(*ppSet, number))
                        return "Could not add number to set";
                    break;
                case '?':
                    *pSetId = setContains(*ppSet, number);
                    switch (*pSetId) {
                        case SET_RETURN_CONTAINS:
                            fprintf(stdout, "[%d] Contains %lu\n", *pActionIndex, number);
                            break;
                        case SET_RETURN_DOES_NOT_CONTAIN:
                            fprintf(stdout, "[%d] Does not contain %lu\n", *pActionIndex, number);
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

static void printHelpMessage() {
    const char **str = helpStrings;
    for (; *str; ++str) puts(*str);
}