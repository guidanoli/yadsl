#include <string.h>
#include <stdio.h>

#define _DEBUG
#include "memdb.h"
#undef _DEBUG /* Force debug macros */

#include "tester.h"

#define matches(a,b) (strcmp(a,b) == 0)

const char *TesterHelpStrings[] = {
    "This is the memdb test module",
    "Indices go from 0 to 9.",
    "",
    "/malloc <idx> <size>",
    "/realloc <idx> <size>",
    "/calloc <idx> <count> <size>",
    "/free <idx>",
    "/contains <idx> [YES/NO]",
    "/size <expected>",
    "/supress [YES/NO]",
    "/clear",
};

#define ARRSIZE 10
#define _VALID(idx) (idx < ARRSIZE)
void *mem_array[ARRSIZE];

char buffer[BUFSIZ];

TesterReturnValue TesterInitCallback()
{
    return TESTER_RETURN_OK;
}

TesterReturnValue TesterParseCallback(const char *command)
{
    if matches(command, "malloc") {
        size_t idx, size;
        void *p;
        if (TesterParseArguments("zz", &idx, &size) != 2)
            return TESTER_RETURN_ARGUMENT;
        if (!_VALID(idx))
            return TESTER_RETURN_ARGUMENT;
        if ((p = malloc(size)) == NULL)
            return TESTER_RETURN_MALLOC;
        mem_array[idx] = p;
    } else if matches(command, "realloc") {
        size_t idx, size;
        void *p;
        if (TesterParseArguments("zz", &idx, &size) != 2)
            return TESTER_RETURN_ARGUMENT;
        if (!_VALID(idx))
            return TESTER_RETURN_ARGUMENT;
        if ((p = realloc(mem_array[idx], size)) == NULL)
            return TESTER_RETURN_MALLOC;
        mem_array[idx] = p;
    } else if matches(command, "calloc") {
        size_t idx, size, count;
        void *p;
        if (TesterParseArguments("zzz", &idx, &size, &count) != 3)
            return TESTER_RETURN_ARGUMENT;
        if (!_VALID(idx))
            return TESTER_RETURN_ARGUMENT;
        if ((p = calloc(count, size)) == NULL)
            return TESTER_RETURN_MALLOC;
        mem_array[idx] = p;
    } else if matches(command, "free") {
        size_t idx;
        if (TesterParseArguments("z", &idx) != 1)
            return TESTER_RETURN_ARGUMENT;
        if (!_VALID(idx))
            return TESTER_RETURN_ARGUMENT;
        free(mem_array[idx]);
    } else if matches(command, "size") {
        size_t actual, expected;
        if (TesterParseArguments("z", &expected) != 1)
            return TESTER_RETURN_ARGUMENT;
        actual = _memdb_list_size();
        if (actual != expected)
            return TESTER_RETURN_RETURN;
    } else if matches(command, "clear") {
        _memdb_clear_list();
    } else if matches(command, "contains") {
        size_t idx;
        int expected, actual;
        if (TesterParseArguments("zs", &idx, buffer) != 2)
            return TESTER_RETURN_ARGUMENT;
        if (!_VALID(idx))
            return TESTER_RETURN_ARGUMENT;
        expected = matches(buffer, "YES");
        if (!expected && !matches(buffer, "NO"))
            TesterLog("Expected \"YES\" or \"NO\", but got \"%s\" instead."
                " (assumed NO)", buffer);
        actual = _memdb_contains(mem_array[idx]);
        if (actual != expected)
            return TESTER_RETURN_RETURN;
    } else {
        return TESTER_RETURN_COMMAND;
    }
    return TESTER_RETURN_OK;
}

TesterReturnValue TesterExitCallback()
{
    return TESTER_RETURN_OK;
}

