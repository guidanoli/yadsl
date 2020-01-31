#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "var.h"
#include "tester.h"

#ifndef VARCNT
#define VARCNT 10
#endif

#define _STR(X) #X
#define XSTR(X) _STR(X)

#define matches(a, b) (strcmp(a, b) == 0)
#define isValidIndex(i) (i >= 0 && i < VARCNT)

static Variable *vars[VARCNT];
static char buffer[TESTER_BUFFER_SIZE];

static TesterReturnValue convertReturnValue(VarReturnID varId);

TesterReturnValue TesterInitCallback()
{
    static Variable **pVars = vars;
    size_t i, size = sizeof(vars)/sizeof(*vars);
    for (i = 0; i < size; ++i) *(pVars++) = NULL;
    return TESTER_RETURN_OK;
}

TesterReturnValue TesterParseCallback(const char *command)
{
    VarReturnID varId = VAR_RETURN_OK;
    int idx1, idx2, expected, obtained;
    FILE *f;
    if matches(command, "add") {
        if (TesterParseArguments("is", &idx1, buffer) != 2)
            return TESTER_RETURN_PARSING_ARGUMENT;
        if (!isValidIndex(idx1))
            return VARTEST_RETURN_INVALID_INDEX;
        if (vars[idx1]) {
            varDestroy(vars[idx1]);
            vars[idx1] = NULL;
        }
        varId = varCreate(buffer, &vars[idx1]);
    } else if matches(command, "rmv") {
        if (TesterParseArguments("i", &idx1) != 1)
            return TESTER_RETURN_PARSING_ARGUMENT;
        if (!isValidIndex(idx1))
            return VARTEST_RETURN_INVALID_INDEX;
        if (!vars[idx1])
            return VARTEST_RETURN_UNDEFINED_VARIABLE;
        varDestroy(vars[idx1]);
        vars[idx1] = NULL;
    } else if matches(command, "cmp") {
        if (TesterParseArguments("iis", &idx1, &idx2, buffer) != 3)
            return TESTER_RETURN_PARSING_ARGUMENT;
        if (!isValidIndex(idx1) || !isValidIndex(idx2))
            return VARTEST_RETURN_INVALID_INDEX;
        if (!vars[idx1] || !vars[idx2])
            return VARTEST_RETURN_UNDEFINED_VARIABLE;
        expected = matches(buffer, "EQUAL");
        varId = varCompare(vars[idx1], vars[idx2], &obtained);
        if (varId == VAR_RETURN_OK && obtained != expected)
            return TESTER_RETURN_UNEXPECTED_RETURN;
    } else if matches(command, "dmp") {
        if (TesterParseArguments("i", &idx1) != 1)
            return TESTER_RETURN_PARSING_ARGUMENT;
        if (!isValidIndex(idx1))
            return VARTEST_RETURN_INVALID_INDEX;
        if (!vars[idx1])
            return VARTEST_RETURN_UNDEFINED_VARIABLE;
        varId = varWrite(vars[idx1], stdout);
        if (varId == VAR_RETURN_OK)
            printf("\n");
    } else if matches(command, "dsr") {
        if (TesterParseArguments("is", &idx1, buffer) != 2)
            return TESTER_RETURN_PARSING_ARGUMENT;
        if (!isValidIndex(idx1))
            return VARTEST_RETURN_INVALID_INDEX;
        if (vars[idx1]) {
            varDestroy(vars[idx1]);
            vars[idx1] = NULL;
        }
        if ((f = fopen(buffer, "r")) == NULL)
            return VARTEST_RETURN_OPEN_FILE;
        varId = varDeserialize(&vars[idx1], f);
        fclose(f);
    } else if matches(command, "ser") {
        if (TesterParseArguments("is", &idx1, buffer) != 2)
            return TESTER_RETURN_PARSING_ARGUMENT;
        if (!isValidIndex(idx1))
            return VARTEST_RETURN_INVALID_INDEX;
        if (!vars[idx1])
            return VARTEST_RETURN_UNDEFINED_VARIABLE;
        if ((f = fopen(buffer, "w")) == NULL)
            return VARTEST_RETURN_OPEN_FILE;
        varId = varSerialize(vars[idx1], f);
        fclose(f);
    } else if matches(command, "wef") {
        if (TesterParseArguments("s", buffer) != 1)
            return TESTER_RETURN_PARSING_ARGUMENT;
        if ((f = fopen(buffer, "w")) == NULL)
            return VARTEST_RETURN_OPEN_FILE;
        fclose(f);
    } else {
        return TESTER_RETURN_PARSING_COMMAND;
    }
    return convertReturnValue(varId);
}

TesterReturnValue TesterExitCallback()
{
    size_t i;
    for (i = 0; i < VARCNT; ++i) {
        if (vars[i] != NULL) {
            varDestroy(vars[i]);
            vars[i] = NULL;
        }
    }
#ifdef _DEBUG
    if (varGetRefCount()) {
        puts("Memory leak detected");
        return TESTER_RETURN_MEMORY_LEAK;
    }
#endif
    return TESTER_RETURN_OK;
}

static TesterReturnValue convertReturnValue(VarReturnID varId)
{
    switch (varId) {
    case VAR_RETURN_OK:
        return TESTER_RETURN_OK;
    case VAR_RETURN_INVALID_PARAMETER:
        return VARTEST_RETURN_INVALID_PARAMETER;
    case VAR_RETURN_FILE_FORMAT_ERROR:
        return VARTEST_RETURN_FILE_FORMAT_ERROR;
    case VAR_RETURN_WRITING_ERROR:
        return VARTEST_RETURN_WRITING_ERROR;
    case VAR_RETURN_MEMORY:
        return VARTEST_RETURN_MEMORY;
    default:
        return VARTEST_RETURN_UNKNOWN;
    }
}

const char *TesterLoadCustomReturnValueInfo(TesterReturnValue value)
{
    switch (value) {
    case VARTEST_RETURN_INVALID_INDEX:
        return "Invalid index (must be between 0 and " XSTR(VARCNT-1) ")";
    case VARTEST_RETURN_UNDEFINED_VARIABLE:
        return "Variable at that index is not yet initialized";
    case VARTEST_RETURN_OPEN_FILE:
        return "Could not open file for reading or writting";
    case VARTEST_RETURN_INVALID_PARAMETER:
        return "Parameter passed to function is invalid";
    case VARTEST_RETURN_FILE_FORMAT_ERROR:
        return "Serialized form is corrupted";
    case VARTEST_RETURN_WRITING_ERROR:
        return "Could not write to file";
    case VARTEST_RETURN_MEMORY:
        return "Could not allocate memory space (in var module)";
    case VARTEST_RETURN_UNKNOWN:
        return "Unkown error return value";
    default:
        return NULL;
    }
}

const char *TesterHelpStrings[] = {
    "This is an interactive module of the variable library",
    "You can interact with up to " XSTR(VARCNT_S) " variables at a time",
    "",
    "/add <index> <text>                add variable from text to index",
    "/rmv <index>                       remove variable at index",
    "/cmp <index1> <index2> [EQUAL|*]   compare variables from two indices",
    "/dmp <index>                       dump variable contents",
    "/ser <index> <filename>            serialize to file",
    "/dsr <index> <filename>            deserialize from file",
    "/wef <filename>                    write empty file",
    NULL,
};