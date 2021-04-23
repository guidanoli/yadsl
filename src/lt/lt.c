#include <stdio.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

/**
 * Run the main Lua program
 * Arguments:
 * [1] = script file name (string or nil)
 */
static int lt_main(lua_State* L)
{
	/* Lua stack indices */
	const int script = 1;
	int traceback = 0;
	const int namestbl = 3;
	const int errorstbl = 4;
	const int testbenchtbl = 5;

	/* Local variables */
	const char* scriptname;
	int strcnt = 0;
	lua_Integer i, passedcnt = 0, failedcnt = 0;

	/* Note: if a script file is not specified,
	 * then Lua will read from standard input */

	scriptname = lua_tostring(L, script);           /* script */
	
	/* Get debug.traceback for more informative
	 * error messages on protected calls */

	lua_getglobal(L, "debug");                      /* script debug */

	if (lua_istable(L, -1))
	{
		lua_getfield(L, -1, "traceback");           /* script debug traceback */

		if (lua_isfunction(L, -1))
		{
			traceback = 2;
		}

		lua_remove(L, -2);                          /* script traceback */
	}

	/* Push tables for storing errors and failing
	 * function names in order (i.e. as an array) */

	lua_newtable(L);                                /* script traceback namestbl */
	lua_newtable(L);                                /* script traceback namestbl errorstbl */

	/* Load script from file (or from standard input,
	 * if the script name is NULL) and execute it
	 * in protected mode and with traceback */

	if (luaL_loadfile(L, scriptname) ||             /* chunk */
	    lua_pcall(L, 0, 1, traceback))              /* testbenchtbl */
	{
		return lua_error(L);                        /* errmsg */
	}

	if (!lua_istable(L, -1))
	{
		return luaL_error(L, "the test script must return a table");
	}

	/* Traverse the test bench table and call each
	 * function in protected mode and with traceback,
	 * storing any errors in the errors table and
	 * the failing function name in the names table */

	lua_pushnil(L);                                 /* nil */

	while (lua_next(L, testbenchtbl))               /* key value */
	{
		if (!lua_isfunction(L, -1) || !lua_isstring(L, -2))
		{
			lua_pop(L, 1);
			continue;
		}

		/* Take care not to convert numbers to strings in-place */

		lua_pushvalue(L, -2);                       /* key value key */
		fprintf(stderr, "%s ... ", lua_tostring(L, -1));
		lua_pop(L, 1);                              /* key value */
		
		lua_pushvalue(L, testbenchtbl);             /* key value testbenchtbl */

		if (lua_pcall(L, 1, 0, traceback))          /* key */
		{
			failedcnt++;                            /* key err */
			lua_rawseti(L, errorstbl, failedcnt);   /* key */
			lua_pushvalue(L, -1);                   /* key key */
			lua_rawseti(L, namestbl, failedcnt);    /* key */
			fputs("FAIL\n", stderr);
		}
		else
		{
			passedcnt++;
			fputs("ok\n", stderr);
		}

	}

	/* Print the error messages in order of occurrance,
	 * if any, plus the failing function name */

	for (i = 1; i <= failedcnt; ++i)
	{
		lua_rawgeti(L, namestbl, i);                /* name */
		lua_rawgeti(L, errorstbl, i);               /* name err */

		if (lua_isstring(L, -1))
		{
			fprintf(stderr, "\nFAIL: %s\n%s\n", lua_tostring(L, -2),
												lua_tostring(L, -1));
		}
		else
		{
			fprintf(stderr, "\nFAIL: %s\n", lua_tostring(L, -2));
		}

		lua_pop(L, 2);                              /* */
	}

	/* Print brief summary of the test bench run in
	 * terms of passed and failed test cases and
	 * raise an error if any test case has failed */

	if (failedcnt > 0)
	{
		lua_pushfstring(L, "%I failed", failedcnt);
		lua_pushliteral(L, ", ");
		strcnt += 2;
	}

	if (passedcnt > 0)
	{
		lua_pushfstring(L, "%I passed", passedcnt);
		lua_pushliteral(L, ", ");
		strcnt += 2;
	}

	/* Join strings with commas by popping the
	 * last comma and concatenating the rest */

	if (strcnt >= 2)
	{
		lua_pop(L, 1);
		lua_concat(L, strcnt - 1);
	}

	if (failedcnt == 0)
	{
		if (passedcnt > 0)
		{
			fprintf(stderr, "%s\n", lua_tostring(L, -1));
		}

		return 0;
	}
	else
	{
		fputs("\n", stderr);
		return lua_error(L);
	}
}

/**
 * Main program flow
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
	
	/* Call lt_main in protected mode with the script name
	 * or with nil, if not provided */

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
	
	/* Handle errors, if any */

	if (status)
	{
		fprintf(stderr, "%s: %s\n", program, lua_tostring(L, -1));
	}

	/* Close the Lua state */

	lua_close(L);

	return status;
}
