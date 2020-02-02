#include <stdio.h>
#include <string.h>
#include "set.h"
#include "var.h"
#include "tester.h"

/* Help */

const char *TesterHelpStrings[] = {
    "This is an interactive module of the set library",
    "You interact with a single set object at all times",
    "",
    "The registered actions are the following:",
    "/save <var>               save variable",
    "/contains [YES/NO]        check if set contains saved variable",
    "/filter <var> [YES/NO]    check if filter matches one variable",
    "/filtersave <var>         filter variable and save it",
    "/add                      add saved variable to set",
    "/remove                   remove saved variable from set",
    "/current <expected>       get variable pointed by the cursor",
    "/size <expected>          get set size",
    "/previous                 move cursor to previous variable",
    "/next                     move cursor to next variable",
    "/first                    move cursor to first variable",
    "/last                     move cursor to last variable",
    NULL, /* Sentinel */
};

TesterReturnValue convertReturn(SetReturnID setId) {
    switch (setId) {
    case SET_RETURN_OK:
        return TESTER_RETURN_OK;
    case SET_RETURN_INVALID_PARAMETER:
        return TesterExternalReturnValue("invalid");
    case SET_RETURN_MEMORY:
        return TesterExternalReturnValue("malloc");
    case SET_RETURN_CONTAINS:
        return TesterExternalReturnValue("contains");
    case SET_RETURN_DOES_NOT_CONTAIN:
        return TesterExternalReturnValue("containsnot");
    case SET_RETURN_EMPTY:
        return TesterExternalReturnValue("empty");
    case SET_RETURN_OUT_OF_BOUNDS:
        return TesterExternalReturnValue("bounds");
    default:
        return TesterExternalReturnValue("unknown");
    }
}

/* Set object */
Set *pSet = NULL;
Variable *pVariable = NULL;

char buffer[TESTER_BUFFER_SIZE], arg[TESTER_BUFFER_SIZE];

TesterReturnValue TesterInitCallback()
{
    SetReturnID setId;
    if (setId = setCreate(&pSet))
        return convertReturn(setId);
    return TESTER_RETURN_OK;
}

#define matches(a, b) (strcmp(a, b) == 0)

int filterItem(void *item, void *arg)
{
    Variable *varItem = (Variable *) item;
    Variable *varArg = (Variable *) arg;
    int result;
    if (varCompare(varItem, varArg, &result))
        return 0;
    return result;
}

TesterReturnValue TesterParseCallback(const char *command)
{
    SetReturnID setId = SET_RETURN_OK;
    Variable *temp;
    if matches(command, "save") {
        if (TesterParseArguments("s", buffer) != 1)
            return TESTER_RETURN_ARGUMENT;
        if (varCreate(buffer, &temp))
            return TESTER_RETURN_MALLOC;
        if (setContainsItem(pSet, pVariable) != SET_RETURN_CONTAINS)
            varDestroy(pVariable);
        pVariable = temp;
    } else if matches(command, "contains") {
        int expected, actual;
        if (TesterParseArguments("s", arg) != 1)
            return TESTER_RETURN_ARGUMENT;
        expected = matches(arg, "YES");
        if (pVariable == NULL)
            TesterLog("Found no variable saved. Checking if contains NULL.");
        setId = setContainsItem(pSet, pVariable);
        actual = (setId == SET_RETURN_CONTAINS);
        if (actual != expected)
            return TESTER_RETURN_RETURN;
        else
            setId = SET_RETURN_OK;
    } else if matches(command, "filter") {
        int actual, expected;
        Variable *foundVar;
        if (TesterParseArguments("ss", buffer, arg) != 2)
            return TESTER_RETURN_ARGUMENT;
        if (varCreate(buffer, &temp))
            return TESTER_RETURN_MALLOC;
        expected = matches(arg, "YES");
        setId = setFilterItem(pSet, filterItem, temp, &foundVar);
        varDestroy(temp);
        actual = (setId == SET_RETURN_OK);
        if (actual != expected)
            return TESTER_RETURN_RETURN;
        else
            setId = SET_RETURN_OK;
    } else if matches(command, "filtersave") {
        Variable *foundVar;
        if (TesterParseArguments("s", buffer) != 1)
            return TESTER_RETURN_ARGUMENT;
        if (varCreate(buffer, &temp))
            return TESTER_RETURN_MALLOC;
        setId = setFilterItem(pSet, filterItem, temp, &foundVar);
        if (setId == SET_RETURN_OK) {
            if (pVariable != NULL &&
                setContainsItem(pSet, pVariable) != SET_RETURN_CONTAINS)
                varDestroy(pVariable);
            pVariable = foundVar;
        }
        varDestroy(temp);
    } else if matches(command, "add") {
        if (pVariable == NULL)
            TesterLog("Found no variable saved. Adding NULL.");
        setId = setAddItem(pSet, pVariable);
    } else if matches(command, "remove") {
        if (pVariable == NULL)
            TesterLog("Found no variable saved. Removing NULL.");
        setId = setRemoveItem(pSet, pVariable);
    } else if matches(command, "current") {
        Variable *curr;
        if (TesterParseArguments("s", buffer) != 1)
            return TESTER_RETURN_ARGUMENT;
        if (varCreate(buffer, &temp))
            return TESTER_RETURN_MALLOC;
        if (setId = setGetCurrentItem(pSet, &curr)) {
            varDestroy(temp);
        } else {
            int equal;
            if (curr == NULL) {
                varDestroy(temp);
                TesterLog("The current item is NULL");
                return TESTER_RETURN_RETURN;
            }
            varCompare(temp, curr, &equal);
            varDestroy(temp);
            if (!equal) {
                varWrite(curr, stdout);
                TesterLog(" is the current item");
                return TESTER_RETURN_RETURN;
            }
        }
    } else if matches(command, "size") {
        unsigned long expected, actual;
        if (TesterParseArguments("i", &expected) != 1)
            return TESTER_RETURN_ARGUMENT;
        setId = setGetSize(pSet, &actual);
        if (setId == SET_RETURN_OK && actual != expected)
            return TESTER_RETURN_RETURN;
    } else if matches(command, "previous") {
        setId = setPreviousItem(pSet);
    } else if matches(command, "next") {
        setId = setNextItem(pSet);
    } else if matches(command, "first") {
        setId = setFirstItem(pSet);
    } else if matches(command, "last") {
        setId = setLastItem(pSet);
    } else {
        return TESTER_RETURN_COMMAND;
    }
    return convertReturn(setId);
}

void freeItem(void *item, void *arg) {
    Variable *var = (Variable *) item;
    if (var == pVariable)
        pVariable = NULL;
    varDestroy(var);
}

TesterReturnValue TesterExitCallback()
{
    setDestroyDeep(pSet, freeItem, NULL);
    if (pVariable != NULL)
        varDestroy(pVariable);
#ifdef _DEBUG
    if (varGetRefCount())
        return TESTER_RETURN_MEMLEAK;
#endif
    return TESTER_RETURN_OK;
}