#include <yadsl/posixstring.h>
#include <stdio.h>

// Force debug being set on
#ifndef _DEBUG
#define _DEBUG
#define _PSEUDO_DEBUG
#endif
#include <memdb/memdb.h>
#ifdef _PSEUDO_DEBUG
#undef _DEBUG
#undef _PSEUDO_DEBUG
#endif

#include <tester/tester.h>

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
    return TESTER_OK;
}

TesterReturnValue TesterParseCallback(const char *command)
{
    if matches(command, "malloc") {
        size_t idx, size;
        void *p;
        if (TesterParseArguments("zz", &idx, &size) != 2)
            return TESTER_ARGUMENT;
        if (!_VALID(idx))
            return TESTER_ARGUMENT;
        if ((p = malloc(size)) == NULL)
            return TESTER_MALLOC;
        mem_array[idx] = p;
    } else if matches(command, "realloc") {
        size_t idx, size;
        void *p;
        if (TesterParseArguments("zz", &idx, &size) != 2)
            return TESTER_ARGUMENT;
        if (!_VALID(idx))
            return TESTER_ARGUMENT;
        if ((p = realloc(mem_array[idx], size)) == NULL)
            return TESTER_MALLOC;
        mem_array[idx] = p;
    } else if matches(command, "calloc") {
        size_t idx, size, count;
        void *p;
        if (TesterParseArguments("zzz", &idx, &size, &count) != 3)
            return TESTER_ARGUMENT;
        if (!_VALID(idx))
            return TESTER_ARGUMENT;
        if ((p = calloc(count, size)) == NULL)
            return TESTER_MALLOC;
        mem_array[idx] = p;
    } else if matches(command, "free") {
        size_t idx;
        if (TesterParseArguments("z", &idx) != 1)
            return TESTER_ARGUMENT;
        if (!_VALID(idx))
            return TESTER_ARGUMENT;
        free(mem_array[idx]);
    } else if matches(command, "size") {
        size_t actual, expected;
        if (TesterParseArguments("z", &expected) != 1)
            return TESTER_ARGUMENT;
        actual = _memdb_list_size();
        if (actual != expected)
            return TESTER_RETURN;
    } else if matches(command, "clear") {
        _memdb_clear_list();
    } else if matches(command, "contains") {
        size_t idx;
        int expected, actual;
        if (TesterParseArguments("zs", &idx, buffer) != 2)
            return TESTER_ARGUMENT;
        if (!_VALID(idx))
            return TESTER_ARGUMENT;
        expected = matches(buffer, "YES");
        if (!expected && !matches(buffer, "NO"))
            TesterLog("Expected \"YES\" or \"NO\", but got \"%s\" instead."
                " (assumed NO)", buffer);
        actual = _memdb_contains(mem_array[idx]);
        if (actual != expected)
            return TESTER_RETURN;
    } else {
        return TESTER_COMMAND;
    }
    return TESTER_OK;
}

TesterReturnValue TesterExitCallback()
{
    return TESTER_OK;
}

