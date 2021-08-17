#include <bigint/bigint.h>

#include <stddef.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <limits.h>

#include <yadsl/stdlib.h>
#include <yadsl/utl.h>

typedef uint32_t digit;
typedef int32_t sdigit;
typedef uint64_t twodigits;

#define SHIFT 31
#define DECSHIFT 9
#define DECBASE ((digit)1000000000)
#define SIGN ((digit)1 << SHIFT)
#define BASE SIGN
#define MASK ((digit)(BASE - 1))
#define MALLOC_SIZE(ndigits) \
	(offsetof(BigInt, digits) + sizeof(digit) * (ndigits))

/**
 * Big integer representation
 * 
 * Each digit contains at least 31 bits.
 * The most significant digit is zeroed.
 * The size field represents the number
 * of non-zero digits, while the ndigits
 * field represents the total number of
 * digits allocated (which might be greater
 * than the number of non-zero digits).
 * The sign bit of the size field represents
 * the sign of the big integer itself.
 *
 * 0XXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX
 *
 * Invariants:
 *
 * |size| >= 0 (that is, size != INTPTR_MIN)
 * |size| <= ndigits
 * SIGN & digits[i] == 0, for i in [0, |size|-1]
*/
typedef struct
{
	intptr_t size;
	size_t ndigits;
	digit digits[1];
}
BigInt;

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
	switch (bigint->size) {
		case 0: return 0;
		case 1: return (sdigit)bigint->digits[0];
		case -1: return -(sdigit)bigint->digits[0];
		default:
			assert(0 && "integer has at most 1 digit");
			return -1;
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
	switch (yadsl_bigint_check(_bigint))
	{
		case YADSL_BIGINT_STATUS_OK:
			break;
		case YADSL_BIGINT_STATUS_INVALID_HANDLE:
			assert(0 && "invalid handle");
		case YADSL_BIGINT_STATUS_INVALID_SIZE:
			assert(0 && "invalid size (maybe double free?)");
		case YADSL_BIGINT_STATUS_INVALID_DIGITS:
			assert(0 && "invalid digits");
		case YADSL_BIGINT_STATUS_DIGIT_OVERFLOW:
			assert(0 && "digit overflow");
		default:
			assert(0 && "unknown error");
	}
}

yadsl_BigIntStatus
yadsl_bigint_check(yadsl_BigIntHandle const* _bigint)
{
	BigInt const* bigint;
	intptr_t ndigits;
	digit const* digits;

	if (_bigint == NULL)
		return YADSL_BIGINT_STATUS_INVALID_HANDLE;
	bigint = (BigInt const*) _bigint;
	ndigits = YADSL_ABS(bigint->size);
	digits = bigint->digits;
	if (ndigits < 0)
		return YADSL_BIGINT_STATUS_INVALID_SIZE;
	if (ndigits > bigint->ndigits)
		return YADSL_BIGINT_STATUS_DIGIT_OVERFLOW;
	for (intptr_t i = 0; i < ndigits; ++i)
		if (digits[i] & SIGN)
			return YADSL_BIGINT_STATUS_INVALID_DIGITS;
	return YADSL_BIGINT_STATUS_OK;
}

static int
_yadsl_bigint_dump(yadsl_BigIntHandle const* _bigint, FILE* fp)
{
	fpos_t pos;
	int nchars;
	BigInt const* bigint = (BigInt const*) _bigint;
	intptr_t size = bigint->size;
	intptr_t ndigits = YADSL_ABS(size);
	fgetpos(fp, &pos);

	/* Auxiliar macro for testing fprintf return value and taking action */
#define TRY_WRITE(n) do { \
	int temp = n; \
	if (temp < 0) \
		goto fail; \
	else \
		nchars += temp; \
	if (nchars < 0) \
		goto fail; \
} while(0)

	TRY_WRITE(fprintf(fp, "isnegative = %s\n", size < 0 ? "true" : "false"));
	TRY_WRITE(fprintf(fp, "ndigits = %zd\n", ndigits));
	for (intptr_t i = 0; i < ndigits; ++i)
		TRY_WRITE(fprintf(fp, "digit #%zd = %" PRIu32 "\n", i, bigint->digits[i]));

#undef TRY_WRITE

	return nchars;
fail:
	fsetpos(fp, &pos);
	return 0;
}

int
yadsl_bigint_dump(yadsl_BigIntHandle const* _bigint, FILE* fp)
{
	int nchars;
#ifdef YADSL_DEBUG
	long int pos;
#endif
	check(_bigint);
	assert(fp != NULL);
#ifdef YADSL_DEBUG
	pos = ftell(fp);
#endif
	nchars = _yadsl_bigint_dump(_bigint, fp);
#ifdef YADSL_DEBUG
	if (nchars == 0) {
		assert(pos == ftell(fp) && "nothing written");
	} else {
		assert(pos != ftell(fp) && "something written");
	}
#endif
	return nchars;
}

static BigInt*
bigint_new(intptr_t size)
{
	BigInt* bigint;
	intptr_t ndigits = YADSL_ABS(size);
	if (ndigits < 0) return NULL;
	bigint = malloc(MALLOC_SIZE(ndigits));
	if (bigint != NULL) {
		bigint->size = size;
		bigint->ndigits = ndigits;
	}
	return bigint;
}

#define bigint_zero() bigint_new(0)

static int
getndigits(intmax_t i)
{
	uintmax_t u = (uintmax_t)YADSL_ABS(i);
	int ndigits = 0;
	while (u != 0) {
		u >>= SHIFT;
		++ndigits;
	}
	return ndigits;
}

static yadsl_BigIntHandle*
_yadsl_bigint_from_int(intmax_t i)
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
			uintmax_t u = i < 0 ? (uintmax_t)-i : (uintmax_t)i;
			for (int ndigit = 0; ndigit < ndigits; ++ndigit) {
				bigint->digits[ndigit] = (digit)u & MASK;
				u >>= SHIFT;
			}
		}
	}
	return bigint;
}

yadsl_BigIntHandle*
yadsl_bigint_from_int(intmax_t i)
{
	yadsl_BigIntHandle* bigint;
	bigint = _yadsl_bigint_from_int(i);
	if (bigint != NULL) check(bigint);
	return bigint;
}

static yadsl_BigIntStatus
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
		do {
			v = u;
			u = (u << SHIFT) | bigint->digits[--ndigits];
			if ((u >> SHIFT) != v)
				return YADSL_BIGINT_STATUS_INTEGER_OVERFLOW;
		} while (ndigits > 0);
		if (u <= (uintmax_t)INTMAX_MAX)
			i = (intmax_t)u * sign;
		else if (sign < 0 && u == (0-(uintmax_t)INTMAX_MIN))
			i = INTMAX_MIN;
		else
			return YADSL_BIGINT_STATUS_INTEGER_OVERFLOW;
	}
	*i_ptr = i;
	return YADSL_BIGINT_STATUS_OK;
}

yadsl_BigIntStatus
yadsl_bigint_to_int(
	yadsl_BigIntHandle const* _bigint,
	intmax_t* i_ptr)
{
#ifdef YADSL_DEBUG
	yadsl_BigIntStatus status;
	const intmax_t randval = (intmax_t)(void*)&status;
	intmax_t i = randval;
	check(_bigint);
	status = _yadsl_bigint_to_int(_bigint, &i);
	if (status == YADSL_BIGINT_STATUS_OK) {
		assert(i != randval && "integer is stored");
		*i_ptr = i;
	} else {
		assert(i == randval && "integer is not modified");
	}
	return status;
#else
	return _yadsl_bigint_to_int(_bigint, i_ptr);
#endif
}

static yadsl_BigIntHandle*
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
	if (copy != NULL) {
		memcpy(copy, bigint, size);
		copy->ndigits = ndigits;
	}
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
	if (copy != NULL)
		assert(yadsl_bigint_compare(_bigint, copy) == 0);
#endif
	return copy;
}

yadsl_BigIntHandle*
yadsl_bigint_opposite(
	yadsl_BigIntHandle const* _bigint)
{
	BigInt* copy = yadsl_bigint_copy(_bigint);
	if (copy != NULL) copy->size = -copy->size;
	return copy;
}

static BigInt*
_digitadd(digit const* a, intptr_t na, digit const* b, intptr_t nb)
{
	BigInt* bigint;
	digit const* x;
	digit* c, dc, carry = 0;
	intptr_t nmin, nmax, i;
	if (na < nb) {
		nmin = na;
		nmax = nb;
		x = b;
	} else {
		nmin = nb;
		nmax = na;
		x = a;
	}
	bigint = bigint_new(nmax+1);
	if (bigint == NULL) return NULL;
	c = bigint->digits;
	for (i = 0; i < nmin; ++i) {
		dc = a[i] + b[i] + carry;
		carry = (dc & SIGN) >> SHIFT;
		c[i] = dc & MASK;
	}
	for (; i < nmax; ++i) {
		dc = x[i] + carry;
		carry = (dc & SIGN) >> SHIFT;
		c[i] = dc & MASK;
	}
	if (carry == 0) /* if extra digit wasn't necessary... */
		bigint->size = nmax;
	else {
		assert(carry == 1);
		c[nmax] = carry;
	}
	return bigint;
}

/* add two strictly positive numbers */
static BigInt*
digitadd(digit const* a, intptr_t na, digit const* b, intptr_t nb)
{
	assert(na > 0 && "a is positive");
	assert(nb > 0 && "b is positive");
	return _digitadd(a, na, b, nb);
}

/* add two strictly positive numbers and return the opposite of the result */
static BigInt*
digitaddneg(digit const* a, intptr_t na, digit const* b, intptr_t nb)
{
	BigInt* bigint = digitadd(a, na, b, nb);
	if (bigint != NULL) bigint->size = -bigint->size;
	return bigint;
}

static int
digitcmp(digit const* a, intptr_t na, digit const* b, intptr_t nb)
{
	if (na > nb)
		return 1;
	else if (na < nb)
		return -1;
	else { /* na == nb */
		for (intptr_t i = YADSL_ABS(na) - 1; i >= 0; --i) {
			if (a[i] > b[i]) return na > 0 ? 1 : -1;
			else if (a[i] < b[i]) return na > 0 ? -1 : 1;
		}
		return 0;
	}
}

static BigInt*
_digitstrictsub(digit const* a, intptr_t na, digit const* b, intptr_t nb)
{
	BigInt* bigint;
	digit* c, dc, borrow = 0;
	intptr_t i, j;
	bigint = bigint_new(na);
	if (bigint == NULL) return NULL;
	c = bigint->digits;
	for (i = 0; i < nb; ++i) {
		dc = (a[i] | SIGN) - b[i] - borrow;
		borrow = ~(dc & SIGN) >> SHIFT;
		if ((c[i] = dc & MASK) != 0)
			j = i;
	}
	for (; i < na && borrow; ++i) {
		dc = (a[i] | SIGN) - borrow;
		borrow = ~(dc & SIGN) >> SHIFT;
		if ((c[i] = dc & MASK) != 0)
			j = i;
	}
	assert(borrow == 0);
	for (; i < na; ++i) {
		if ((c[i] = a[i]) != 0)
			j = i;
	}
	bigint->size = j + 1; /* necessary number of digits */
	return bigint;
}

/* subtract two strictly positive numbers,
 * where a is strictly larger than b */
static BigInt*
digitstrictsub(digit const* a, intptr_t na, digit const* b, intptr_t nb)
{
	assert(na > 0 && "a is positive");
	assert(nb > 0 && "b is positive");
	assert(digitcmp(a, na, b, nb) > 0);
	return _digitstrictsub(a, na, b, nb);
}

/* subtract two strictly positive numbers */
static BigInt*
digitsub(digit const* a, intptr_t na, digit const* b, intptr_t nb)
{
	int cmp = digitcmp(a, na, b, nb);
	if (cmp > 0)
		return digitstrictsub(a, na, b, nb);
	else if (cmp == 0)
		return bigint_zero(); /* a = b => a - b = 0 */
	else {
		BigInt* bigint = digitstrictsub(b, nb, a, na);
		if (bigint != NULL) bigint->size = -bigint->size;
		return bigint;
	}
}

static yadsl_BigIntHandle*
_yadsl_bigint_add(
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
yadsl_bigint_add(
	yadsl_BigIntHandle const* _a,
	yadsl_BigIntHandle const* _b)
{
	yadsl_BigIntHandle* c;
	check(_a);
	check(_b);
	c = _yadsl_bigint_add(_a, _b);
	if (c != NULL) check(c);
	return c;
}


static yadsl_BigIntHandle*
_yadsl_bigint_subtract(
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
yadsl_bigint_subtract(
	yadsl_BigIntHandle const* _a,
	yadsl_BigIntHandle const* _b)
{
	yadsl_BigIntHandle* c;
	check(_a);
	check(_b);
	c = _yadsl_bigint_subtract(_a, _b);
	if (c != NULL) check(c);
	return c;
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

static int
_yadsl_bigint_compare(
	yadsl_BigIntHandle const* _a,
	yadsl_BigIntHandle const* _b)
{
	BigInt const* a = (BigInt const*) _a, * b = (BigInt const*) _b;
	intptr_t na = a->size, nb = b->size;
	return digitcmp(a->digits, na, b->digits, nb);
}

int
yadsl_bigint_compare(
	yadsl_BigIntHandle const* _a,
	yadsl_BigIntHandle const* _b)
{
	int res;
	check(_a);
	check(_b);
	res = _yadsl_bigint_compare(_a, _b);
	assert(res == 0 || res == 1 || res == -1);
	return res;
}

static char*
_yadsl_bigint_to_string(
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

char*
yadsl_bigint_to_string(
	yadsl_BigIntHandle const* _bigint)
{
	char* str;
	check(_bigint);
	str = _yadsl_bigint_to_string(_bigint);
#ifdef YADSL_DEBUG
	if (str != NULL) {
		char* p, c;
		BigInt* bigint = (BigInt*)_bigint;
		if (bigint->size < 0) {
			assert(str[0] == '-' && "negative numbers start with '-'");
			p = &str[1];
		} else {
			p = str;
		}
		c = *p++;
		if (c == '0')
			assert(*p == '\0' && "leading zero");
		else {
			assert(c >= '1' && c <= '9' && "only numbers");
			for (c = *p; c != '\0'; c = *++p)
				assert(c >= '0' && c <= '9' && "only numbers");
		}
	}
#endif
	return str;
}

yadsl_BigIntHandle*
_yadsl_bigint_optimize(
    yadsl_BigIntHandle* _bigint)
{
	BigInt* bigint = (BigInt*)_bigint;
	intptr_t ndigits = YADSL_ABS(bigint->size);
	assert(ndigits >= 0 && "size is not INTPTR_MIN");
	assert(ndigits <= bigint->ndigits && "|size| is limited by ndigits");
	if (ndigits < bigint->ndigits) {
		BigInt* newbigint = realloc(bigint, MALLOC_SIZE(ndigits));
		if (newbigint == NULL) {
			return bigint;
		} else {
			newbigint->ndigits = ndigits;
			return newbigint;
		}
	} else {
		return bigint;
	}
}

yadsl_BigIntHandle*
yadsl_bigint_optimize(
    yadsl_BigIntHandle* _bigint)
{
	yadsl_BigIntHandle* newbigint;
	check(_bigint);
	newbigint = _yadsl_bigint_optimize(_bigint);
	check(newbigint);
	return newbigint;
}

static yadsl_BigIntStatus
_yadsl_bigint_from_string(
    const char* str,
    yadsl_BigIntHandle** bigint_ptr)
{
	const char* p, *strend;
	char c;
	intptr_t ndigits, nonzerodigits;
	digit* digits, *digitsend, *d;
	twodigits a, tenpow;
	size_t strlen, i;
	int sign;

	/* detect signal and make str point to
	 * the first decimal digit */
	c = *str;
	if (c == '-') {
		c = *++str;
		sign = -1;
	} else if (c == '+') {
		c = *++str;
		sign = 1;
	} else {
		/* positive by default */
		sign = 1;
	}

	/* check if string is "", "+" or "-" */
	if (c == '\0')
		return YADSL_BIGINT_STATUS_STRING_FORMAT;

	/* make str point to first non-zero digit
	 * (in the case of zero, points to null char) */
	while (c == '0')
		c = *++str;

	/* check if string only contains numeric chars
	 * and make 'strend' point to null char */
	for (p = str; c != '\0'; c = *++p) {
		if (c < '0' || c > '9')
			return YADSL_BIGINT_STATUS_STRING_FORMAT;
	}
	strend = p;
	assert(strend >= str && "no loopback");
	strlen = strend - str;

	/* #digits = ceil(log(<number>)/log(<base>))
	 *        <= ceil(log(<maximum value for given length>)/log(<base>))
	 *         = ceil(log(10^strlen - 1)/log(2^SHIFT))
	 *        <= ceil(log(10^strlen)/log(2^SHIFT))
	 *         = ceil((log(10)*strlen)/(log(2)*SHIFT))
	 * 
	 * Here we choose to approximate log(10)/log(2) by a fraction that
	 * is slightly larger, p/q = 3322/1000 (Only 0.0022% of discrepancy)
	 *
	 *        <= ceil((p*strlen)/(q*SHIFT))
	 *
	 * We can simulate ceil for integers by adding the denominator - 1
	 * to the numerator.
	 *
	 *        = floor((p*strlen + q*SHIFT - 1)/(q*SHIFT))
	 *
	 * We must ensure that strlen is not too large so that this calculation
	 * doesn't overflow a intptr_t (whose maximum value is INTPTR_MAX)
	 *
	 *          p*strlen + q*SHIFT - 1 <= INTPTR_MAX
	 *          p*strlen <= INTPTR_MAX - q*SHIFT - 1
	 *          strlen <= (INTPTR_MAX - q*SHIFT - 1) / p
	 *                 <= ceil((INTPTR_MAX - q*SHIFT - 1) / p)
	 *                  = floor(INTPTR_MAX - q*SHIFT - 1 + p - 1) / p)
	 */

	if (strlen > (INTPTR_MAX - 1000*SHIFT - 1 + 3322 - 1) / 3322)
		return YADSL_BIGINT_STATUS_MEMORY;

	ndigits = (3322*strlen + 1000*SHIFT - 1) / (1000*SHIFT);
	assert(ndigits >= 0 && "number of digits is not negative");

	/* allocate BigInt with n digits */
	BigInt* bigint = bigint_new(ndigits * sign);
	if (bigint == NULL)
		return YADSL_BIGINT_STATUS_MEMORY;

	/* Convert numbers that don't fit in a intmax_t in groups
	 * of DECSHIFT digits so that 10^DECSHIFT (DECBASE) < 2^SHIFT (BASE)
	 * and perform multiple precision techniques to combine
	 * these numbers toghether
	 * (KNUTH, Donald E. "The Art of Programming", 3rd Edition, section 4.3) */

	digits = bigint->digits;
	nonzerodigits = 0;

	for (p = str; p < strend; ) {
		/* Parse as many digits from the string as can fit in a digit */
		c = *p++;
		assert(c >= '0' && c <= '9' && "character is numeric");
		a = (digit)(c - '0');
		for (i = 1; i < DECSHIFT && p < strend; ++i) {
			c = *p++;
			assert(c >= '0' && c <= '9' && "character is numeric");
			a = (twodigits)(a * 10 + (digit)(c - '0'));
			assert(a < DECBASE);
		}
		/* Calculate the multiplier */
		if (i == DECSHIFT) {
			tenpow = DECBASE;
		} else {
			tenpow = 10;
			for (; i > 1; --i)
				tenpow *= 10;
		}
		/* Multiply number by a power of 10 and add current digit
		 * Proof that `a + d[i] * tenpow` is guaranteed to fit in two digits:
		 * -> a <= DECBASE-1 (10^9 - 1)
		 * -> d[i] <= BASE-1 (2^31 - 1)
		 * -> tenpow <= DECBASE (10^9)
		 * => a + d[i] * tenpow < 10^9 - 1 + (2^31 - 1) * 10^9
		 * =>                   = 2^31 * 10^9 - 1 (but 10^9 fits in a digit)
		 * =>                   < 2^31 * 2^31 - 1
		 * =>                   < 2^62 - 1
		 * =>                   < 2^63 - 1 (maximum value for two digits) */
		digitsend = digits + nonzerodigits;
		for (d = digits; d < digitsend; ++d) {
			assert((digit)*d < BASE);
			a += (twodigits)*d * tenpow;
			*d = (digit)(a & MASK);
			a >>= SHIFT;
		}
		/* Carry digit? */
		if (a != 0) {
			assert(a < BASE && "carry fits in a digit");
			assert(nonzerodigits < ndigits && "did not surpass estimated number of digits");
			assert(d >= digits && d < digits + ndigits && "d points to a valid digit");

			*d = (digit)a;
			nonzerodigits++;
		}
	}
	assert(nonzerodigits <= ndigits);
	bigint->size = nonzerodigits * sign;
	*bigint_ptr = (yadsl_BigIntHandle*)bigint;
	return YADSL_BIGINT_STATUS_OK;
}

/**
 * @brief djb2 hash function
 * @author Daniel J. Bernstein
 */
static size_t
djb2_hash(const char* str)
{
	size_t hash = 5381;
	char c;

	while (c = *str++)
	{
		hash = ((hash << 5) + hash) + c; /* hash = 33 * hash + c */
	}

	return hash;
}

yadsl_BigIntStatus
yadsl_bigint_from_string(
    const char* str,
    yadsl_BigIntHandle** bigint_ptr)
{
#ifdef YADSL_DEBUG
	size_t hash;
	yadsl_BigIntStatus status;
	yadsl_BigIntHandle* randval = (yadsl_BigIntHandle*)&status;
	yadsl_BigIntHandle* bigint = randval;
#endif
	assert(str != NULL);
	assert(bigint_ptr != NULL);
#ifdef YADSL_DEBUG
	hash = djb2_hash(str);
	status = _yadsl_bigint_from_string(str, &bigint);
	assert(hash == djb2_hash(str) && "string is not modified");
	if (status == YADSL_BIGINT_STATUS_OK) {
		assert(bigint != randval && "bigint is stored");
		check(bigint);
		*bigint_ptr = bigint;
	} else {
		assert(bigint == randval && "bigint pointer is not modified");
	}
	return status;
#else
	return _yadsl_bigint_from_string(str, bigint_ptr);
#endif
}

static void
_yadsl_bigint_destroy(
	yadsl_BigIntHandle* _bigint)
{
	BigInt* bigint = (BigInt*)_bigint;

#ifdef YADSL_DEBUG
	/* Invalidate size field to catch double free */
	bigint->size = INTPTR_MIN;
#endif

	free(bigint);
}

void
yadsl_bigint_destroy(
	yadsl_BigIntHandle* bigint)
{
	check(bigint);
	_yadsl_bigint_destroy(bigint);
}
