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
size_t _memdb_list_size();

// Check if data is allocated in list
int _memdb_contains(void *_mem);

// Check if any error occurred
int _memdb_error_occurred();

// Clear list of allocated data
void _memdb_clear_list();

// Set logging output or NULL to reset
void _memdb_set_logger(FILE *fp);

// Overwritten (de)allocation functions
void _memdb_free(void *mem);
void *_memdb_malloc(size_t size, const char *file, const int line);
void *_memdb_realloc(void *mem, size_t size, const char *file, const int line);
void *_memdb_calloc(size_t cnt, size_t size, const char *file, const int line);
char *_memdb_strdup(const char *str, const char *file, const int line);

#ifndef _MEMDB_INTERNAL
#  ifndef _DEBUG
#    define _memdb_dump() ((void) 0)
#  else
#    define _memdb_dump() printf("MEMDB: %zu items in list\n", _memdb_list_size())
#    define free _memdb_free
#    define malloc(_size) _memdb_malloc(_size, __FILE__, __LINE__)
#    define realloc(_mem, _size) _memdb_realloc(_mem, _size, __FILE__, __LINE__)
#    define calloc(_cnt, _size) _memdb_calloc(_cnt, _size, __FILE__, __LINE__)
#    define strdup(_str) _memdb_strdup(_str, __FILE__, __LINE__)
#  endif /* _DEBUG */
#endif /* _MEMDB_INTERNAL */

#endif
