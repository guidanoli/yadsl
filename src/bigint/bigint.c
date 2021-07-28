#include <bigint/bigint.h>

#include <stddef.h>
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

#include <yadsl/utl.h>

typedef uint32_t digit;
typedef int32_t sdigit;
typedef uint64_t twodigits;

#define SHIFT 31
#define DECSHIFT 9
#define DECBASE ((digit)1000000000)
#define SIGN ((digit)1 << SHIFT)
#define MALLOC_SIZE(ndigits) \
	(offsetof(BigInt, digits) + sizeof(digit) * (ndigits))

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
 * |size| >= 0 (that is, size != INTPTR_MIN)
 * SIGN & digits[i] == 0, for i in [0, |size|-1]
*/
typedef struct
{
	intptr_t size;
	digit digits[1];
}
BigInt;

static yadsl_BigIntHandle*
_yadsl_bigint_copy(
	yadsl_BigIntHandle const* _bigint);

static int getndigits(intmax_t i)
{
	uintmax_t u = (uintmax_t)YADSL_ABS(i);
	int ndigits = 0;
	while (u != 0) {
		u >>= SHIFT;
		++ndigits;
	}
	return ndigits;
}

/**
 * Gets the signed value of a bigint with at most 1 digit
 * Preconditions:
 *   bigint points to a valid BigInt
 *   bigint->size is -1, 0 or 1
 * Postconditions:
 *   Returns the signed value of bigint
*/
static sdigit getdigitvalue(BigInt const* bigint)
{
	intptr_t size = bigint->size;
	assert(-1 <= size && size <= 1 && "integer has 1 digit");
	switch (size) {
		case 1: return (sdigit)bigint->digits[0];
		case -1: return -(sdigit)bigint->digits[0];
		default: return 0;
	}
}

#ifdef YADSL_DEBUG
#define check(bigint) _check(bigint)
#else
#define check(bigint) ((void) bigint)
#endif

static void
_check(yadsl_BigIntHandle const* _bigint)
{
	assert(_bigint != NULL);
	switch (yadsl_bigint_check(_bigint))
	{
		case YADSL_BIGINT_STATUS_OK:
			break;
		case YADSL_BIGINT_STATUS_INVALID_SIZE:
			assert(0 && "Invalid size");
		case YADSL_BIGINT_STATUS_INVALID_DIGITS:
			assert(0 && "Invalid digits");
		case YADSL_BIGINT_STATUS_LEADING_ZEROS:
			assert(0 && "Leading zeros");
		default:
			assert(0 && "Unknown error");
	}
}

yadsl_BigIntStatus
yadsl_bigint_check(yadsl_BigIntHandle const* _bigint)
{
	BigInt const* bigint = (BigInt const*) _bigint;
	intptr_t ndigits = YADSL_ABS(bigint->size);
	digit const* digits = bigint->digits;
	if (ndigits < 0)
		return YADSL_BIGINT_STATUS_INVALID_SIZE;
	for (intptr_t i = 0; i < ndigits; ++i)
		if (digits[i] & SIGN)
			return YADSL_BIGINT_STATUS_INVALID_DIGITS;
	if (ndigits > 0 && digits[ndigits-1] == 0)
		return YADSL_BIGINT_STATUS_LEADING_ZEROS;
	return YADSL_BIGINT_STATUS_OK;
}

void
_yadsl_bigint_dump(yadsl_BigIntHandle const* _bigint)
{
	BigInt const* bigint = (BigInt const*) _bigint;
	intptr_t size = bigint->size;
	intptr_t ndigits = YADSL_ABS(size);
	fprintf(stderr, "isnegative = %s\n", size < 0 ? "true" : "false");
	fprintf(stderr, "ndigits = %zd\n", ndigits);
	for (intptr_t i = 0; i < ndigits; ++i)
		fprintf(stderr, "digit #%zd = %" PRIu32 "\n", i, bigint->digits[i]);
}

void
yadsl_bigint_dump(yadsl_BigIntHandle const* _bigint)
{
	check(_bigint);
	_yadsl_bigint_dump(_bigint);
}

static BigInt*
bigint_new(intptr_t size)
{
	BigInt* bigint;
	intptr_t ndigits = YADSL_ABS(size);
	if (ndigits < 0) return NULL;
	bigint = malloc(MALLOC_SIZE(ndigits));
	if (bigint != NULL) bigint->size = size;
	return bigint;
}

yadsl_BigIntHandle*
yadsl_bigint_from_int(intmax_t i)
{
	int ndigits = getndigits(i);
	int size = i > 0 ? ndigits : -ndigits;
	BigInt* bigint = bigint_new((intptr_t) size);
	if (bigint != NULL) {
		if (YADSL_ABS(i) < 0) {
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
	check(bigint);
	return bigint;
}

static bool
_yadsl_bigint_to_int(
	yadsl_BigIntHandle const* _bigint,
	intmax_t* i_ptr)
{
	intmax_t i;
	uintmax_t u, v;
	int sign;
	intptr_t ndigits;
	intptr_t size;
	BigInt const* bigint;
	check(_bigint);
	bigint = (BigInt *) _bigint;
	size = bigint->size;
	switch (size) {
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
		ndigits = YADSL_ABS(size);
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

bool
yadsl_bigint_to_int(
	yadsl_BigIntHandle const* _bigint,
	intmax_t* i_ptr)
{
	bool res;
	check(_bigint);
	assert(i_ptr != NULL);
	res = _yadsl_bigint_to_int(_bigint, i_ptr);
#ifdef YADSL_DEBUG
	if (res) {
		yadsl_BigIntHandle* copy = _yadsl_bigint_copy(_bigint);
		if (copy != NULL) {
			assert(yadsl_bigint_compare(_bigint, copy) == 0);
			yadsl_bigint_destroy(copy);
		}
	}
#endif
	return res;
}

yadsl_BigIntHandle*
_yadsl_bigint_copy(
	yadsl_BigIntHandle const* _bigint)
{
	BigInt const* bigint;
	BigInt* copy;
	size_t size;
	intptr_t ndigits;
	bigint = (BigInt const*) _bigint;
	ndigits = YADSL_ABS(bigint->size);
	assert(ndigits >= 0);
	size = MALLOC_SIZE(ndigits);
	copy = malloc(size);
	if (copy != NULL) memcpy(copy, bigint, size);
	return copy;
}

yadsl_BigIntHandle*
yadsl_bigint_copy(
	yadsl_BigIntHandle const* _bigint)
{
	yadsl_BigIntHandle* copy;
	check(_bigint);
	copy = _yadsl_bigint_copy(_bigint);
#ifdef YADSL_DEBUG
	if (copy != NULL) {
		check(copy);
		assert(yadsl_bigint_compare(_bigint, copy) == 0);
	}
#endif
	return copy;
}

yadsl_BigIntHandle*
_yadsl_bigint_opposite(
	yadsl_BigIntHandle const* _bigint)
{
	BigInt* copy = yadsl_bigint_copy(_bigint);
	if (copy != NULL) copy->size *= -1;
	return copy;
}

yadsl_BigIntHandle*
yadsl_bigint_opposite(
	yadsl_BigIntHandle const* _bigint)
{
	yadsl_BigIntHandle* op;
	check(_bigint);
   	op = _yadsl_bigint_opposite(_bigint);
#ifdef YADSL_DEBUG
	if (op != NULL) {
		yadsl_BigIntHandle* bigint2;
		check(op);
		bigint2 = _yadsl_bigint_opposite(op);
		if (bigint2 != NULL) {
			assert(yadsl_bigint_compare(_bigint, bigint2) == 0);
			yadsl_bigint_destroy(bigint2);
		}
	}
#endif
	return op;
}

static BigInt*
digitadd(digit const* a, intptr_t na, digit const* b, intptr_t nb)
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
digitaddneg(digit const* a, intptr_t na, digit const* b, intptr_t nb)
{
	BigInt* bigint = digitadd(a, na, b, nb);
	if (bigint != NULL) bigint->size *= -1;
	return bigint;
}

static int
digitcmp(digit const* a, intptr_t na, digit const* b, intptr_t nb)
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
		for (intptr_t i = YADSL_ABS(na)-1; i >= 0; --i) {
			cmp = (a[i] > b[i]) - (b[i] > a[i]);
			if (cmp != 0) break;
		}
		return cmp * sign;
	}
}

/* digitcmp(a, na, b, nb) >= 0 */
static BigInt*
digitstrictsub(digit const* a, intptr_t na, digit const* b, intptr_t nb)
{
	BigInt* bigint;
	digit* c, da, db, dc, borrow = 0;
	intptr_t n = na > nb ? na : nb, m = 0;
	for (intptr_t i = 0; i < n; ++i) {
		da = i < na ? a[i] : 0;
		db = i < nb ? b[i] : 0;
		dc = (da | SIGN) - db - borrow;
		borrow = ~(dc & SIGN) >> SHIFT;
		if (dc & ~SIGN != 0) m = i+1;
	}
	assert(borrow == 0);
	bigint = bigint_new(m);
	if (bigint == NULL) return NULL;
	c = bigint->digits;
	borrow = 0;
	for (intptr_t i = 0; i < m; ++i) {
		da = i < na ? a[i] : 0;
		db = i < nb ? b[i] : 0;
		dc = (da | SIGN) - db - borrow;
		borrow = ~(dc & SIGN) >> SHIFT;
		c[i] = dc & ~SIGN;
	}
	return bigint;
}

static BigInt*
digitsub(digit const* a, intptr_t na, digit const* b, intptr_t nb)
{
	if (digitcmp(a, na, b, nb) >= 0)
		return digitstrictsub(a, na, b, nb);
	else {
		BigInt* bigint = digitstrictsub(b, nb, a, na);
		if (bigint != NULL) bigint->size *= -1;
		return bigint;
	}
}

yadsl_BigIntHandle*
yadsl_bigint_add(
	yadsl_BigIntHandle const* _a,
	yadsl_BigIntHandle const* _b)
{
	BigInt const* a = (BigInt const*) _a, * b = (BigInt const*) _b;
	intptr_t na = a->size, nb = b->size;
	if (na == 0) {
		return yadsl_bigint_copy(b);
	} else if (nb == 0) {
		return yadsl_bigint_copy(a);
	} else if (YADSL_ABS(na) <= 1 && YADSL_ABS(nb) <= 1) {
		return yadsl_bigint_from_int((intmax_t)getdigitvalue(a) + (intmax_t)getdigitvalue(b));
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
	yadsl_BigIntHandle const* _a,
	yadsl_BigIntHandle const* _b)
{
	BigInt const* a = (BigInt const*) _a, * b = (BigInt const*) _b;
	intptr_t na = a->size, nb = b->size;
	if (na == 0) {
		return yadsl_bigint_opposite(b);
	} else if (nb == 0) {
		return yadsl_bigint_copy(a);
	} else if (YADSL_ABS(na) <= 1 && YADSL_ABS(nb) <= 1) {
		return yadsl_bigint_from_int((intmax_t)getdigitvalue(a) - (intmax_t)getdigitvalue(b));
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
	yadsl_BigIntHandle const* bigint1,
	yadsl_BigIntHandle const* bigint2)
{
	return NULL;
}

yadsl_BigIntHandle*
yadsl_bigint_divide(
	yadsl_BigIntHandle const* bigint1,
	yadsl_BigIntHandle const* bigint2)
{
	return NULL;
}

int
yadsl_bigint_compare(
	yadsl_BigIntHandle const* _a,
	yadsl_BigIntHandle const* _b)
{
	BigInt const* a = (BigInt const*) _a, * b = (BigInt const*) _b;
	intptr_t na = a->size, nb = b->size;
	return digitcmp(a->digits, na, b->digits, nb);
}

char*
yadsl_bigint_to_string(
	yadsl_BigIntHandle const* _bigint)
{
	BigInt const* bigint = (BigInt const*) _bigint;
	digit const* pin;
	digit* pout;
	digit rem, tenpow;
	size_t ndigits, strsize, size, i, j;
	int negative, d;
	char* str = NULL, *p;

	ndigits = (size_t) YADSL_ABS(bigint->size);
	negative = bigint->size < 0;

	d = (33 * DECSHIFT) /
		(10 * SHIFT - 33 * DECSHIFT);
	size = 1 + ndigits + ndigits / d;
	pout = malloc(size * sizeof(digit));

	if (pout == NULL) return NULL;

	pin = bigint->digits;
	size = 0;
	for (i = ndigits; i-- >= 1; ) {
		digit hi = pin[i];
		for (j = 0; j < size; ++j) {
			twodigits z = (twodigits)pout[j] << SHIFT | hi;
			hi = (digit)(z / DECBASE);
			pout[j] = (digit)(z - (twodigits)hi * DECBASE);
		}
		while (hi) {
			pout[size++] = hi % DECBASE;
			hi /= DECBASE;
		}
	}
	
	if (size == 0)
		pout[size++] = 0;

	strsize = negative + 1 + (size - 1) * DECSHIFT;
	tenpow = 10;
	rem = pout[size-1];
	while (rem >= tenpow) {
		tenpow *= 10;
		strsize++;
	}
	
	str = malloc(strsize + 1);
	if (str == NULL)
		goto end;

	p = str + strsize;
	*p = '\0';

	for (i = 0; i < size - 1; i++) {
		rem = pout[i];
		for (j = 0; j < DECSHIFT; j++) {
			*--p = '0' + rem % 10;
			rem /= 10;
		}
	}

	rem = pout[i];
	do {
		*--p = '0' + rem % 10;
		rem /= 10;
	} while (rem != 0);

	if (negative)
		*--p = '-';

	assert(p == str);

end:
	free(pout);
	return str;
}

void
yadsl_bigint_destroy(
	yadsl_BigIntHandle* bigint)
{
	free(bigint);
}
