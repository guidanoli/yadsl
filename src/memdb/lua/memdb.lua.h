#ifndef MEMDB_LUA_H
#define MEMDB_LUA_H

#include <yadsl/dll.h>
#include "lauxlib.h"

YADSL_EXPORT int luaopen_memdb(lua_State* L);

#endif
