#ifndef __COMMON_H__
#define __COMMON_H__

// Common header, which defines key symbols for the aa modules

#include <stdio.h>
#include <assert.h>

// Debug printing function
#define dprintf(format) fprintf(stderr, __FILE__ ":%u " format "\n", __LINE__)
#define dprintf(format, ...) fprintf(stderr, __FILE__ ":%u " format "\n", __LINE__, __VA_ARGS__)

// Unreachable code assertino
#define unreachable() \
do { \
	dprintf("Unreachable code"); \
	assert(0); \
} while(0)

#endif