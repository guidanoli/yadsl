#ifndef __YADSL_POSIXSTRING_H__
#define __YADSL_POSIXSTRING_H__

/* Definitions for including POSIX string definitions
   Include this instead of <string.h> */

#ifdef __STDC_ALLOC_LIB__
#  define __STDC_WANT_LIB_EXT2__ 1
#else
#  define _POSIX_C_SOURCE 200809L
#endif

#include <string.h>

/* Silence MSC complains about strdup */
#if defined(_MSC_VER)
#  pragma warning(disable : 4996)
#endif

#endif
