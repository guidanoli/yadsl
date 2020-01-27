#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "map.h"
#include "var.h"

#ifndef MAP_TEST_BUFFER_SIZE
#define MAP_TEST_BUFFER_SIZE 256
#endif

#define _SEP "," /* Key-value separator */
#define _ADD "+" /* Add entry */
#define _REM "-" /* Remove entry */
#define _GET "?" /* Get entry */
#define _NUM "n" /* Get number of entries */

static const char *helpStrings[] = {
    "This is an interactive module of the map library",
    "You interact with a single map object at all times",
    "Actions to the map are parsed by the command line arguments",
    "",
    _NUM "\tget number of entries",
    "K" _SEP "V" _ADD "\tadd entry of key K and value V",
    "K" _REM "\tremove entry of key K and value V",
    "K" _GET "\tget entry of key K",
    NULL, /* Sentinel */
};

// Variable functions
static int _cmpVariables(struct Variable *pVariableA, struct Variable *pVariableB);
static void _freeVariableCallback(struct Variable *pKeyVariable, struct Variable *pValueVariable, void *arg);
#define _varDestroyMultiple(pVariable, ...) __varDestroyMultiple(pVariable, __VA_ARGS__, NULL)
static void __varDestroyMultiple(struct Variable *pVariable, ...);

// Testing functions
static void _printHelpMessage();
static char *_parseActions(Map **ppMap, MapReturnID *pMapId,
    unsigned int *pActionIndex, char **actions, int actionCount);

int main(int argc, char **argv)
{
    Map *pTestMap = NULL;
    MapReturnID mapId;
    unsigned int actionIndex = 0;
    char *errorString = NULL;
    int refCount;
    if (argc == 1) {
        _printHelpMessage();
        return 0;
    }
    errorString = _parseActions(&pTestMap, &mapId, &actionIndex,
        argv + 1, argc - 1);
    if (pTestMap != NULL)
        mapDestroy(pTestMap);
#ifdef _DEBUG
    if ((refCount = varGetRefCount()) != 0)
        printf("%lld leaked variables!\n", refCount);
#endif
    pTestMap = NULL;
    if (errorString != NULL) {
        if (mapId != MAP_RETURN_OK) {
            fprintf(stderr, "Error #%d on action #%d (%s): %s\n",
                mapId, actionIndex, argv[actionIndex], errorString);
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

static void _printHelpMessage() {
    const char **str = helpStrings;
    for (; *str; ++str) puts(*str);
}

static char *_parseActions(Map **ppMap, MapReturnID *pMapId,
    unsigned int *pActionIndex, char **actions, int actionCount)
{
    int i;
    if (*pMapId = mapCreate(ppMap, _cmpVariables, _freeVariableCallback, NULL)) {
        if (*pMapId == MAP_RETURN_MEMORY)
            *ppMap = NULL;
        return "Could not create map";
    }
    for (i = 0; i < actionCount; ++i) {
        char character, end = (char) 0;
        struct Variable *pKeyVariable, *pValueVariable;
        char keyBuffer[MAP_TEST_BUFFER_SIZE], valueBuffer[MAP_TEST_BUFFER_SIZE];
        *pActionIndex = i + 1;
        if (sscanf(actions[i], "%[^" _SEP "]" _SEP "%[^" _ADD "]%c%c",
            keyBuffer, valueBuffer, &character, &end) == 3 && !end) {
            // K,V+
            if (varCreate(keyBuffer, &pKeyVariable))
                return "Could not allocate memory space for key";
            if (varCreate(valueBuffer, &pValueVariable)) {
                varDestroy(pKeyVariable);
                return "Could not allocate memory space for value";
            }
            if (character == _ADD[0]) {
                // Add entry
                struct Variable *previousVariable = NULL;
                if (*pMapId = mapPutEntry(*ppMap, pKeyVariable, pValueVariable, &previousVariable)) {
                    if (*pMapId == MAP_RETURN_OVERWROTE_ENTRY) {
                        varDestroy(pKeyVariable);
                    } else {
                        _varDestroyMultiple(pKeyVariable, pValueVariable);
                        return "Could not put entry";
                    }
                }
                if (previousVariable) {
                    fprintf(stdout, "[%u] Previous value was ", *pActionIndex);
                    varWrite(previousVariable, stdout);
                    fprintf(stdout, "\n");
                    varDestroy(previousVariable);
                }
            } else {
                _varDestroyMultiple(pKeyVariable, pValueVariable);
                return "Could not parse last character in action";
            }
        } else if (sscanf(actions[i], "%[^" _GET _REM "]%c%c",
            keyBuffer, &character, &end) == 2 && !end) {
            // K?, K-
            if (varCreate(keyBuffer, &pKeyVariable))
                return "Could not allocate memory space for key";
            if (character == _GET[0]) {
                // Get entry
                switch (*pMapId = mapGetEntry(*ppMap, pKeyVariable, &pValueVariable)) {
                case MAP_RETURN_OK:
                    fprintf(stdout, "[%u] Value of key ", *pActionIndex);
                    varWrite(pKeyVariable, stdout);
                    fprintf(stdout, " is ");
                    varWrite(pValueVariable, stdout);
                    fprintf(stdout, "\n");
                    break;
                case MAP_RETURN_ENTRY_NOT_FOUND:
                    fprintf(stdout, "[%u] Entry of key ", *pActionIndex);
                    varWrite(pKeyVariable, stdout);
                    fprintf(stdout, " does not exist\n");
                    break;
                default:
                    varDestroy(pKeyVariable);
                    return "Could not get entry";
                }
                varDestroy(pKeyVariable);
            } else if (character == _REM[0]) {
                // Remove entry 
                struct Variable *pReturnedKeyVariable = NULL, *pReturnedValueVariable = NULL;
                if (*pMapId = mapRemoveEntry(*ppMap, pKeyVariable, &pReturnedKeyVariable,
                    &pReturnedValueVariable)) {
                    varDestroy(pKeyVariable);
                    return "Could not remove entry";
                }
                if (pReturnedValueVariable) {
                    fprintf(stdout, "[%u] Value was ", *pActionIndex);
                    varWrite(pReturnedValueVariable, stdout);
                    fprintf(stdout, "\n");
                }
                _varDestroyMultiple(pKeyVariable, pReturnedKeyVariable, pReturnedValueVariable);
            } else {
                varDestroy(pKeyVariable);
                return "Could not parse last character in action";
            }
        } else if (sscanf(actions[i], "%c%c",
            &character, &end) == 1 && !end) {
            // n
            if (character == _NUM[0]) {
                unsigned long numOfEntries;
                if (*pMapId = mapGetNumberOfEntries(*ppMap, &numOfEntries))
                    return "Could not get number of entries";
                fprintf(stdout, "[%u] %lu entries\n", *pActionIndex, numOfEntries);
            } else {
                return "Could not parse character";
            }
        } else {
            return "Could not parse action";
        }
    }
    return NULL;
}

static void __varDestroyMultiple(Variable *pVariable, ...)
{
    va_list va;
    for (va_start(va, pVariable); /* initialize */
        pVariable; /* condition (not reached sentinel) */
        pVariable = va_arg(va, Variable *) /* next variable */ )
        varDestroy(pVariable); /* free the variable */
    va_end(va);
}

static void _freeVariableCallback(struct Variable *pKeyVariable, struct Variable *pValueVariable, void *arg)
{
    _varDestroyMultiple(pKeyVariable, pValueVariable);
}

static int _cmpVariables(struct Variable *pVariableA, struct Variable *pVariableB)
{
    int cmpReturn = 0; // by default, aren't equal
    if (varCompare(pVariableA, pVariableB, &cmpReturn)) {
#ifdef _DEBUG
        puts("Error while trying to compare variables.");
#endif
    }
    return cmpReturn;
}