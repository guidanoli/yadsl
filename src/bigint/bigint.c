#include <bigint/bigint.h>

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

#define shift (sizeof(digit)*CHAR_BIT - 1)

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
		u >>= shift;
		++ndigits;
	}
	return ndigits;
}

static void
yadsl_bigint_dump_internal(BigInt* bigint)
{
	intptr_t size = bigint->size;
	intptr_t ndigits = size < 0 ? -size : size;
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
				bigint->digits[ndigit] = (digit)u & ~(1 << shift);
				u >>= shift;
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
	BigInt* bigint = (BigInt *) _bigint;
	switch (bigint->size) {
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
		if (bigint->size < 0) {
			ndigits = -bigint->size;
			sign = -1;
		} else {
			ndigits = bigint->size;
			sign = 1;
		}
		u = 0;
		while (ndigits > 0) {
			v = u;
			u = (u << shift) | bigint->digits[--ndigits];
			if ((u >> shift) != v) return false;
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
	if (bigint->size < 0) ndigits = -bigint->size;
	else ndigits = bigint->size;
	size = sizeof(*copy)+(ndigits-1)*sizeof(digit);
	copy = malloc(size);
	if (copy != NULL) memcpy(copy, bigint, size);
	return copy;
}

yadsl_BigIntHandle*
yadsl_bigint_opposite(
	yadsl_BigIntHandle* _bigint)
{
	BigInt* bigint = yadsl_bigint_copy(_bigint);
	if (bigint != NULL) bigint->size *= -1;
	return bigint;
}

yadsl_BigIntHandle*
yadsl_bigint_add(
	yadsl_BigIntHandle* bigint1,
	yadsl_BigIntHandle* bigint2)
{
	return NULL;
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
