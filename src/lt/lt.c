#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "ltlib.h"

static int g_failed_tests;
static int g_passed_tests;
static int g_test_suite;
static int g_msg_hdlr;
static int g_errors;
static int g_names;

static int get_test_suite_field(lua_State* L, const char* field)
{
	assert((L != NULL) && "Lua state isn't NULL");
	assert((g_test_suite != 0) && "Test suite exists");
	assert((field != NULL) && "Field isn't NULL");

	lua_getfield(L, g_test_suite, field);

	if (lua_isnil(L, -1))
	{
		lua_pop(L, 1);
		return 0;
	}
	else
	{
		return lua_gettop(L);
	}
}

static int call_test_suite_method(lua_State* L, const char* method)
{
	assert((L != NULL) && "Lua state isn't NULL");
	assert((method != NULL) && "Method name isn't NULL");

	int index = get_test_suite_field(L, method);
	if (index == 0)
	{
		return 0;
	}
	else
	{
		lua_pushvalue(L, g_test_suite);
		return lua_pcall(L, 1, 0, g_msg_hdlr);
	}
}

static int run_test_case(lua_State* L)
{
	assert((L != NULL) && "Lua state isn't NULL");

	/* stack: name function */

	if (call_test_suite_method(L, "beforeEach"))
	{
		lua_remove(L, -2); /* stack: name function error */
		return 1;          /* stack: name error */
	}

	/* stack: name function */

	lua_pushvalue(L, g_test_suite); /* stack: name function suite */

	if (lua_pcall(L, 1, 0, g_msg_hdlr))
	{
		return 1; /* stack: name error */
	}

	/* stack: name */

	if (call_test_suite_method(L, "afterEach"))
	{
		return 1; /* stack: name error */
	}

	return 0; /* stack: name */
}

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
			g_msg_hdlr = lua_gettop(L);
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
	g_errors = lua_gettop(L);

	lua_newtable(L);
	g_names = lua_gettop(L);

	/* Open lt library */

	lua_pushcfunction(L, luaopen_lt);
	
	if (lua_pcall(L, 0, 1, g_msg_hdlr))
	{
		return lua_error(L);
	}
	else
	{
		lua_setglobal(L, "lt");
	}

	/* Load script from file (or from standard input,
	 * if the script file path is missing) and execute it
	 * in protected mode */

	if (luaL_loadfile(L, lua_tostring(L, 1)) ||
		lua_pcall(L, 0, 1, g_msg_hdlr))
	{
		return lua_error(L);
	}

	if (!lua_istable(L, -1))
	{
		return luaL_error(L, "the test script must return a table");
	}

	g_test_suite = lua_gettop(L);

	/* Call before all function */

	if (call_test_suite_method(L, "beforeAll"))
	{
		return lua_error(L);
	}

	/* Traverse the test case table and call each
	 * function in protected mode */

	lua_pushnil(L);

	while (lua_next(L, g_test_suite))
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
		
		if (run_test_case(L))
		{
			g_failed_tests++;
			lua_rawseti(L, g_errors, g_failed_tests);
			lua_pushvalue(L, -1);
			lua_rawseti(L, g_names, g_failed_tests);
			fputs("FAIL\n", stderr);
		}
		else
		{
			g_passed_tests++;
			fputs("ok\n", stderr);
		}
	}

	/* Call after all function */

	if (call_test_suite_method(L, "afterAll"))
	{
		return lua_error(L);
	}

	/* For each failing test function, print its name and the error message
	 * related to it, in same order of occurence */

	for (int i = 1; i <= g_failed_tests; ++i)
	{
		lua_rawgeti(L, g_names, i);
		lua_rawgeti(L, g_errors, i);
		fprintf(stderr, "\nFAIL: %s\n%s\n", lua_tostring(L, -2),
		                                    lua_tostring(L, -1));
		lua_pop(L, 2);
	}

	/* Print separator */

	if (g_failed_tests > 0 || g_passed_tests > 0)
	{
		fputs("\n", stderr);
	}

	/* Push brief message */

	lua_pushfstring(L, "%I failed, %I passed", (lua_Integer)g_failed_tests, (lua_Integer)g_passed_tests);

	if (g_failed_tests == 0)
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
