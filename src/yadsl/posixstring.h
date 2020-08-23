#ifndef _YADSL_POSIXSTRING_H_
#define _YADSL_POSIXSTRING_H_

// Definitions for including POSIX string definitions
// Include this instead of <string.h>

#ifdef __STDC_ALLOC_LIB__
/* push macro __STDC_ALLOC_LIB__ 1 */
#ifdef __STDC_WANT_LIB_EXT2__
#define __STDC_WANT_LIB_EXT2___HAD_BEFORE 1
#define __STDC_WANT_LIB_EXT2___BEFORE __STDC_WANT_LIB_EXT2__
#undef __STDC_WANT_LIB_EXT2__
#else
#define __STDC_WANT_LIB_EXT2___HAD_BEFORE 0
#endif
#define __STDC_WANT_LIB_EXT2__ 1
#else
/* push macro _POSIX_C_SOURCE 200809L */
#ifdef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE_HAD_BEFORE 1
#define _POSIX_C_SOURCE_BEFORE _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#else
#define _POSIX_C_SOURCE_HAD_BEFORE 0
#endif
#define _POSIX_C_SOURCE 200809L
#endif

#include <string.h>

#ifdef __STDC_ALLOC_LIB__
/* pop macro __STDC_ALLOC_LIB__ */
#undef __STDC_WANT_LIB_EXT2__
#if __STDC_WANT_LIB_EXT2___HAD_BEFORE
#define __STDC_WANT_LIB_EXT2__ __STDC_WANT_LIB_EXT2___BEFORE
#endif
#else
/* pop macro _POSIX_C_SOURCE */
#undef _POSIX_C_SOURCE
#if _POSIX_C_SOURCE_HAD_BEFORE
#define _POSIX_C_SOURCE _POSIX_C_SOURCE_BEFORE
#endif
#endif

#endif
