#ifndef __YADSL_MEMDB_MACROS_H__
#define __YADSL_MEMDB_MACROS_H__

#include <stdlib.h>
#include <memdb/memdb.h>

#define malloc(size) yadsl_memdb_malloc(size, __FILE__, __LINE__)
#define realloc(ptr, size) yadsl_memdb_realloc(ptr, size, __FILE__, __LINE__)
#define calloc(cnt, size) yadsl_memdb_calloc(cnt, size, __FILE__, __LINE__)
#define free(ptr) yadsl_memdb_free(ptr, __FILE__, __LINE__)

#endif
