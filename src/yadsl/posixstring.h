#ifndef _YADSL_POSIXSTRING_H_
#define _YADSL_POSIXSTRING_H_

// Definitions for including POSIX string definitions
// Include this instead of <string.h>

#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

#include <string.h>

#endif
