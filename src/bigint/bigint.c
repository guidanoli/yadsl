#include <bigint/bigint.h>

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
yadsl_BigInt;

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
yadsl_bigint_dump_internal(yadsl_BigInt* bigint)
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
	yadsl_BigInt* bigint = malloc(sizeof(*bigint)+(ndigits-1)*sizeof(digit));
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
	fprintf(stderr, "num = %" PRIdMAX "\n", i);
	yadsl_bigint_dump_internal(bigint);
	return bigint;
}

bool
yadsl_bigint_to_int(
	yadsl_BigIntHandle* _bigint,
	intmax_t* i_ptr)
{
	intmax_t i, iprev;
	int negative;
	intptr_t ndigits;
	yadsl_BigInt* bigint = (yadsl_BigInt *) _bigint;
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
			negative = 1;
		} else {
			ndigits = bigint->size;
			negative = 0;
		}
		i = 0;
		while (ndigits > 0) {
			iprev = i;
			i = (i << shift) | bigint->digits[--ndigits];
			if ((i >> shift) != iprev) return false;
		}
		if (negative) i = -i;
	}
	*i_ptr = i;
	return true;
}

yadsl_BigIntHandle*
yadsl_bigint_copy(
	yadsl_BigIntHandle* bigint)
{
	return NULL;
}

yadsl_BigIntHandle*
yadsl_bigint_opposite(
	yadsl_BigIntHandle* bigint)
{
	return NULL;
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
