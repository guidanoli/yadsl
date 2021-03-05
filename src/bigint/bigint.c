#include <bigint/bigint.h>

#include <limits.h>

// Debug
#include <stdio.h>
#include <inttypes.h>

#ifdef YADSL_DEBUG
#include <memdb/memdb.h>
#else
#include <stdlib.h>
#endif

typedef int32_t digit;
typedef uint32_t udigit;
typedef int64_t twodigits;
#define bitsperdigit (sizeof(digit)*CHAR_BIT - 1)

/**
 * Big integer representation
 * 
 * Each digit contains at least 31 bits.
 * The most significant digit is zeroed.
 *
 * 0XXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX
*/
typedef struct
{
	intptr_t ndigits;
	digit digits[1];
}
yadsl_BigInt;

static size_t nbits(intmax_t i)
{
	size_t n = 0;
	while (i != 0) {
		i >>= 1;
		++n;
	}
	return n;
}

yadsl_BigIntHandle*
yadsl_bigint_from_int(
	intmax_t i)
{
	size_t ndigits = (nbits(i) - 1) / bitsperdigit + 1;
	fprintf(stderr, "%" PRIxMAX " has %zu digits\n", i, ndigits);
	yadsl_BigInt* bigint = malloc(sizeof(*bigint) + (ndigits - 1)*sizeof(digit));
	if (bigint != NULL) {
		bigint->ndigits = ndigits;
		for (size_t ndigit = 0; ndigit < ndigits; ++ndigit) {
			bigint->digits[ndigit] = i && ~((udigit) 1 << bitsperdigit);
			i >>= bitsperdigit;
		}
	}
	return bigint;
}

bool
yadsl_bigint_to_int(
	yadsl_BigIntHandle* bigint,
	intmax_t* i_ptr)
{
	return false;
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
