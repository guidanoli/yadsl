#include <yadsl/dll.h>
#include <assert.h>

#include "lauxlib.h"

#include <memdb/memdb.h>

static int get_amb_list_size(lua_State* L) {
	lua_pushinteger(L, (lua_Integer)yadsl_memdb_amb_list_size());
	return 1;
}

static int get_amb_list(lua_State* L) {
	size_t size = yadsl_memdb_amb_list_size();
	yadsl_MemDebugAMB* list = yadsl_memdb_get_amb_list();
	lua_createtable(L, size, 0);
	while (list != NULL)
	{
		lua_createtable(L, 0, 5);
		lua_pushstring(L, list->funcname);
		lua_setfield(L, -2, "funcname");
		lua_pushstring(L, list->file);
		lua_setfield(L, -2, "filename");
		lua_pushinteger(L, (lua_Integer)list->line);
		lua_setfield(L, -2, "line");
		lua_pushinteger(L, (lua_Integer)list->size);
		lua_setfield(L, -2, "size");
		lua_pushlightuserdata(L, list->amb);
		lua_setfield(L, -2, "address");
		lua_rawseti(L, -2, (lua_Integer)size);
		list = list->next;
		size--;
	}
	assert(size == 0);
	return 1;
}

static int error_occurred(lua_State* L) {
	lua_pushboolean(L, (int)yadsl_memdb_error_occurred());
	return 1;
}

static const struct luaL_Reg memdblib[] = {
        {"get_amb_list_size", get_amb_list_size},
		{"get_amb_list", get_amb_list},
        {"error_occurred", error_occurred},
        {NULL, NULL}  /* sentinel */
};

YADSL_EXPORT int luaopen_memdb(lua_State* L) {
    luaL_newlib(L, memdblib);
    return 1;
}
