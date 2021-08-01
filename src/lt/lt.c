#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "lauxlib.h"

/* Add value at index val to the buffer B */
static void lt_addvalue(lua_State* L, luaL_Buffer* B, int val)
{
	switch (lua_type(L, val))
	{
		case LUA_TNIL:
		case LUA_TNUMBER:
		case LUA_TBOOLEAN:
			luaL_tolstring(L, val, NULL);
			luaL_addvalue(B);
			break;
		case LUA_TSTRING:
			luaL_addchar(B, '"');
			lua_pushvalue(L, val);
			luaL_addvalue(B);
			luaL_addchar(B, '"');
			break;
		case LUA_TTABLE:
		case LUA_TFUNCTION:
		case LUA_TUSERDATA:
		case LUA_TTHREAD:
		case LUA_TLIGHTUSERDATA:
			luaL_addchar(B, '<');
			luaL_tolstring(L, val, NULL);
			luaL_addvalue(B);
			luaL_addchar(B, '>');
			break;
		default:
			assert(0 && "Unknown Lua type");
			luaL_addchar(B, '?');
	}
}

/* Push error message containing values at indices op1 and op2 with the string opstr in
 * between + an optional error message at index errmsg */
static void lt_pushErrorString(lua_State* L, int op1, int op2, const char* opstr, int errmsg)
{
	luaL_Buffer b;
	luaL_buffinit(L, &b);
	luaL_addstring(&b, "assertion `");
	lt_addvalue(L, &b, op1);
	luaL_addchar(&b, ' ');
	luaL_addstring(&b, opstr);
	luaL_addchar(&b, ' ');
	lt_addvalue(L, &b, op2);
	luaL_addstring(&b, "' failed");
	if (lua_isstring(L, errmsg)) {
		luaL_addstring(&b, ": ");
		lua_pushvalue(L, errmsg);
		luaL_addvalue(&b);
	}
	luaL_pushresult(&b);
}

static int lt_assertBinop(lua_State* L, int op, bool invert, const char* opstr)
{
	lua_settop(L, 3);
	if (lua_compare(L, 1, 2, op) == invert)
	{
		lt_pushErrorString(L, 1, 2, opstr, 3);
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
		lt_pushErrorString(L, 1, 2, opstr, 3);
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

static int lt_assertType(lua_State* L)
{
	lua_settop(L, 3);                        /* obj etype msg */
	lua_pushstring(L, luaL_typename(L, 1));  /* obj etype msg otype */
	lua_replace(L, 1);                       /* otype etype msg */
	return lt_assertEqual(L);
}

static int lt_assertNotType(lua_State* L)
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

/* Asserts element is in table and returns key
 * Usage: lt.assertValue(obj, tbl : table [, msg : string])
 * Returns: key, such that tbl[key] = obj */
static int lt_assertValue(lua_State* L)
{
	luaL_checktype(L, 2, LUA_TTABLE);  /* obj tbl [...] */
	lua_settop(L, 3);                  /* obj tbl msg */
	lua_pushnil(L);                    /* obj tbl msg nil */
	while (lua_next(L, 2) != 0)        /* obj tbl msg key value */
	{
		if (lua_compare(L, 1, -1, LUA_OPEQ))
		{
			lua_pop(L, 1);             /* obj tbl msg key */
			return 1;
		}
		lua_pop(L, 1);                 /* obj tbl msg key */
	}
	lt_pushErrorString(L, 1, 2, "is in", 3);
	return lua_error(L);
}

static int lt_assertNotValue(lua_State* L)
{
	luaL_checktype(L, 2, LUA_TTABLE);  /* obj tbl [...] */
	lua_settop(L, 3);                  /* obj tbl msg */
	lua_pushnil(L);                    /* obj tbl msg nil */
	while (lua_next(L, 2) != 0)        /* obj tbl msg key value */
	{
		if (lua_compare(L, 1, -1, LUA_OPEQ))
		{
			lt_pushErrorString(L, 1, 2, "is not in", 3);
			return lua_error(L);
		}
		lua_pop(L, 1);                 /* obj tbl msg key */
	}
	return 0;
}

/* Asserts string (haystack) has substring (needle)
 * Usage: lt.assertSubstring(needle : string, haystack : string [, msg : string])
 * Returns index where substring was found */
static int lt_assertSubstring(lua_State* L)
{
	const char* needle, * haystack, * ptr;
	needle = luaL_checkstring(L, 1);
	haystack = luaL_checkstring(L, 2);
	if ((ptr = strstr(haystack, needle)) != NULL)
	{
		lua_pushinteger(L, ptr - haystack + 1);
		return 1;
	}
	lt_pushErrorString(L, 1, 2, "is not a substring of", 3);
	return lua_error(L);
}

/* Asserts string (haystack) does not have substring (needle)
 * Usage: lt.assertNotSubstring(needle : string, haystack : string [, msg : string]) */
static int lt_assertNotSubstring(lua_State* L)
{
	const char* needle, * haystack, * ptr;
	needle = luaL_checkstring(L, 1);
	haystack = luaL_checkstring(L, 2);
	if ((ptr = strstr(haystack, needle)) == NULL)
	{
		return 0;
	}
	lt_pushErrorString(L, 1, 2, "is a substring of", 3);
	return lua_error(L);
}

/* Asserts function call raises an error
 * Usage: lt.assertRaises(f : function, ...)
 * Returns: error object */
static int lt_assertRaises(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TFUNCTION);
	if (lua_pcall(L, lua_gettop(L) - 1, 0, 0))
	{
		return 1;
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
	{ "assertType", lt_assertType },
	{ "assertNotType", lt_assertNotType },
	{ "assertValue", lt_assertValue },
	{ "assertNotValue", lt_assertNotValue },
	{ "assertSubstring", lt_assertSubstring },
	{ "assertNotSubstring", lt_assertSubstring },
	{ "assertRaises", lt_assertRaises },
	{ "udata", lt_udata },
	{ NULL, NULL },
};

int luaopen_lt(lua_State* L)
{
	luaL_newlib(L, ltlib);
	return 1;
}
