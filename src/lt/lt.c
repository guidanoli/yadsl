#include <string.h>
#include <assert.h>

#include "lauxlib.h"

static int lt_assertEqual(lua_State* L)
{
	luaL_argcheck(L, lua_gettop(L) >= 2, 2, "expected 2 arguments");
	if (lua_compare(L, 1, 2, LUA_OPEQ))
	{
		return 0;
	}
	else
	{
		return luaL_error(L, "%s != %s", luaL_tolstring(L, 1, NULL),
		                                 luaL_tolstring(L, 2, NULL));
	}
}

static int lt_assertNotEqual(lua_State* L)
{
	luaL_argcheck(L, lua_gettop(L) >= 2, 2, "expected 2 arguments");
	if (!lua_compare(L, 1, 2, LUA_OPEQ))
	{
		return 0;
	}
	else
	{
		return luaL_error(L, "%s == %s", luaL_tolstring(L, 1, NULL),
		                                 luaL_tolstring(L, 2, NULL));
	}
}

static int lt_assertGreaterThan(lua_State* L)
{
	luaL_argcheck(L, lua_gettop(L) >= 2, 2, "expected 2 arguments");
	if (!lua_compare(L, 1, 2, LUA_OPLE))
	{
		return 0;
	}
	else
	{
		return luaL_error(L, "%s <= %s", luaL_tolstring(L, 1, NULL),
		                                 luaL_tolstring(L, 2, NULL));
	}
}

static int lt_assertGreaterEqual(lua_State* L)
{
	luaL_argcheck(L, lua_gettop(L) >= 2, 2, "expected 2 arguments");
	if (!lua_compare(L, 1, 2, LUA_OPLT))
	{
		return 0;
	}
	else
	{
		return luaL_error(L, "%s < %s", luaL_tolstring(L, 1, NULL),
		                                luaL_tolstring(L, 2, NULL));
	}
}

static int lt_assertLessThan(lua_State* L)
{
	luaL_argcheck(L, lua_gettop(L) >= 2, 2, "expected 2 arguments");
	if (lua_compare(L, 1, 2, LUA_OPLT))
	{
		return 0;
	}
	else
	{
		return luaL_error(L, "%s >= %s", luaL_tolstring(L, 1, NULL),
		                                 luaL_tolstring(L, 2, NULL));
	}
}

static int lt_assertLessEqual(lua_State* L)
{
	luaL_argcheck(L, lua_gettop(L) >= 2, 2, "expected 2 arguments");
	if (lua_compare(L, 1, 2, LUA_OPLE))
	{
		return 0;
	}
	else
	{
		return luaL_error(L, "%s > %s", luaL_tolstring(L, 1, NULL),
		                                luaL_tolstring(L, 2, NULL));
	}
}

static int lt_assertNil(lua_State* L)
{
	luaL_argcheck(L, lua_gettop(L) >= 1, 1, "expected 1 argument");
	if (lua_isnil(L, 1))
	{
		return 0;
	}
	else
	{
		return luaL_error(L, "%s is not nil", luaL_tolstring(L, 1, NULL));
	}
}

static int lt_assertNotNil(lua_State* L)
{
	luaL_argcheck(L, lua_gettop(L) >= 1, 1, "expected 1 argument");
	if (!lua_isnil(L, 1))
	{
		return 0;
	}
	else
	{
		return luaL_error(L, "%s is nil", luaL_tolstring(L, 1, NULL));
	}
}

static int lt_assertRawEqual(lua_State* L)
{
	luaL_argcheck(L, lua_gettop(L) >= 2, 2, "expected 2 arguments");
	if (lua_rawequal(L, 1, 2))
	{
		return 0;
	}
	else
	{
		return luaL_error(L, "%s != %s (raw)", luaL_tolstring(L, 1, NULL),
		                                       luaL_tolstring(L, 2, NULL));
	}
}

static int lt_assertRawNotEqual(lua_State* L)
{
	luaL_argcheck(L, lua_gettop(L) >= 2, 2, "expected 2 arguments");
	if (!lua_rawequal(L, 1, 2))
	{
		return 0;
	}
	else
	{
		return luaL_error(L, "%s == %s (raw)", luaL_tolstring(L, 1, NULL),
		                                       luaL_tolstring(L, 2, NULL));
	}
}

static int lt_assertTrue(lua_State* L)
{
	luaL_argcheck(L, lua_gettop(L) >= 1, 1, "expected 1 argument");
	if (lua_toboolean(L, 1))
	{
		return 0;
	}
	else
	{
		return luaL_error(L, "%s is false", luaL_tolstring(L, 1, NULL));
	}
}

static int lt_assertFalse(lua_State* L)
{
	luaL_argcheck(L, lua_gettop(L) >= 1, 1, "expected 1 argument");
	if (!lua_toboolean(L, 1))
	{
		return 0;
	}
	else
	{
		return luaL_error(L, "%s is true", luaL_tolstring(L, 1, NULL));
	}
}

static int lt_assertIsOfType(lua_State* L)
{
	const char* obtained, * expected;
	luaL_argcheck(L, lua_gettop(L) >= 2, 2, "expected 2 arguments");
	luaL_argexpected(L, lua_type(L, 2) == LUA_TSTRING, 2, "string");
	obtained = luaL_typename(L, 1);
	expected = lua_tostring(L, 2);
	assert(obtained != NULL);
	assert(expected != NULL);
	if (strcmp(obtained, expected) == 0)
	{
		return 0;
	}
	else
	{
		return luaL_error(L, "%s is of type %s, not %s", luaL_tolstring(L, 1, NULL),
		                                                 obtained,
														 expected);
	}
}

static int lt_assertIsNotOfType(lua_State* L)
{
	const char* obtained, * expected;
	luaL_argcheck(L, lua_gettop(L) >= 2, 2, "expected 2 arguments");
	luaL_argexpected(L, lua_type(L, 2) == LUA_TSTRING, 2, "string");
	obtained = luaL_typename(L, 1);
	expected = lua_tostring(L, 2);
	assert(obtained != NULL);
	assert(expected != NULL);
	if (strcmp(obtained, expected) != 0)
	{
		return 0;
	}
	else
	{
		return luaL_error(L, "%s is of type %s", luaL_tolstring(L, 1, NULL),
		                                         obtained);
	}
}

static int lt_assertIsIn(lua_State* L)
{
	luaL_argcheck(L, lua_gettop(L) >= 2, 2, "expected 2 arguments");
	luaL_argexpected(L, lua_type(L, 2) == LUA_TTABLE, 2, "table");
	lua_pushnil(L);
	while (lua_next(L, 2) != 0)
	{
		if (lua_compare(L, 1, 4, LUA_OPEQ))
		{
			return 0;
		}
		lua_pop(L, 1);
	}
	return luaL_error(L, "%s is not in %s", luaL_tolstring(L, 1, NULL),
	                                        luaL_tolstring(L, 2, NULL));
}

static int lt_assertIsNotIn(lua_State* L)
{
	luaL_argcheck(L, lua_gettop(L) >= 2, 2, "expected 2 arguments");
	luaL_argexpected(L, lua_type(L, 2) == LUA_TTABLE, 2, "table");
	lua_pushnil(L);
	while (lua_next(L, 2) != 0)
	{
		if (lua_compare(L, 1, 4, LUA_OPEQ))
		{
			return luaL_error(L, "%s is in %s (key is %s)", luaL_tolstring(L, 1, NULL),
			                                                luaL_tolstring(L, 2, NULL),
															luaL_tolstring(L, 3, NULL));
		}
		lua_pop(L, 1);
	}
	return 0;
}

static int lt_assertRaises(lua_State* L)
{
	luaL_argcheck(L, lua_gettop(L) >= 1, 1, "expected 1 argument");
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
	luaL_argcheck(L, lua_gettop(L) >= 2, 2, "expected 2 arguments");
	if (lua_pcall(L, lua_gettop(L) - 2, 0, 0))
	{
		if (lua_type(L, -1) != LUA_TSTRING)
		{
			return luaL_error(L, "error object is not a string");
		}

		lua_getglobal(L, "string");

		if (lua_type(L, -1) != LUA_TTABLE)
		{
			return luaL_error(L, "string library is missing");
		}

		lua_getfield(L, -1, "find");
		
		if (lua_type(L, -1) != LUA_TFUNCTION)
		{
			return luaL_error(L, "string.find function is missing");
		}

		lua_remove(L, -2);
		lua_pushvalue(L, 2);
		lua_pushvalue(L, 1);

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
	luaL_newlib(L, ltlib);
	return 1;
}
