#ifndef __YADSL_MEMDB_MACROS_H__
#define __YADSL_MEMDB_MACROS_H__

#include <stdlib.h>
#include <memdb/memdb.h>
#define malloc(size) yadsl_memdb_malloc(size, __FILE__, __LINE__)
#define realloc(mem, size) yadsl_memdb_realloc(mem, size, __FILE__, __LINE__)
#define calloc(cnt, size) yadsl_memdb_calloc(cnt, size, __FILE__, __LINE__)
#define free yadsl_memdb_free

#endif
