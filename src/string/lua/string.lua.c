#include <yadsl/dll.h>

#include <string/string.h>

#include <yadsl/stdlib.h>

#include <memdb/lua/memdb.h>
#include "lauxlib.h"

static int duplicate(lua_State* L)
{
	const char* str = luaL_checkstring(L, 1);
	char* dup = yadsl_string_duplicate(str);
	if (dup == NULL) return luaL_error(L, "bad malloc");
	lua_pushstring(L, dup);
	free(dup);
	return 1;
}

static int compare_ic(lua_State* L)
{
	const char* str1 = luaL_checkstring(L, 1);
	const char* str2 = luaL_checkstring(L, 2);
	int cmp = yadsl_string_compare_ic(str1, str2);
	lua_pushinteger(L, (lua_Integer)cmp);
	return 1;
}

static const struct luaL_Reg stringlib[] = {
        {"duplicate", duplicate},
		{"compare_ic", compare_ic},
        {NULL, NULL}  /* sentinel */
};


YADSL_EXPORT int luaopen_stringutils(lua_State* L)
{
    luaL_newlib(L, stringlib);

	/* register 'memdb' submodule */
	yadsl_memdb_openlib(L);
	lua_setfield(L, -2, "memdb");

	return 1;
}
