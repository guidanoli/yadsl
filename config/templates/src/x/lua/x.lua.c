#include "dlldefines.h"

#include "lauxlib.h"

#include "%name%.h"

static const struct luaL_Reg lua%name%[] = {
    {NULL, NULL}  /* sentinel */
};

EXPORT int luaopen_lua%name%(lua_State* L) {
    luaL_newlib(L, lua%name%);
    return 1;
}
