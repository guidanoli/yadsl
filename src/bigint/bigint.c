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
#define SIGN ((digit)1 << SHIFT)
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
 * The size field sign bit represents the
 * sign of the big integer itself.
 *
 * 0XXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX
 *
 * Invariants:
 *
 * ABS(size) >= 0
 * SIGN & digits[i] == 0, for i in [0, ABS(size)]
*/
typedef struct
{
	intptr_t size;
	digit digits[1];
}
BigInt;

static int getndigits(intmax_t i)
{
	uintmax_t u = (uintmax_t)ABS(i);
	int ndigits = 0;
	while (u != 0) {
		u >>= SHIFT;
		++ndigits;
	}
	return ndigits;
}

void
yadsl_bigint_dump(yadsl_BigIntHandle* _bigint)
{
	BigInt* bigint = (BigInt*) _bigint;
	intptr_t size = SIZE(bigint);
	intptr_t ndigits = ABS(size);
	fprintf(stderr, "isnegative = %s\n", size < 0 ? "true" : "false");
	fprintf(stderr, "ndigits = %zd\n", ndigits);
	for (intptr_t i = 0; i < ndigits; ++i)
		fprintf(stderr, "digit #%zd = %" PRIu32 "\n", i, bigint->digits[i]);
}

static BigInt*
bigint_new(intptr_t size)
{
	BigInt* bigint;
	intptr_t ndigits = ABS(size);
	if (ndigits < 0) return NULL;
	bigint = malloc(sizeof(*bigint)+(ndigits-1)*sizeof(digit));
	if (bigint != NULL) SIZE(bigint) = size;
	return bigint;
}

yadsl_BigIntHandle*
yadsl_bigint_from_int(intmax_t i)
{
	int ndigits = getndigits(i);
	int size = i > 0 ? ndigits : -ndigits;
	BigInt* bigint = bigint_new((intptr_t) size);
	if (bigint != NULL) {
		if (ABS(i) < 0) {
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
	BigInt* copy = yadsl_bigint_copy(_bigint);
	if (copy != NULL) SIZE(copy) *= -1;
	return copy;
}

static BigInt*
digitadd(digit* a, intptr_t na, digit* b, intptr_t nb)
{
	BigInt* bigint;
	digit* c, da, db, dc, carry = 0;
	intptr_t n = na > nb ? na : nb;
	intptr_t m = 0;
	for (intptr_t i = 0; i < n; ++i) {
		da = i < na ? a[i] : 0;
		db = i < nb ? b[i] : 0;
		dc = da + db + carry;
		carry = (dc & SIGN) >> SHIFT;
		if (dc != 0) m = i+1;
	}
	if (carry) ++m;
	bigint = bigint_new(m);
	if (bigint == NULL) return NULL;
	c = bigint->digits;
	carry = 0;
	for (intptr_t i = 0; i < m; ++i) {
		da = i < na ? a[i] : 0;
		db = i < nb ? b[i] : 0;
		dc = da + db + carry;
		carry = (dc & SIGN) >> SHIFT;
		c[i] = dc & ~SIGN;
	}
	return bigint;
}

static BigInt*
digitaddneg(digit* a, intptr_t na, digit* b, intptr_t nb)
{
	BigInt* bigint = digitadd(a, na, b, nb);
	if (bigint != NULL) SIZE(bigint) *= -1;
	return bigint;
}

static int
digitcmp(digit* a, intptr_t na, digit* b, intptr_t nb)
{
	if (na > nb)
		return 1;
	else if (na < nb)
		return -1;
	else if (na == 0)
		return 0;
	else {
		int cmp;
		int sign = na < 0 ? -1 : 1;
		for (intptr_t i = ABS(na)-1; i >= 0; --i) {
			cmp = (a[i] > b[i]) - (b[i] > a[i]);
			if (cmp != 0) break;
		}
		return cmp * sign;
	}
}

/* digitcmp(a, na, b, nb) >= 0 */
static BigInt*
digitstrictsub(digit* a, intptr_t na, digit* b, intptr_t nb)
{
	BigInt* bigint;
	digit* c, da, db, dc, carry = 0;
	intptr_t n = na > nb ? na : nb, m = 0;
	for (intptr_t i = 0; i < n; ++i) {
		da = i < na ? a[i] : 0;
		db = i < nb ? b[i] : 0;
		dc = (da | SIGN) - db - carry;
		carry = ~(dc & SIGN) >> SHIFT;
		if (dc & ~SIGN != 0) m = i+1;
	}
	bigint = bigint_new(m);
	if (bigint == NULL) return NULL;
	c = bigint->digits;
	carry = 0;
	for (intptr_t i = 0; i < m; ++i) {
		da = i < na ? a[i] : 0;
		db = i < nb ? b[i] : 0;
		dc = (da | SIGN) - db - carry;
		carry = ~(dc & SIGN) >> SHIFT;
		c[i] = dc & ~SIGN;
	}
	return bigint;
}

static BigInt*
digitsub(digit* a, intptr_t na, digit* b, intptr_t nb)
{
	if (digitcmp(a, na, b, nb) >= 0)
		return digitstrictsub(a, na, b, nb);
	else {
		BigInt* bigint = digitstrictsub(b, nb, a, na);
		if (bigint != NULL) SIZE(bigint) *= -1;
		return bigint;
	}
}

yadsl_BigIntHandle*
yadsl_bigint_add(
	yadsl_BigIntHandle* _a,
	yadsl_BigIntHandle* _b)
{
	BigInt* a = (BigInt*) _a, * b = (BigInt*) _b;
	intptr_t na = SIZE(a), nb = SIZE(b);
	if (na == 0) {
		return yadsl_bigint_copy(b);
	} else if (nb == 0) {
		return yadsl_bigint_copy(a);
	} else if (ABS(na) <= 1 && ABS(nb) <= 1) {
		return yadsl_bigint_from_int((intmax_t)DIGITVALUE(a) + (intmax_t)DIGITVALUE(b));
	} else if (na > 0 && nb > 0) {
		return digitadd(a->digits, na, b->digits, nb);
	} else if (na < 0 && nb < 0) {
		return digitaddneg(a->digits, -na, b->digits, -nb);
	} else if (na > 0 && nb < 0) {
		return digitsub(a->digits, na, b->digits, -nb);
	} else { /* na < 0 && nb > 0 */
		return digitsub(b->digits, nb, a->digits, -na);
	}
}

yadsl_BigIntHandle*
yadsl_bigint_subtract(
	yadsl_BigIntHandle* _a,
	yadsl_BigIntHandle* _b)
{
	BigInt* a = (BigInt*) _a, * b = (BigInt*) _b;
	intptr_t na = SIZE(a), nb = SIZE(b);
	if (na == 0) {
		return yadsl_bigint_opposite(b);
	} else if (nb == 0) {
		return yadsl_bigint_copy(a);
	} else if (ABS(na) <= 1 && ABS(nb) <= 1) {
		return yadsl_bigint_from_int((intmax_t)DIGITVALUE(a) - (intmax_t)DIGITVALUE(b));
	} else if (na > 0 && nb > 0) {
		return digitsub(a->digits, na, b->digits, nb);
	} else if (na < 0 && nb < 0) {
		return digitsub(b->digits, -nb, a->digits, -na);
	} else if (na > 0 && nb < 0) {
		return digitadd(a->digits, na, b->digits, -nb);
	} else { /* na < 0 && nb > 0 */
		return digitaddneg(a->digits, -na, b->digits, nb);
	}
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
	yadsl_BigIntHandle* _a,
	yadsl_BigIntHandle* _b)
{
	BigInt* a = (BigInt*) _a, * b = (BigInt*) _b;
	intptr_t na = SIZE(a), nb = SIZE(b);
	return digitcmp(a->digits, na, b->digits, nb);
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
