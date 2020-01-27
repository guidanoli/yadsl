#include <stdio.h>
#include <string.h>
#include "set.h"
#include "var.h"

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
    "-<cf>\tcheck if there is a number that satisfies function",
    "",
    "The registered custom functions are the following:",
    /* Follows custom functions here... */
    NULL, /* Sentinel */
};

/* Custom functions */

// Macros
#define CF_MAX_NAME_SIZE 64
#define CF_NAME(label) func_ ## label
#define CF_PROTOTYPE(label) static int CF_NAME(label) (void *item, void *arg)
#define CF(label, desc) {CF_NAME(label), #label, desc}

// Function prototypes
CF_PROTOTYPE(even);
CF_PROTOTYPE(square);

struct customFunction {
    int (*func)(void *item, void *arg);
    const char *label;
    const char *description;
}; 

static struct customFunction customFunctions[] = {
    CF(even, "even number"),
    CF(square, "perfect square"),
    { NULL, NULL, NULL }, /* Sentinel */
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
    int i;
    char buffer[CF_MAX_NAME_SIZE];
    if (*pSetId = setCreate(ppSet)) {
        if (*pSetId == SET_RETURN_MEMORY)
            *ppSet = NULL;
        return "Could not create set";
    }
    for (i = 0; i < actionCount; ++i) {
        *pActionIndex = i+1;
        if (sscanf(actions[i], "%lu%c", &number, &c) == 2) {
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
                        fprintf(stdout, "[%u] Contains %lu\n", *pActionIndex, number);
                        break;
                    case SET_RETURN_DOES_NOT_CONTAIN:
                        fprintf(stdout, "[%u] Does not contain %lu\n", *pActionIndex, number);
                        break;
                    default:
                        return "Cannot not assert if set contains number or not";
                }
                break;
            default:
                return "Unknown command";
            }
        } else if (sscanf(actions[i], "%c", &c) == 1) {
            struct customFunction *cfunc = customFunctions;
            int matchedCustomFunction = 0;
            switch (c) {
            case '-':
                if (strlen(actions[i]) >= CF_MAX_NAME_SIZE - 1)
                    return "Custom function exceeds the maximum size";
                if (sscanf(actions[i], "-%s", buffer) != 1)
                    return "Could not parse custom function name";
                for (; cfunc->func && !matchedCustomFunction; ++cfunc) {
                    if (!strcmp(buffer, cfunc->label)) {
                        matchedCustomFunction = 1;
                        *pSetId = setFilterItem(*ppSet, cfunc->func, NULL, (void **) &number);
                        switch (*pSetId) {
                        case SET_RETURN_OK:
                            fprintf(stdout, "[%u] Contains %lu (%s)\n", *pActionIndex, number, cfunc->description);
                            break;
                        case SET_RETURN_DOES_NOT_CONTAIN:
                            fprintf(stdout, "[%u] Does not contain %s\n", *pActionIndex, cfunc->description);
                            break;
                        default:
                            return "Cannot assert if set contains number or not";
                       }
                    }
                }
                if (!matchedCustomFunction)
                    return "Custom function not found";
                break;
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
                fprintf(stdout, "[%u] %lu\n", *pActionIndex, number);
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
                fprintf(stdout, "[%u] %lu\n", *pActionIndex, number);
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
    struct customFunction *cfunc = customFunctions;
    for (; *str; ++str) puts(*str);
    for (; cfunc->func; ++cfunc)
        printf("%s\tmatches %s\n", cfunc->label, cfunc->description);
}

CF_PROTOTYPE(even) {
    unsigned long number = (unsigned long) item;
    return number % 2 == 0;
}

CF_PROTOTYPE(square) {
    unsigned long number = (unsigned long) item;
    unsigned long aux = 1;
    // makes use of the following equation:
    // 1 + 3 + 5 + ... + (2*n - 1) = n^2
    while (number) {
        if (aux > number)
            return 0;
        number -= aux;
        aux += 2;
    }
    return 1;
}