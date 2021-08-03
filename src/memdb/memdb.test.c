#include <string.h>
#include <stdio.h>

/* Force debug being set on */
#ifndef YADSL_DEBUG
#define YADSL_DEBUG
#endif

#include <yadsl/stdlib.h>

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
    "/write <idx> <hex>",
    "/dump <idx> <expected-hex>",
};

#define ARRSIZE 10
#define _VALID(idx) (idx < ARRSIZE)
void *mem_array[ARRSIZE];
size_t amb_list_initial_size;
char buffer[BUFSIZ];
size_t filecnt;

static void clear_mem_array()
{
    memset(mem_array, 0, sizeof(mem_array));
}

yadsl_TesterRet yadsl_tester_init()
{
    amb_list_initial_size = yadsl_memdb_amb_list_size();
    return YADSL_TESTER_RET_OK;
}

static FILE* new_tmpfile()
{
    FILE *fp;
    char filename[256];
    snprintf(filename, sizeof(filename), "tmp%zu", filecnt++);
    fp = fopen(filename, "w+");
    if (fp != NULL) yadsl_testerutils_add_tempfile_to_list(filename);
    return fp;
}

yadsl_TesterRet yadsl_tester_parse(const char *command)
{
    if (yadsl_testerutils_match(command, "malloc")) {
        size_t idx, size;
        void *p;
        if (yadsl_tester_parse_arguments("zz", &idx, &size) != 2)
            return YADSL_TESTER_RET_ARGUMENT;
        if (!_VALID(idx))
            return YADSL_TESTER_RET_ARGUMENT;
        if (mem_array[idx])
            return YADSL_TESTER_RET_ARGUMENT;
        if ((p = malloc(size)) == NULL)
            return YADSL_TESTER_RET_MALLOC;
        mem_array[idx] = p;
    } else if (yadsl_testerutils_match(command, "realloc")) {
        size_t idx, size;
        void *p;
        if (yadsl_tester_parse_arguments("zz", &idx, &size) != 2)
            return YADSL_TESTER_RET_ARGUMENT;
        if (!_VALID(idx))
            return YADSL_TESTER_RET_ARGUMENT;
        if (!mem_array[idx])
            return YADSL_TESTER_RET_ARGUMENT;
        if ((p = realloc(mem_array[idx], size)) == NULL)
            return YADSL_TESTER_RET_MALLOC;
        mem_array[idx] = p;
    } else if (yadsl_testerutils_match(command, "calloc")) {
        size_t idx, size, count;
        void *p;
        if (yadsl_tester_parse_arguments("zzz", &idx, &size, &count) != 3)
            return YADSL_TESTER_RET_ARGUMENT;
        if (!_VALID(idx))
            return YADSL_TESTER_RET_ARGUMENT;
        if (mem_array[idx])
            return YADSL_TESTER_RET_ARGUMENT;
        if ((p = calloc(count, size)) == NULL)
            return YADSL_TESTER_RET_MALLOC;
        mem_array[idx] = p;
    } else if (yadsl_testerutils_match(command, "free")) {
        size_t idx;
        if (yadsl_tester_parse_arguments("z", &idx) != 1)
            return YADSL_TESTER_RET_ARGUMENT;
        if (!_VALID(idx))
            return YADSL_TESTER_RET_ARGUMENT;
        if (!mem_array[idx])
            return YADSL_TESTER_RET_ARGUMENT;
        free(mem_array[idx]);
        mem_array[idx] = NULL;
    } else if (yadsl_testerutils_match(command, "size")) {
        size_t actual, expected;
        if (yadsl_tester_parse_arguments("z", &expected) != 1)
            return YADSL_TESTER_RET_ARGUMENT;
        actual = yadsl_memdb_amb_list_size();
        if (actual != expected + amb_list_initial_size)
            return YADSL_TESTER_RET_RETURN;
    } else if (yadsl_testerutils_match(command, "clear")) {
        clear_mem_array();
        yadsl_memdb_clear_amb_list_from_file(__FILE__);
    } else if (yadsl_testerutils_match(command, "contains")) {
        size_t idx;
        bool expected, actual;
        if (yadsl_tester_parse_arguments("zs", &idx, buffer) != 2)
            return YADSL_TESTER_RET_ARGUMENT;
        if (!_VALID(idx))
            return YADSL_TESTER_RET_ARGUMENT;
        expected = yadsl_testerutils_str_to_bool(buffer);
        actual = yadsl_memdb_contains_amb(mem_array[idx]);
        if (actual != expected)
            return YADSL_TESTER_RET_RETURN;
    } else if (yadsl_testerutils_match(command, "write")) {
        size_t idx;
        if (yadsl_tester_parse_arguments("zs", &idx, buffer) != 2)
            return YADSL_TESTER_RET_ARGUMENT;
        if (!_VALID(idx))
            return YADSL_TESTER_RET_ARGUMENT;
        if (!mem_array[idx])
            return YADSL_TESTER_RET_ARGUMENT;
        strcpy((char*) mem_array[idx], buffer);
    } else if (yadsl_testerutils_match(command, "dump")) {
        size_t idx;
        FILE* fp;
        bool equal;
        if (yadsl_tester_parse_arguments("zs", &idx, buffer) != 2)
            return YADSL_TESTER_RET_ARGUMENT;
        if (!_VALID(idx))
            return YADSL_TESTER_RET_ARGUMENT;
        if (!mem_array[idx])
            return YADSL_TESTER_RET_ARGUMENT;
        fp = new_tmpfile();
        if (fp == NULL)
            return YADSL_TESTER_RET_FILE;
        yadsl_memdb_dump(fp, mem_array[idx]);
        rewind(fp);
        equal = yadsl_testerutils_compare_file_and_string(fp, buffer);
        fclose(fp);
        if (!equal) {
            rewind(fp);
            yadsl_tester_log("Contents of file:");
            yadsl_memdb_dump(stderr, mem_array[idx]);
            fputc('\n', stderr);
            yadsl_tester_log("Contents of string:");
            fprintf(stderr, "%s\n", buffer);
        }
        if (!equal)
            return YADSL_TESTER_RET_RETURN;
    } else {
        return YADSL_TESTER_RET_COMMAND;
    }
    return YADSL_TESTER_RET_OK;
}

yadsl_TesterRet yadsl_tester_release()
{
    clear_mem_array();
    yadsl_testerutils_clear_tempfile_list();
    return YADSL_TESTER_RET_OK;
}

