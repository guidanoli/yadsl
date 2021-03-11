#ifndef __YADSL_UTL_H__
#define __YADSL_UTL_H__

#include <limits.h>

/* Utility definitions */

/* Minimum buffer size for representing all
 * values in type T (including null byte)
 *
 * Comes from ceil(log10(2^N - 1)),
 * where N = #bits in T */

#define YADSL_BUFSIZ_FOR(T) \
	((CHAR_BIT * sizeof(T) * 28) / 93 + 2)

/* Absolute value of R
 * Warning: May evaluate R twice */

#define YADSL_ABS(R) \
	((R) < 0 ? (-R) : (R))

#endif
