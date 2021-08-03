#ifndef yadsl_stdlib_h
#define yadsl_stdlib_h

#include <stdlib.h>

/* On debug builds, override malloc, realloc, calloc and free
 * with wrapper functions from the memdb library */
#ifdef YADSL_DEBUG
#   include <memdb/memdb.h>
#   define malloc(size) yadsl_memdb_malloc(size, __FILE__, __LINE__)
#   define realloc(mem, size) yadsl_memdb_realloc(mem, size, __FILE__, __LINE__)
#   define calloc(cnt, size) yadsl_memdb_calloc(cnt, size, __FILE__, __LINE__)
#   define free yadsl_memdb_free
#endif

#endif
