#include <limits.h>
#include <assert.h>

#include <memdb/memdb.h>
#include <memdb/lua/memdb.lua.h>

#define PTR "MEMDB_POINTER"
#define UNSAFE_PTR "MEMDB_UNSAFE_POINTER"

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

static int set_fail_countdown(lua_State* L) {
	luaL_argexpected(L, lua_isinteger(L, 1), 1, "expected integer");
	lua_Integer cd = lua_tointeger(L, 1);
	luaL_argcheck(L, cd >= 0, 1, "expected positive integer");
	yadsl_memdb_set_fail_countdown((size_t) cd);
	return 0;
}

static int get_fail_countdown(lua_State* L) {
	size_t cd = yadsl_memdb_get_fail_countdown();
	if (cd > (size_t)LUA_MAXINTEGER)
		return luaL_error(L, "integer overflow");
	lua_pushinteger(L, (lua_Integer)cd);
	return 1;
}

static const char * const log_channels[] = {
	"allocation",
	"deallocation",
	"leakage",
};

#define sizeofv(x) (sizeof(x)/sizeof(*x))

static int get_log_channel(lua_State* L) {
	yadsl_MemDebugLogChannel log_channel;
	bool enabled;
	log_channel = luaL_checkoption(L, 1, NULL, log_channels);
	assert(log_channel >= 0 && log_channel < sizeofv(log_channels));
	enabled = yadsl_memdb_log_channel_get(log_channel);
	lua_pushboolean(L, enabled);
	return 1;
}

static int get_log_channel_list(lua_State* L) {
	lua_createtable(L, sizeofv(log_channels), 0);
	assert(sizeofv(log_channels) < INT_MAX);
	for (int i = 0; i < sizeofv(log_channels); ++i) {
		lua_pushstring(L, log_channels[i]);
		lua_rawseti(L, -2, i + 1);
	}
	return 1;
}

static int set_log_channel(lua_State* L) {
	yadsl_MemDebugLogChannel log_channel;
	bool enabled;
	log_channel = luaL_checkoption(L, 1, NULL, log_channels);
	assert(log_channel >= 0 && log_channel < sizeofv(log_channels));
	luaL_argexpected(L, lua_isboolean(L, 2), 2, "boolean");
	enabled = lua_toboolean(L, 2);
	yadsl_memdb_log_channel_set(log_channel, enabled);
	return 0;
}

static int set_all_log_channels(lua_State* L) {
	luaL_argexpected(L, lua_isboolean(L, 1), 1, "boolean");
	bool enable = lua_toboolean(L, 1);
	for (size_t i = 0; i < sizeofv(log_channels); ++i)
		yadsl_memdb_log_channel_set((yadsl_MemDebugLogChannel)i, enable);
	return 0;
}

static int safe_malloc(lua_State* L) {
	lua_Integer size = luaL_checkinteger(L, 1);
	if (size < 0) return luaL_error(L, "negative size");
	void** uv = (void**) lua_newuserdata(L, sizeof(void*));
	*uv = NULL;
	luaL_setmetatable(L, PTR);
	void* p = malloc((size_t) size);
	if (p == NULL) return luaL_error(L, "bad malloc");
	*uv = p;
	return 1;
}

static int unsafe_malloc(lua_State* L) {
	lua_Integer size = luaL_checkinteger(L, 1);
	if (size < 0) return luaL_error(L, "negative size");
	void** uv = (void**) lua_newuserdata(L, sizeof(void*));
	*uv = NULL;
	luaL_setmetatable(L, UNSAFE_PTR);
	void* p = malloc((size_t) size);
	if (p == NULL) return luaL_error(L, "bad malloc");
	*uv = p;
	return 1;
}

static int unsafe_free(lua_State* L) {
	void** uv = (void**) luaL_checkudata(L, 1, UNSAFE_PTR);
	bool isvalid = *uv != NULL;
	lua_pushboolean(L, isvalid);
	if (isvalid) {
		free(*uv);
		*uv = NULL;
	}
	return 1;
}

static int pointer_finalizer(lua_State* L) {
	void** uv = (void**) lua_touserdata(L, 1);
	if (*uv != NULL) free(*uv);
	return 0;
}

static const struct luaL_Reg memdblib[] = {
        {"get_amb_list_size", get_amb_list_size},
		{"get_amb_list", get_amb_list},
        {"error_occurred", error_occurred},
		{"set_fail_countdown", set_fail_countdown},
		{"get_fail_countdown", get_fail_countdown},
		{"get_log_channel", get_log_channel},
		{"get_log_channel_list", get_log_channel_list},
		{"set_log_channel", set_log_channel},
		{"set_all_log_channels", set_all_log_channels},
		{"safe_malloc", safe_malloc},
		{"malloc", unsafe_malloc},
		{"free", unsafe_free},
        {NULL, NULL}  /* sentinel */
};


YADSL_EXPORT int luaopen_memdb(lua_State* L) {
    luaL_newlib(L, memdblib);

	/* register the 'debug' field */
#ifdef YADSL_DEBUG
	lua_pushboolean(L, 1);
#else
	lua_pushboolean(L, 0);
#endif
	lua_setfield(L, -2, "debug");

	/* register the PTR metatable */
	lua_createtable(L, 0, 3);
	lua_pushboolean(L, 0);
	lua_setfield(L, -2, "__metatable");
	lua_pushcfunction(L, pointer_finalizer);
	lua_setfield(L, -2, "__gc");
	lua_pushstring(L, PTR);
	lua_setfield(L, -2, "__name");
	lua_setfield(L, LUA_REGISTRYINDEX, PTR);

	/* register the UNSAFE_PTR metatable */
	lua_createtable(L, 0, 2);
	lua_pushboolean(L, 0);
	lua_setfield(L, -2, "__metatable");
	lua_pushstring(L, UNSAFE_PTR);
	lua_setfield(L, -2, "__name");
	lua_setfield(L, LUA_REGISTRYINDEX, UNSAFE_PTR);

    return 1;
}
