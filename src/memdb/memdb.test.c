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
#include <testerutils/testerutils.h>

const char *yadsl_tester_help_strings[] = {
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

yadsl_TesterRet yadsl_tester_init()
{
    return YADSL_TESTER_RET_OK;
}

yadsl_TesterRet yadsl_tester_parse(const char *command)
{
    if matches(command, "malloc") {
        size_t idx, size;
        void *p;
        if (yadsl_tester_parse_arguments("zz", &idx, &size) != 2)
            return YADSL_TESTER_RET_ARGUMENT;
        if (!_VALID(idx))
            return YADSL_TESTER_RET_ARGUMENT;
        if ((p = malloc(size)) == NULL)
            return YADSL_TESTER_RET_MALLOC;
        mem_array[idx] = p;
    } else if matches(command, "realloc") {
        size_t idx, size;
        void *p;
        if (yadsl_tester_parse_arguments("zz", &idx, &size) != 2)
            return YADSL_TESTER_RET_ARGUMENT;
        if (!_VALID(idx))
            return YADSL_TESTER_RET_ARGUMENT;
        if ((p = realloc(mem_array[idx], size)) == NULL)
            return YADSL_TESTER_RET_MALLOC;
        mem_array[idx] = p;
    } else if matches(command, "calloc") {
        size_t idx, size, count;
        void *p;
        if (yadsl_tester_parse_arguments("zzz", &idx, &size, &count) != 3)
            return YADSL_TESTER_RET_ARGUMENT;
        if (!_VALID(idx))
            return YADSL_TESTER_RET_ARGUMENT;
        if ((p = calloc(count, size)) == NULL)
            return YADSL_TESTER_RET_MALLOC;
        mem_array[idx] = p;
    } else if matches(command, "free") {
        size_t idx;
        if (yadsl_tester_parse_arguments("z", &idx) != 1)
            return YADSL_TESTER_RET_ARGUMENT;
        if (!_VALID(idx))
            return YADSL_TESTER_RET_ARGUMENT;
        free(mem_array[idx]);
    } else if matches(command, "size") {
        size_t actual, expected;
        if (yadsl_tester_parse_arguments("z", &expected) != 1)
            return YADSL_TESTER_RET_ARGUMENT;
        actual = yadsl_memdb_list_size();
        if (actual != expected)
            return YADSL_TESTER_RET_RETURN;
    } else if matches(command, "clear") {
        yadsl_memdb_clear_list();
    } else if matches(command, "contains") {
        size_t idx;
        bool expected, actual;
        if (yadsl_tester_parse_arguments("zs", &idx, buffer) != 2)
            return YADSL_TESTER_RET_ARGUMENT;
        if (!_VALID(idx))
            return YADSL_TESTER_RET_ARGUMENT;
        expected = TesterUtilsGetYesOrNoFromString(buffer);
        actual = yadsl_memdb_contains(mem_array[idx]);
        if (actual != expected)
            return YADSL_TESTER_RET_RETURN;
    } else {
        return YADSL_TESTER_RET_COMMAND;
    }
    return YADSL_TESTER_RET_OK;
}

yadsl_TesterRet yadsl_tester_release()
{
    return YADSL_TESTER_RET_OK;
}

