#include <bigint/bigint.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <limits.h>

#ifdef YADSL_DEBUG
#include <memdb/memdb.h>
#else
#include <stdlib.h>
#endif

typedef uint32_t digit;
typedef int32_t sdigit;

#define SHIFT (sizeof(digit)*CHAR_BIT - 1)
#define ABS(num) ((num < 0) ? -(num) : (num))
#define SIZE(bigint) (bigint->size)
#define DIGITVALUE(bigint) \
	(assert(-1 <= SIZE(bigint) && SIZE(bigint) <= 1), \
		SIZE(bigint) < 0 ? -(sdigit)(bigint)->digits[0] : \
	 		                (SIZE(bigint) == 0 ? (sdigit)0 : \
			                                     (sdigit)(bigint)->digits[0]))

/**
 * Big integer representation
 * 
 * Each digit contains at least 31 bits.
 * The most significant digit is zeroed.
 *
 * 0XXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX
 *
 * The size field is signed so that
 * the sign bit indicates the sign of
 * the number itself.
*/
typedef struct
{
	intptr_t size;
	digit digits[1];
}
BigInt;

static int getndigits(intmax_t i)
{
	uintmax_t u;
	int ndigits = 0;
	if (i == INTMAX_MIN) {
		u = UINTMAX_MAX;
	} else if (i < 0) {
		u = (uintmax_t)-i;
	} else {
		u = (uintmax_t)i;
	}
	while (u != 0) {
		u >>= SHIFT;
		++ndigits;
	}
	return ndigits;
}

static void
yadsl_bigint_dump_internal(BigInt* bigint)
{
	intptr_t size = SIZE(bigint);
	intptr_t ndigits = ABS(size);
	fprintf(stderr, "isnegative = %s\n", size < 0 ? "true" : "false");
	fprintf(stderr, "ndigits = %zd\n", ndigits);
	for (intptr_t i = 0; i < ndigits; ++i)
		fprintf(stderr, "digit #%zd = %" PRIu32 "\n", i, bigint->digits[i]);
}

yadsl_BigIntHandle*
yadsl_bigint_from_int(intmax_t i)
{
	int ndigits = getndigits(i);
	BigInt* bigint = malloc(sizeof(*bigint)+(ndigits-1)*sizeof(digit));
	if (bigint != NULL) {
		bigint->size = i > 0 ? ndigits : -ndigits;
		if (i == INTMAX_MIN) {
			for (int ndigit = 0; ndigit < ndigits-1; ++ndigit)
				bigint->digits[ndigit] = 0;
			bigint->digits[ndigits-1] = 2;
		} else {
			uintmax_t u = i > 0 ? (uintmax_t)i : (uintmax_t)-i;
			for (int ndigit = 0; ndigit < ndigits; ++ndigit) {
				bigint->digits[ndigit] = (digit)u & ~(1 << SHIFT);
				u >>= SHIFT;
			}
		}
	}
	return bigint;
}

bool
yadsl_bigint_to_int(
	yadsl_BigIntHandle* _bigint,
	intmax_t* i_ptr)
{
	intmax_t i;
	uintmax_t u, v;
	int sign;
	intptr_t ndigits;
	intptr_t size;
	BigInt* bigint = (BigInt *) _bigint;
	switch (SIZE(bigint)) {
	case 0:
		i = 0;
		break;
	case 1:
		i = (sdigit)bigint->digits[0];
		break;
	case -1:
		i = -(sdigit)bigint->digits[0];
		break;
	default:
		size = SIZE(bigint);
		ndigits = ABS(size);
		sign = size < 0 ? -1 : 1;
		u = 0;
		while (ndigits > 0) {
			v = u;
			u = (u << SHIFT) | bigint->digits[--ndigits];
			if ((u >> SHIFT) != v) return false;
		}
		if (u <= (uintmax_t)INTMAX_MAX)
			i = (intmax_t)u * sign;
		else if (sign < 0 && u == (0-(uintmax_t)INTMAX_MIN))
			i = INTMAX_MIN;
		else
			return false;
	}
	*i_ptr = i;
	return true;
}

yadsl_BigIntHandle*
yadsl_bigint_copy(
	yadsl_BigIntHandle* _bigint)
{
	BigInt* bigint, * copy;
	size_t size;
	intptr_t ndigits;
	bigint = (BigInt*) _bigint;
	ndigits = ABS(SIZE(bigint));
	size = sizeof(*copy)+(ndigits-1)*sizeof(digit);
	copy = malloc(size);
	if (copy != NULL) memcpy(copy, bigint, size);
	return copy;
}

yadsl_BigIntHandle*
yadsl_bigint_opposite(
	yadsl_BigIntHandle* _bigint)
{
	BigInt* bigint, * copy;
	bigint = (BigInt*) _bigint;
	if (SIZE(bigint) == INTPTR_MIN)
		return NULL;
	copy = yadsl_bigint_copy(_bigint);
	if (copy != NULL) SIZE(copy) *= -1;
	return copy;
}

yadsl_BigIntHandle*
yadsl_bigint_add(
	yadsl_BigIntHandle* _bigint1,
	yadsl_BigIntHandle* _bigint2)
{
	BigInt* bigint1, * bigint2;
	intptr_t size1, size2;
	bigint1 = (BigInt*) _bigint1;
	bigint2 = (BigInt*) _bigint2;
	size1 = SIZE(bigint1);
	size2 = SIZE(bigint2);
	if (size1 == 0) {
		return yadsl_bigint_copy(_bigint2);
	} else if (size2 == 0) {
		return yadsl_bigint_copy(_bigint1);
	} else if (ABS(size1) <= 1 && ABS(size2) <= 1) {
		return yadsl_bigint_from_int((intmax_t)(DIGITVALUE(bigint1) + DIGITVALUE(bigint2)));
	} else {
		return NULL;
	}
}

yadsl_BigIntHandle*
yadsl_bigint_subtract(
	yadsl_BigIntHandle* bigint1,
	yadsl_BigIntHandle* bigint2)
{
	return NULL;
}

yadsl_BigIntHandle*
yadsl_bigint_multiply(
	yadsl_BigIntHandle* bigint1,
	yadsl_BigIntHandle* bigint2)
{
	return NULL;
}

yadsl_BigIntHandle*
yadsl_bigint_divide(
	yadsl_BigIntHandle* bigint1,
	yadsl_BigIntHandle* bigint2)
{
	return NULL;
}

int
yadsl_bigint_compare(
	yadsl_BigIntHandle* bigint1,
	yadsl_BigIntHandle* bigint2)
{
	return 0;
}

char*
yadsl_bigint_to_string(
	yadsl_BigIntHandle* bigint)
{
	return NULL;
}

void
yadsl_bigint_destroy(
	yadsl_BigIntHandle* bigint)
{
	free(bigint);
}
