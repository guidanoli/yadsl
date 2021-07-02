#include <yadsl/dll.h>

#include "lauxlib.h"

#include <diff/diff.h>
#include <memdb/lua/memdb.h>

static int l_diff(lua_State* L) {
    const char* s1 = luaL_checkstring(L, 1);
    const char* s2 = luaL_checkstring(L, 2);
    if (!s1 || !s2)
        return 0;
    double result = yadsl_utils_diff(s1, s2);
    if (result == -1.0)
        return 0;
    lua_pushnumber(L, result);
    return 1;
}

static const struct luaL_Reg difflib[] = {
        {"diff", l_diff},
        {NULL, NULL}  /* sentinel */
};

YADSL_EXPORT int luaopen_diff(lua_State* L) {
    luaL_newlib(L, difflib);
    yadsl_memdb_openlib(L);
    lua_setfield(L, -2, "memdb");
    return 1;
}
