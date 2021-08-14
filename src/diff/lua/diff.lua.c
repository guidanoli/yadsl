#include <yadsl/dll.h>

#include "lauxlib.h"

#include <diff/diff.h>
#include <memdb/lua/memdb.h>

static int l_diff(lua_State* L) {
    size_t alen, blen;
    const char* a = luaL_checklstring(L, 1, &alen);
    const char* b = luaL_checklstring(L, 2, &blen);
    double result = yadsl_utils_diff(a, alen, b, blen);
    if (result < 0.0) return luaL_error(L, "bad malloc");
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
