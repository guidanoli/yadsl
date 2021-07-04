#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "lauxlib.h"

#define STRFIND "LT_STRING_FIND"

static void lt_pushErrorString(lua_State* L, int op1, int op2, const char* opstr)
{
	int n = 0;
	lua_pushstring(L, "assertion `"); n++;
	luaL_tolstring(L, op1, NULL); n++;
	lua_pushstring(L, " "); n++;
	lua_pushstring(L, opstr); n++;
	lua_pushstring(L, " "); n++;
	luaL_tolstring(L, op2, NULL); n++;
	lua_pushstring(L, "` failed"); n++;
	if (lua_isstring(L, 3))
	{
		lua_pushstring(L, ": "); n++;
		lua_pushvalue(L, 3); n++;
	}
	lua_concat(L, n);
}

static int lt_assertBinop(lua_State* L, int op, bool invert, const char* opstr)
{
	lua_settop(L, 3);
	if (lua_compare(L, 1, 2, op) == invert)
	{
		lt_pushErrorString(L, 1, 2, opstr);
		return lua_error(L);
	}
	else
	{
		return 0;
	}
}

static int lt_assertRawOp(lua_State* L, bool equal, const char* opstr)
{
	lua_settop(L, 3);
	if (lua_rawequal(L, 1, 2) != equal)
	{
		lt_pushErrorString(L, 1, 2, opstr);
		return lua_error(L);
	}
	else
	{
		return 0;
	}
}

static int lt_assertEqual(lua_State* L)
{
	return lt_assertBinop(L, LUA_OPEQ, false, "==");
}

static int lt_assertNotEqual(lua_State* L)
{
	return lt_assertBinop(L, LUA_OPEQ, true, "~=");
}

static int lt_assertGreaterThan(lua_State* L)
{
	return lt_assertBinop(L, LUA_OPLE, true, ">");
}

static int lt_assertGreaterEqual(lua_State* L)
{
	return lt_assertBinop(L, LUA_OPLT, true, ">=");
}

static int lt_assertLessThan(lua_State* L)
{
	return lt_assertBinop(L, LUA_OPLT, false, "<");
}

static int lt_assertLessEqual(lua_State* L)
{
	return lt_assertBinop(L, LUA_OPLE, false, "<=");
}

static int lt_assertNil(lua_State* L)
{
	lua_settop(L, 3);  /* obj msg nil */
	lua_insert(L, -2); /* obj nil msg */
	return lt_assertEqual(L);
}

static int lt_assertNotNil(lua_State* L)
{
	lua_settop(L, 3);  /* obj msg nil */
	lua_insert(L, -2); /* obj nil msg */
	return lt_assertNotEqual(L);
}

static int lt_assertTrue(lua_State* L)
{
	lua_settop(L, 2);                        /* obj msg */
	lua_pushboolean(L, lua_toboolean(L, 1)); /* obj msg bool */
	lua_replace(L, 1);                       /* bool msg */
	lua_pushboolean(L, 1);                   /* bool msg true */
	lua_insert(L, 2);                        /* bool true msg */
	return lt_assertEqual(L);
}

static int lt_assertFalse(lua_State* L)
{
	lua_settop(L, 2);                        /* obj msg */
	lua_pushboolean(L, lua_toboolean(L, 1)); /* obj msg bool */
	lua_replace(L, 1);                       /* bool msg */
	lua_pushboolean(L, 0);                   /* bool msg false */
	lua_insert(L, 2);                        /* bool false msg */
	return lt_assertEqual(L);
}

static int lt_assertIsOfType(lua_State* L)
{
	lua_settop(L, 3);                        /* obj etype msg */
	lua_pushstring(L, luaL_typename(L, 1));  /* obj etype msg otype */
	lua_replace(L, 1);                       /* otype etype msg */
	return lt_assertEqual(L);
}

static int lt_assertIsNotOfType(lua_State* L)
{
	lua_settop(L, 3);                        /* obj etype msg */
	lua_pushstring(L, luaL_typename(L, 1));  /* obj etype msg otype */
	lua_replace(L, 1);                       /* otype etype msg */
	return lt_assertNotEqual(L);
}

static int lt_assertRawEqual(lua_State* L)
{
	return lt_assertRawOp(L, true, "== (raw)");
}

static int lt_assertRawNotEqual(lua_State* L)
{
	return lt_assertRawOp(L, false, "~= (raw)");
}

static int lt_assertIsIn(lua_State* L)
{
	lua_settop(L, 3);
	lua_pushnil(L);
	while (lua_next(L, 2) != 0)
	{
		if (lua_compare(L, 1, -1, LUA_OPEQ))
		{
			return 0;
		}
		lua_pop(L, 1);
	}
	lt_pushErrorString(L, 1, 2, "is in");
	return lua_error(L);
}

static int lt_assertIsNotIn(lua_State* L)
{
	lua_settop(L, 3);
	lua_pushnil(L);
	while (lua_next(L, 2) != 0)
	{
		if (lua_compare(L, 1, -1, LUA_OPEQ))
		{
			lt_pushErrorString(L, 1, 2, "is not in");
			return lua_error(L);
		}
		lua_pop(L, 1);
	}
	return 0;
}

static int lt_assertRaises(lua_State* L)
{
	luaL_argcheck(L, lua_gettop(L) >= 1, 1, "expected function");
	if (lua_pcall(L, lua_gettop(L) - 1, 0, 0))
	{
		return 0;
	}
	else
	{
		return luaL_error(L, "expected error");
	}
}

static int lt_assertRaisesRegex(lua_State* L)
{
	luaL_argcheck(L, lua_gettop(L) >= 2, 2, "expected string and function");
	if (lua_pcall(L, lua_gettop(L) - 2, 0, 0))
	{
		lua_getfield(L, LUA_REGISTRYINDEX, STRFIND); /* regex error string.find */
		lua_insert(L, 1);                            /* string.find regex error */
		lua_insert(L, 2);                            /* string.find error regex */

		if (lua_pcall(L, 2, 1, 0))
		{
			return lua_error(L);
		}

		if (lua_isnil(L, -1))
		{
			return luaL_error(L, "regex doesn't match");
		}

		return 0;
	}
	else
	{
		return luaL_error(L, "expected error");
	}
}

static int lt_udata(lua_State* L)
{
	lua_newuserdata(L, 0);
	return 1;
}

static luaL_Reg ltlib[] = {
	{ "assertEqual", lt_assertEqual },
	{ "assertNotEqual", lt_assertNotEqual },
	{ "assertGreaterThan", lt_assertGreaterThan },
	{ "assertGreaterEqual", lt_assertGreaterEqual },
	{ "assertLessThan", lt_assertLessThan },
	{ "assertLessEqual", lt_assertLessEqual },
	{ "assertNil", lt_assertNil },
	{ "assertNotNil", lt_assertNotNil },
	{ "assertRawEqual", lt_assertRawEqual },
	{ "assertRawNotEqual", lt_assertRawNotEqual },
	{ "assertTrue", lt_assertTrue },
	{ "assertFalse", lt_assertFalse },
	{ "assertIsOfType", lt_assertIsOfType },
	{ "assertIsNotOfType", lt_assertIsNotOfType },
	{ "assertIsIn", lt_assertIsIn },
	{ "assertIsNotIn", lt_assertIsNotIn },
	{ "assertRaises", lt_assertRaises },
	{ "assertRaisesRegex", lt_assertRaisesRegex },
	{ "udata", lt_udata },
	{ NULL, NULL },
};

int luaopen_lt(lua_State* L)
{
	lua_getglobal(L, "string");
	lua_getfield(L, -1, "find");
	if (!lua_isfunction(L, -1))
		return luaL_error(L, "string.find is missing");
	lua_setfield(L, LUA_REGISTRYINDEX, STRFIND);
	lua_pop(L, 1);

	luaL_newlib(L, ltlib);
	return 1;
}
