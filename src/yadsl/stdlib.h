#ifndef yadsl_stdlib_h
#define yadsl_stdlib_h

#ifdef YADSL_DEBUG
/* On debug builds, override malloc, realloc, calloc and free
 * with wrapper functions from the memdb library */
#include <memdb/stdlib.h>
#endif
#include <stdlib.h>
#endif
