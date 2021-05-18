#include <stdio.h>
#include <string.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

/**
 * Main Lua routine
 *
 * Arguments:
 * [1] = script file path (string or nil)
 *
 * Returns:
 * [1] = brief message (string)
 */
static int lt_main(lua_State* L)
{
	const int script = 1;
	int msgh = 0;
	int names, errors, testcase;
	lua_Integer passedcnt, failedcnt;

	lua_settop(L, 1);

	/* Try to get a message handler for generating
	 * more informative error messages on protected calls.
	 * If a message handler could not be found, pushes nil. */

	lua_getglobal(L, "debug");

	if (lua_istable(L, -1))
	{
		lua_getfield(L, -1, "traceback");

		if (lua_isfunction(L, -1))
		{
			/* If 'debug.traceback' is a function, use it as a
			 * message handler for protected calls */

			lua_remove(L, -2);
			msgh = lua_gettop(L);
		}
		else
		{
			lua_pop(L, 2);
			lua_pushnil(L);
		}
	}
	else
	{
		lua_pop(L, 1);
		lua_pushnil(L);
	}

	/* Push arrays for storing error messages and failing
	 * test function names in order of occurence */

	lua_newtable(L);
	errors = lua_gettop(L);

	lua_newtable(L);
	names = lua_gettop(L);

	/* Load script from file (or from standard input,
	 * if the script file path is missing) and execute it
	 * in protected mode */

	if (luaL_loadfile(L, lua_tostring(L, script)) ||
		lua_pcall(L, 0, 1, msgh))
	{
		return lua_error(L);
	}

	if (!lua_istable(L, -1))
	{
		return luaL_error(L, "the test script must return a table");
	}

	testcase = lua_gettop(L);

	/* Traverse the test case table and call each
	 * function in protected mode */

	passedcnt = 0;
	failedcnt = 0;

	lua_pushnil(L);

	while (lua_next(L, testcase))
	{
		const char* name;

		/* Ignore table entries whose keys aren't strings or
		 * whose values aren't functions */

		if (!lua_isfunction(L, -1) || lua_type(L, -2) != LUA_TSTRING)
		{
			lua_pop(L, 1);
			continue;
		}

		/* Check if function name starts with "test" */

		name = lua_tostring(L, -2);

		if (strstr(name, "test") != name)
		{
			lua_pop(L, 1);
			continue;
		}

		/* Print function name */

		fprintf(stderr, "%s ... ", name);
		
		/* Call each test function with the test case */

		lua_pushvalue(L, testcase);

		if (lua_pcall(L, 1, 0, msgh))
		{
			/* For each failing test function, append the error message to
			 * the 'errors' array, and the name of the test function to the
			 * 'names' array. */

			failedcnt++;
			lua_rawseti(L, errors, failedcnt);
			lua_pushvalue(L, -1);
			lua_rawseti(L, names, failedcnt);
			fputs("FAIL\n", stderr);

			if (failedcnt < 0)
			{
				return luaL_error(L, "failed tests counter overflew");
			}
		}
		else
		{
			passedcnt++;
			fputs("ok\n", stderr);

			if (passedcnt < 0)
			{
				return luaL_error(L, "passed tests counter overflew");
			}
		}

	}

	/* For each failing test function, print its name and the error message
	 * related to it, in same order of occurence */

	for (lua_Integer i = 1; i <= failedcnt; ++i)
	{
		lua_rawgeti(L, names, i);
		lua_rawgeti(L, errors, i);
		fprintf(stderr, "\nFAIL: %s\n%s\n", lua_tostring(L, -2),
		                                    lua_tostring(L, -1));
		lua_pop(L, 2);
	}

	/* Print separator */

	if (failedcnt > 0 || passedcnt > 0)
	{
		fputs("\n", stderr);
	}

	/* Push brief message */

	lua_pushfstring(L, "%I failed, %I passed", failedcnt, passedcnt);

	if (failedcnt == 0)
	{
		/* Return brief message */

		return 1;
	}
	else
	{
		/* Raise error with brief message */

		return lua_error(L);
	}
}

/**
 * Main routine
 * Usage: lt [script]
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
	 * program name is not available from the host environment. */
	
	if (argc > 0 && argv[0][0] != '\0')
	{
		program = argv[0];
	}
	else
	{
		program = "lt";
	}

	/* Create a Lua state */

	L = luaL_newstate();

	if (L == NULL)
	{
		fprintf(stderr, "%s: Failed memory allocation for Lua state\n", program);
		return LUA_ERRMEM;
	}

	/* Open standard libraries */

	luaL_openlibs(L);
	
	/* Call lt_main with the script file path or with nil if not provided */

	lua_pushcfunction(L, lt_main);

	if (argc >= 2)
	{
		lua_pushstring(L, argv[1]);
	}
	else
	{
		lua_pushnil(L);
	}

	status = lua_pcall(L, 1, 1, 0);
	
	/* Print brief message */

	fprintf(stderr, "%s: %s\n", program, lua_tostring(L, -1));

	/* Close the Lua state */

	lua_close(L);

	return status;
}
