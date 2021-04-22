#include <stdio.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

/**
 * Checks if the value at the given valid index is of a
 * given expected type. If not, a Lua error is raised
 * informing the value of a given name has the wrong type.
 */
static void lt_checktype(lua_State* L, int idx, const char* name, int expected_type)
{
	if (lua_type(L, idx) != expected_type)
	{
		luaL_error(L, "%s must be %s, not %s", name,
											   lua_typename(L, expected_type),
		                                       luaL_typename(L, idx));
	}
}

/**
 * Run the main Lua program
 * Arguments:
 * [1] = script file name (string or nil)
 */
static int lt_main(lua_State* L)
{
	/* Lua stack indices */
	const int script = 1;
	const int traceback = 2;
	const int namestbl = 3;
	const int errorstbl = 4;
	const int testbenchtbl = 5;

	const char* scriptname;
	lua_Integer i, n = 0;

	lua_getglobal(L, "debug");         /* script debug */
	lt_checktype(L, -1, "debug", LUA_TTABLE);

	lua_getfield(L, -1, "traceback");  /* script debug traceback */
	lt_checktype(L, -1, "debug.traceback", LUA_TFUNCTION);

	lua_remove(L, -2);                 /* script traceback */
	lua_newtable(L);                   /* script traceback namestbl */
	lua_newtable(L);                   /* script traceback namestbl errorstbl */

	if (lua_isstring(L, script))
	{
		scriptname = lua_tostring(L, script);
	}
	else
	{
		/* Note: if a script file is not specified,
		 * then luaL_loadfile reads from stdin */
		scriptname = NULL;
	}

	if (luaL_loadfile(L, scriptname) ||  /* chunk */
	    lua_pcall(L, 0, 1, traceback))   /* testbenchtbl */
	{
		return lua_error(L);             /* errmsg */
	}

	lt_checktype(L, -1, "test bench", LUA_TTABLE);

	lua_pushnil(L);                         /* nil */

	while (lua_next(L, testbenchtbl))       /* key value */
	{
		const char* key;

		lt_checktype(L, -2, "test case name", LUA_TSTRING);

		key = lua_tostring(L, -2);

		lt_checktype(L, -1, "test case", LUA_TFUNCTION);
		
		lua_pushvalue(L, testbenchtbl);     /* key value testbenchtbl */

		fprintf(stderr, "%s ... ", key);

		if (lua_pcall(L, 1, 0, traceback))  /* key */
		{
			n++;                            /* key err */
			lua_rawseti(L, errorstbl, n);   /* key */
			lua_pushvalue(L, -1);           /* key key */
			lua_rawseti(L, namestbl, n);    /* key */
			fputs("FAIL\n", stderr);
		}
		else
		{
			fputs("ok\n", stderr);
		}
	}

	for (i = 1; i <= n; ++i)
	{
		lua_rawgeti(L, namestbl, i);      /* name */
		lua_rawgeti(L, errorstbl, i);     /* name err */
		fprintf(stderr, "\nFAIL: %s\n%s\n", lua_tostring(L, -2),
		                                    lua_tostring(L, -1));
		lua_pop(L, 2);                    /* */
	}

	if (n == 0)
	{
		return 0;
	}
	else
	{
		fputs("\n", stderr);
		return luaL_error(L, "%I test cases failed", n);
	}
}

/**
 * Main program flow
 */
int main(int argc, char** argv)
{
	const char* program;
	lua_State* L;
	int status;

	/* Specified in 5.1.2.2.1 (Program Startup) of ISO/IEC 9899:TC3
	 *
	 * If the value of argc is greater than zero, the string pointed to by argv[0]
	 * represents the program name; argv[0][0] shall be the null character if the
	 * program name is not available from the host environment.
	 */
	if (argc > 0 && argv[0][0] != '\0')
	{
		program = argv[0];
	}
	else
	{
		program = "lt";
	}

	L = luaL_newstate();

	if (L == NULL)
	{
		fprintf(stderr, "%s: Could not allocate Lua state\n", program);
		return LUA_ERRMEM;
	}

	luaL_openlibs(L);
	
	lua_pushcfunction(L, lt_main);

	if (argc >= 2)
	{
		lua_pushstring(L, argv[1]);
	}
	else
	{
		lua_pushnil(L);
	}

	status = lua_pcall(L, 1, 0, 0);
	
	if (status)
	{
		fprintf(stderr, "%s: %s\n", program, lua_tostring(L, -1));
	}

	lua_close(L);

	return status;
}
