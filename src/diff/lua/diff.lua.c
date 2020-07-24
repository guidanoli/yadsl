#include "dlldefines.h"

#include "lauxlib.h"

#include "diff.h"

static int c_diff(lua_State* L) {
    size_t size1, size2;
    const char* s1 = luaL_checklstring(L, 1, &size1);
    const char* s2 = luaL_checklstring(L, 2, &size2);
    if (!s1 || !s2)
        return 0;
    double diffs1s2 = diff(s1, s2);
    if (diffs1s2 == -1.0)
        return 0;
    lua_pushnumber(L, diffs1s2);
    return 1;
}

static const struct luaL_Reg luadiff[] = {
        {"diff", c_diff},
        {NULL, NULL}  /* sentinel */
};

EXPORT int luaopen_luadiff(lua_State* L) {
    luaL_newlib(L, luadiff);
    return 1;
}
