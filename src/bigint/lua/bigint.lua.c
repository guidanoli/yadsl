#include "lauxlib.h"

#include <yadsl/dlldefines.h>

#include <bigint/bigint.h>

static const struct luaL_Reg luabigint[] = {
    {NULL, NULL}  /* sentinel */
};

YADSL_EXPORT int luaopen_luabigint(lua_State* L) {
    luaL_newlib(L, luabigint);
    return 1;
}
