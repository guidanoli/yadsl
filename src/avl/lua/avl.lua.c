#include <yadsl/dll.h>

#include "lauxlib.h"

#include <avl/avl.h>
#include <memdb/lua/memdb.h>

static const struct luaL_Reg avllib[] = {
        {NULL, NULL}  /* sentinel */
};

YADSL_EXPORT int luaopen_avl(lua_State* L) {
    luaL_newlib(L, avllib);
    yadsl_memdb_openlib(L);
    lua_setfield(L, -2, "memdb");
    return 1;
}
