#ifndef __MEMDB_H__
#define __MEMDB_H__

//
//       __  ___               ____  ____ 
//      /  |/  /__  ____ ___  / __ \/ __ )
//     / /|_/ / _ \/ __ `__ \/ / / / __  |
//    / /  / /  __/ / / / / / /_/ / /_/ / 
//   /_/  /_/\___/_/ /_/ /_/_____/_____/  
//                                        
// The Memory Debuger allows further investigation in cases of memory
// leakage due to unresponsible housekeeping. It overwrites the main
// dynamic allocation routines in order to keep track of the blocks
// are allocated and dealocatted in and from the heap.
//
// The idea is that, at the end of your program, the list of allocated
// data should be empty. Otherwise, this indicates that some memory
// block was not properly deallocated from the heap.
//
// It is important to include this header AFTER including stdlib.h
// and string.h in order to correctly override these functions.
//

#include <stddef.h>
#include <stdio.h>

// Get size of list of allocated data
size_t yadsl_memdb_list_size();

// Check if data is allocated in list
int yadsl_memdb_contains(void *_mem);

// Check if any error occurred
int yadsl_memdb_error_occurred();

// Clear list of allocated data
void yadsl_memdb_clear_list();

// Set logging output or NULL to reset
void yadsl_memdb_set_logger(FILE *fp);

// Overwritten (de)allocation functions
void yadsl_memdb_free(void *mem);
void *yadsl_memdb_malloc(size_t size, const char *file, const int line);
void *yadsl_memdb_realloc(void *mem, size_t size, const char *file, const int line);
void *yadsl_memdb_calloc(size_t cnt, size_t size, const char *file, const int line);
char *yadsl_memdb_strdup(const char *str, const char *file, const int line);

#ifndef _MEMDB_INTERNAL
#  ifndef _DEBUG
#    define yadsl_memdb_dump() ((void) 0)
#  else
#    define yadsl_memdb_dump() printf("MEMDB: %zu items in list\n", yadsl_memdb_list_size())
#    define free yadsl_memdb_free
#    define malloc(_size) yadsl_memdb_malloc(_size, __FILE__, __LINE__)
#    define realloc(_mem, _size) yadsl_memdb_realloc(_mem, _size, __FILE__, __LINE__)
#    define calloc(_cnt, _size) yadsl_memdb_calloc(_cnt, _size, __FILE__, __LINE__)
#    define strdup(_str) yadsl_memdb_strdup(_str, __FILE__, __LINE__)
#  endif /* _DEBUG */
#endif /* _MEMDB_INTERNAL */

#endif
