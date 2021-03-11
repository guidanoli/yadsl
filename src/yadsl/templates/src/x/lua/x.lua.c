#include "lauxlib.h"

#include <yadsl/dll.h>

#include <%name%/%name%.h>

static const struct luaL_Reg lua%name%[] = {
    {NULL, NULL}  /* sentinel */
};

YADSL_EXPORT int luaopen_lua%name%(lua_State* L) {
    luaL_newlib(L, lua%name%);
    return 1;
}
