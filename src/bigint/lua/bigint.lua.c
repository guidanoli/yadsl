#include "lauxlib.h"

#include <assert.h>

#include <yadsl/dll.h>

#include <bigint/bigint.h>
#include <memdb/lua/memdb.h>

#include <yadsl/stdlib.h>

#define BIGINT "BigInt"

typedef struct
{
	yadsl_BigIntHandle* bigint;
}
bigint_udata;

/* Module functions */

static void push_bigint(lua_State* L, yadsl_BigIntHandle* bigint)
{
	bigint_udata* udata;

	if (bigint == NULL)
		luaL_error(L, "bad malloc");

	udata = lua_newuserdata(L, sizeof(bigint_udata));
	udata->bigint = bigint;
	luaL_setmetatable(L, BIGINT);
}

static int bigint_new(lua_State* L)
{
	yadsl_BigIntHandle* bigint = NULL;

	switch (lua_type(L, 1)) {
		case LUA_TNUMBER:
		{
			lua_Integer integer = luaL_checkinteger(L, 1);
			if (integer < INTMAX_MIN || integer > INTMAX_MAX)
				return luaL_error(L, "integer overflow");
			bigint = yadsl_bigint_from_int((intmax_t)integer);
			break;
		}
		case LUA_TSTRING:
		{
			size_t size;
			const char* str = lua_tolstring(L, 1, &size);
			yadsl_BigIntStatus status;
			for (size_t i = 0; i < size; ++i)
				if (str[i] == '\0')
					return luaL_argerror(L, 1, "string contains embedded zeros");
			status = yadsl_bigint_from_string(str, &bigint);
			switch (status) {
				case YADSL_BIGINT_STATUS_OK:
					break;
				case YADSL_BIGINT_STATUS_STRING_FORMAT:
					return luaL_argerror(L, 1, "string is ill-formatted");
				case YADSL_BIGINT_STATUS_MEMORY:
					return luaL_error(L, "bad malloc");
				default:
					assert(0 && "Unknown return value");
					return luaL_error(L, "unknown error");
			}
			break;
		}
		default:
			return luaL_argerror(L, 1, "expected string or number");
	}

	push_bigint(L, bigint);
	return 1;
}

static const struct luaL_Reg bigint_lib[] = {
	{BIGINT, bigint_new},
	{NULL, NULL}  /* sentinel */
};

/* Auxiliary functions for working with BigInt */

static bigint_udata* check_bigint_udata(lua_State* L, int arg)
{
	bigint_udata* udata;
	udata = (bigint_udata*)luaL_checkudata(L, arg, BIGINT);
	assert(udata != NULL && "Userdata is valid");
	return udata;
}

static yadsl_BigIntHandle* check_bigint(lua_State* L, int arg)
{
	bigint_udata* udata;
	yadsl_BigIntStatus status;
	udata = check_bigint_udata(L, arg);
#ifdef YADSL_DEBUG
	switch (status = yadsl_bigint_check(udata->bigint)) {
		case YADSL_BIGINT_STATUS_OK:
			break;
		case YADSL_BIGINT_STATUS_INVALID_HANDLE:
			luaL_error(L, "invalid handle");
		case YADSL_BIGINT_STATUS_INVALID_SIZE:
			luaL_error(L, "invalid size (maybe double free?)");
		case YADSL_BIGINT_STATUS_INVALID_DIGITS:
			luaL_error(L, "invalid digits");
		default:
			luaL_error(L, "unknown error %d", status);
	}
#endif
	return udata->bigint;
}

/* BigInt metamethods */

static int bigint_tostring(lua_State* L)
{
	yadsl_BigIntHandle* bigint = check_bigint(L, 1);
	char* str = yadsl_bigint_to_string(bigint);
	if (str == NULL)
		return luaL_error(L, "bad malloc");
	lua_pushstring(L, str);
	free(str);
	return 1;
}

static int bigint_gc(lua_State* L)
{
	bigint_udata* udata = check_bigint_udata(L, 1);
	yadsl_BigIntHandle* bigint = udata->bigint;
	if (bigint != NULL) {
		udata->bigint = NULL;
		yadsl_bigint_destroy(bigint);
	}
	return 0;
}

static int bigint_add(lua_State* L)
{
	yadsl_BigIntHandle* a = check_bigint(L, 1);
	yadsl_BigIntHandle* b = check_bigint(L, 2);
	yadsl_BigIntHandle* c = yadsl_bigint_add(a, b);
	push_bigint(L, c);
	return 1;
}

static int bigint_sub(lua_State* L)
{
	yadsl_BigIntHandle* a = check_bigint(L, 1);
	yadsl_BigIntHandle* b = check_bigint(L, 2);
	yadsl_BigIntHandle* c = yadsl_bigint_subtract(a, b);
	push_bigint(L, c);
	return 1;
}

static int bigint_unm(lua_State* L)
{
	yadsl_BigIntHandle* a = check_bigint(L, 1);
	yadsl_BigIntHandle* b = yadsl_bigint_opposite(a);
	push_bigint(L, b);
	return 1;
}

static int bigint_compare(lua_State* L)
{
	yadsl_BigIntHandle* a = check_bigint(L, 1);
	yadsl_BigIntHandle* b = check_bigint(L, 2);
	return yadsl_bigint_compare(a, b);
}

static int bigint_eq(lua_State* L)
{
	lua_pushboolean(L, bigint_compare(L) == 0);
	return 1;
}

static int bigint_lt(lua_State* L)
{
	lua_pushboolean(L, bigint_compare(L) < 0);
	return 1;
}

static int bigint_le(lua_State* L)
{
	lua_pushboolean(L, bigint_compare(L) <= 0);
	return 1;
}

static const struct luaL_Reg bigint_metamethods[] = {
	{"__tostring", bigint_tostring},
	{"__gc", bigint_gc},
	{"__add", bigint_add},
	{"__sub", bigint_sub},
	{"__unm", bigint_unm},
	{"__eq", bigint_eq},
	{"__lt", bigint_lt},
	{"__le", bigint_le},
	{NULL, NULL}  /* sentinel */
};

/* BigInt methods */

static int bigint_tointeger(lua_State* L)
{
	yadsl_BigIntHandle* bigint = check_bigint(L, 1);
	intmax_t integer;
	yadsl_BigIntStatus status = yadsl_bigint_to_int(bigint, &integer);
	if (status != YADSL_BIGINT_STATUS_OK || integer < LUA_MININTEGER || integer > LUA_MAXINTEGER)
		return luaL_error(L, "integer overflow");
	lua_pushinteger(L, (lua_Integer)integer);
	return 1;
}

static const struct luaL_Reg bigint_methods[] = {
	{"to_integer", bigint_tointeger},
	{NULL, NULL}  /* sentinel */
};

/* Entry point */

YADSL_EXPORT int luaopen_bigint(lua_State* L)
{
	/* register library */
	luaL_newlib(L, bigint_lib);

	/* register memdb submodule */
	yadsl_memdb_openlib(L);
	lua_setfield(L, -2, "memdb");

	/* register BigInt metatable */
	luaL_newlib(L, bigint_metamethods);
	luaL_newlib(L, bigint_methods);
	lua_setfield(L, -2, "__index");
	lua_setfield(L, LUA_REGISTRYINDEX, BIGINT);

	return 1;
}
