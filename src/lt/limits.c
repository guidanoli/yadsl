#include <limits.h>
#include <stdint.h>
#include <float.h>
#include <assert.h>

#include "lauxlib.h"

void register_signed(lua_State* L, intmax_t i, const char* name)
{
	char digits[((sizeof(i)*CHAR_BIT-1)*30103+99999)/100000+2];
	char *p = digits + sizeof(digits);
	char sign = i < 0 ? '-' : '+';
	uintmax_t u;

	assert(p-1 >= digits);
	*--p = '\0';

	if (i == INTPTR_MIN)
	{
		/* INTPTR_MIN is -2^K, which is an even
		 * number, so the last digit cannot be 9.
		 * We write the last digit manually. */
		u = (uintptr_t)INTPTR_MAX;
		assert(p-1 >= digits);
		*--p = u % 10 + 1 + '0';
		u /= 10;
	}
	else if (i < 0)
	{
		u = (uintmax_t)-i;
	}
	else
	{
		u = (uintmax_t)i;
	}

	do
	{
		assert(p-1 >= digits);
		*--p = u % 10 + '0';
		u /= 10;
	}
	while (u != 0);

	assert(p-1 >= digits);
	*--p = sign;

	lua_pushlstring(L, p, digits + sizeof(digits) - 1 - p);
	lua_setfield(L, -2, name);
}

void register_unsigned(lua_State* L, uintmax_t u, const char* name)
{
	char digits[(sizeof(u)*CHAR_BIT*30103+99999)/100000+1];
	char *p = digits + sizeof(digits);

	assert(p-1 >= digits);
	*--p = '\0';

	do
	{
		assert(p-1 >= digits);
		*--p = u % 10 + '0';
		u /= 10;
	}
	while (u != 0);

	lua_pushlstring(L, p, digits + sizeof(digits) - 1 - p);
	lua_setfield(L, -2, name);
}


int luaopen_limits(lua_State* L)
{
	lua_createtable(L, 0, 19 + 15 + 9*4 + 29);

#define REGISTER(x) register_signed(L, x, #x)
#define UREGISTER(x) register_unsigned(L, x, #x)

	/* limits.h (19) */

	UREGISTER(CHAR_BIT);
	REGISTER(SCHAR_MIN);
	REGISTER(SCHAR_MAX);
	UREGISTER(UCHAR_MAX);
	REGISTER(CHAR_MIN);
	REGISTER(CHAR_MAX);
	UREGISTER(MB_LEN_MAX);
	REGISTER(SHRT_MIN);
	REGISTER(SHRT_MAX);
	UREGISTER(USHRT_MAX);
	REGISTER(INT_MIN);
	REGISTER(INT_MAX);
	UREGISTER(UINT_MAX);
	REGISTER(LONG_MIN);
	REGISTER(LONG_MAX);
	UREGISTER(ULONG_MAX);
	REGISTER(LLONG_MIN);
	REGISTER(LLONG_MAX);
	UREGISTER(ULLONG_MAX);

	/* stdint.h (15 + 9*4) */

	REGISTER(INTMAX_MIN);
	REGISTER(INTMAX_MAX);
	UREGISTER(UINTMAX_MAX);
	REGISTER(INTPTR_MIN);
	REGISTER(INTPTR_MAX);
	UREGISTER(UINTPTR_MAX);

#define REGISTERN(n) \
	do { \
		REGISTER(INT ## n ## _MIN); \
		REGISTER(INT ## n ## _MAX); \
		UREGISTER(UINT ## n ## _MAX); \
		REGISTER(INT_LEAST ## n ## _MIN); \
		REGISTER(INT_LEAST ## n ## _MAX); \
		UREGISTER(UINT_LEAST ## n ## _MAX); \
		REGISTER(INT_FAST ## n ## _MIN); \
		REGISTER(INT_FAST ## n ## _MAX); \
		UREGISTER(UINT_FAST ## n ## _MAX); \
	} \
	while(0)

	REGISTERN(8);
	REGISTERN(16);
	REGISTERN(32);
	REGISTERN(64);

	UREGISTER(SIZE_MAX);
	REGISTER(PTRDIFF_MIN);
	REGISTER(PTRDIFF_MAX);
	REGISTER(SIG_ATOMIC_MIN);
	REGISTER(SIG_ATOMIC_MAX);
	REGISTER(WCHAR_MIN);
	REGISTER(WCHAR_MAX);
	REGISTER(WINT_MIN);
	REGISTER(WINT_MAX);


	/* float.h (29) */

	UREGISTER(FLT_RADIX);
	UREGISTER(FLT_MANT_DIG);
	UREGISTER(DBL_MANT_DIG);
	UREGISTER(LDBL_MANT_DIG);
	UREGISTER(FLT_DIG);
	UREGISTER(DBL_DIG);
	UREGISTER(LDBL_DIG);
	REGISTER(FLT_MIN_EXP);
	REGISTER(DBL_MIN_EXP);
	REGISTER(LDBL_MIN_EXP);
	REGISTER(FLT_MIN_10_EXP);
	REGISTER(DBL_MIN_10_EXP);
	REGISTER(LDBL_MIN_10_EXP);
	REGISTER(FLT_MAX_EXP);
	REGISTER(DBL_MAX_EXP);
	REGISTER(LDBL_MAX_EXP);
	REGISTER(FLT_MAX_10_EXP);
	REGISTER(DBL_MAX_10_EXP);
	REGISTER(LDBL_MAX_10_EXP);

#define REGISTERF(f) \
	do \
	{ \
		lua_pushnumber(L, f); \
		lua_setfield(L, -2, #f); \
	} \
	while(0)

	REGISTERF(FLT_MAX);
	REGISTERF(DBL_MAX);
	// REGISTERF(LDBL_MAX);
	REGISTERF(FLT_EPSILON);
	REGISTERF(DBL_EPSILON);
	// REGISTERF(LDBL_EPSILON);
	REGISTERF(FLT_MIN);
	REGISTERF(DBL_MIN);
	// REGISTERF(LDBL_MIN);

	REGISTER(FLT_ROUNDS);

#undef REGISTERN
#undef UREGISTER
#undef REGISTER

	return 1;
}
