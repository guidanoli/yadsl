#include <stdio.h>
#include <stdarg.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "lt.h"

static const char* const LT_TRACEBACK = "LT_TRACEBACK";

static const char* script_name;
static const char* program_name;
static lua_State* L;

static void print(const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	fprintf(stderr, "%s: ", program_name);
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
	va_end(va);
}

static void printluaerr(int st)
{
	switch (st)
	{
		case LUA_OK:
			break;
		case LUA_ERRRUN:
			print("Runtime error");
			break;
		case LUA_ERRMEM:
			print("Memory allocation error");
			break;
		case LUA_ERRERR:
			print("Error while running the message handler");
			break;
		case LUA_ERRSYNTAX:
			print("Syntax error");
			break;
		case LUA_YIELD:
			print("Thread yielded");
			break;
		case LUA_ERRFILE:
			print("File-related error");
			break;
		default:
			print("Unknown error");
	}
}

/**
 * Initializes the program with the arguments provided on startup
 * Returns the current program status
 */
static int initialize(int argc, char** argv)
{
	/* Specified in 5.1.2.2.1 (Program Startup) of ISO/IEC 9899:TC3
	 *
	 * If the value of argc is greater than zero, the string pointed to by argv[0]
	 * represents the program name; argv[0][0] shall be the null character if the
	 * program name is not available from the host environment.
	 */
	if (argc > 0 && argv[0][0] != '\0')
	{
		program_name = argv[0];
	}
	else
	{
		program_name = "lt";
	}

	if (argc >= 2)
	{
		script_name = argv[1];
	}

	L = luaL_newstate();

	if (L == NULL)
	{
		print("Could not allocate Lua state");
		return LUA_ERRMEM;
	}

	/* Load Lua libraries */
	luaL_openlibs(L);

	if (!lua_checkstack(L, 7))
	{
		print("Could not pre-allocate stack space");
		return LUA_ERRMEM;
	}

	/* Load user library */
	if (lt_openlib(L) < 1)
	{
		print("Could not open user library");
		return LUA_ERRRUN;
	}

	lua_settop(L, 1);                   /* lib */

	return 0;
}

/**
 * Run the program
 * Receives the current program status
 * Returns the (new) program status
 */
static int run(int st)
{
	if (st)
	{
		return st;
	}

	lua_getglobal(L, "debug");          /* lib debug */
	lua_getfield(L, -1, "traceback");   /* lib debug traceback */
	lua_remove(L, -2);                  /* lib traceback */

	/* Note: if script is not specified, script_name = NULL,
	 * and luaL_loadfile reads from stdin */
	st = luaL_loadfile(L, script_name); /* lib traceback func */
	
	if (st)
	{
		printluaerr(st);
		return st;
	}

	st = lua_pcall(L, 0, 1, -2);        /* lib traceback ret */

	if (st)
	{
		if (st == LUA_ERRRUN)
		{
			fprintf(stderr, "%s\n", lua_tostring(L, -1));
		}
		printluaerr(st);
		return st;
	}

	if (!lua_istable(L, -1))
	{
		print("Expected script to return table");
		return LUA_ERRRUN;
	}

	lua_pushnil(L);                       /* lib traceback tbl nil */

	while (lua_next(L, -1))               /* lib traceback tbl key value */
	{
		size_t keylen;
		const char* key;

		if (lua_isstring(L, -2))
		{
			key = lua_tolstring(L, -2, &keylen);
		}
		else
		{
			print("Table key is %s, not string", lua_typename(L, lua_type(L, -2)));
			lua_pop(L, 1);                /* lib traceback tbl key */
			continue;
		}

		if (lua_isfunction(L, -1))
		{
			lua_pushvalue(L, -3);         /* lib traceback tbl key value tbl */
			lua_pushvalue(L, -6);         /* lib traceback tbl key value tbl lib */
			st = lua_pcall(L, 2, 0, -6);  /* lib traceback tbl key */
			
			if (st)
			{
				if (st == LUA_ERRRUN)
				{
					fprintf(stderr, "%s\n", lua_tostring(L, -1));
				}
				printluaerr(st);
				return st;
			}
		}
		else
		{
			print("Table value is %s, not function", lua_typename(L, lua_type(L, -1)));
			lua_pop(L, 1);                /* lib traceback tbl key */
			continue;
		}
	}

	return 0;
}

/**
 * Terminates the program and all its dependencies
 * Receives the current program status
 * Returns the (new) program status
 */
static int terminate(int st)
{
	if (L != NULL)
	{
		lua_close(L);
		L = NULL;
	}

	return st;
}

/**
 * Main program flow
 */
int main(int argc, char** argv)
{
	int st;
	st = initialize(argc, argv);
	st = run(st);
	return terminate(st);
}
