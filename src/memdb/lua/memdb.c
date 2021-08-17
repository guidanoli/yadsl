#include <limits.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <memdb/memdb.h>

#include "lauxlib.h"

#define PTR "Pointer"
#define CONSTPTR "ConstantPointer"

/* Userdata structs */

typedef struct
{
	void* ptr; /* nullable -- null when deleted */
	char* filename; /* nullable -- null when deleted */
}
ptr_t;

typedef struct
{
	const void* ptr; /* nullable -- null when deleted */
}
constptr_t;

typedef struct
{
	lua_State* listen_thread; /* null if listen_cb is nil */
	lua_State* ask_thread; /* null if ask_cb is nil */
}
pcallarg_t;

/* Helper functions */

static size_t validate_size(lua_State* L, int arg)
{
	size_t size;
	size = luaL_checkinteger(L, arg);
	if (size < 0) return luaL_argerror(L, arg, "negative size");
	return size;
}

static int validate_line(lua_State* L, int arg)
{
	lua_Integer line;
	line = luaL_checkinteger(L, arg);
	if (line > INT_MAX) return luaL_argerror(L, 3, "integer overflow");
	return (int)line;
}

static char* copy_string_at(lua_State* L, int arg)
{
	size_t len;
	char* strcopy;
	const char* str = luaL_checklstring(L, arg, &len);
	strcopy = malloc(len + 1);
	if (strcopy != NULL)
		memcpy(strcopy, str, len + 1);
	return strcopy;
}

/* memdb functions */

static ptr_t* new_ptr(lua_State* L)
{
	ptr_t* ptr = (ptr_t*)lua_newuserdata(L, sizeof(ptr_t));
	ptr->ptr = NULL;
	ptr->filename = NULL;
	luaL_setmetatable(L, PTR);
	return ptr;
}

static int lmemdb_malloc(lua_State* L)
{
	void* p;
	size_t size = validate_size(L, 1);
	char* filename;
	int line = validate_line(L, 3);
	ptr_t* ptr = new_ptr(L);
	filename = copy_string_at(L, 2);
	if (filename == NULL) return luaL_error(L, "could not copy filename");
	ptr->filename = filename;
	p = yadsl_memdb_malloc(size, filename, line);
	if (size != 0 && p == NULL) return luaL_error(L, "bad malloc");
	ptr->ptr = p;
	return 1;
}

static int lmemdb_calloc(lua_State* L)
{
	void* p;
	size_t nmemb = validate_size(L, 1);
	size_t size = validate_size(L, 2);
	char* filename;
	int line = validate_line(L, 4);
	ptr_t* ptr = new_ptr(L);
	filename = copy_string_at(L, 3);
	if (filename == NULL) return luaL_error(L, "could not copy filename");
	ptr->filename = filename;
	p = yadsl_memdb_calloc(nmemb, size, filename, line);
	if (nmemb != 0 && size != 0 && p == NULL) return luaL_error(L, "bad calloc");
	ptr->ptr = p;
	return 1;
}

static int lmemdb_nullptr(lua_State* L)
{
	ptr_t* ptr = new_ptr(L);
	return 1;
}

static constptr_t* new_constptr(lua_State* L, const void* ptr)
{
	constptr_t* constptr = (constptr_t*)lua_newuserdata(L, sizeof(constptr));
	constptr->ptr = ptr;
	luaL_setmetatable(L, CONSTPTR);
	return constptr;
}

static const char* funcnames[] = {
	[YADSL_MEMDB_FREE] = "free",
	[YADSL_MEMDB_MALLOC] = "malloc",
	[YADSL_MEMDB_REALLOC] = "realloc",
	[YADSL_MEMDB_CALLOC] = "calloc",
};

static int nfields[] = {
	[YADSL_MEMDB_FREE] = 1,
	[YADSL_MEMDB_MALLOC] = 1,
	[YADSL_MEMDB_REALLOC] = 2,
	[YADSL_MEMDB_CALLOC] = 2,
};

static int push_event(lua_State* L, const yadsl_MemDebugEvent* event)
{
	yadsl_MemDebugFunction function = event->function;
	lua_createtable(L, 0, 3 + nfields[function]);
	lua_pushstring(L, event->file);
	lua_setfield(L, -2, "file");
	lua_pushinteger(L, event->line);
	lua_setfield(L, -2, "line");
	lua_pushstring(L, funcnames[function]);
	lua_setfield(L, -2, "func");
	switch (function) {
	case YADSL_MEMDB_FREE:
		new_constptr(L, event->free.ptr);
		lua_setfield(L, -2, "ptr");
		break;
	case YADSL_MEMDB_MALLOC:
		lua_pushinteger(L, (lua_Integer)event->malloc.size);
		lua_setfield(L, -2, "size");
		break;
	case YADSL_MEMDB_REALLOC:
		new_constptr(L, event->realloc.ptr);
		lua_setfield(L, -2, "ptr");
		lua_pushinteger(L, (lua_Integer)event->realloc.size);
		lua_setfield(L, -2, "size");
		break;
	case YADSL_MEMDB_CALLOC:
		lua_pushinteger(L, (lua_Integer)event->calloc.nmemb);
		lua_setfield(L, -2, "nmemb");
		lua_pushinteger(L, (lua_Integer)event->calloc.size);
		lua_setfield(L, -2, "size");
		break;
	}
	return 1;
}

static void generic_listen_cb(const yadsl_MemDebugEvent* event, const void* ptr, yadsl_MemDebugListenerArgument* arg)
{
	lua_State* L = ((pcallarg_t*)arg)->listen_thread;
	if (L != NULL) {
		lua_pushvalue(L, 1);   // f f
		push_event(L, event);  // f f event
		new_constptr(L, ptr);  // f f event ptr
		if (lua_pcall(L, 2, 0, 0)) {
			lua_pop(L, 1);     // f
		}
	}
}

static bool generic_ask_cb(const yadsl_MemDebugEvent* event, yadsl_MemDebugListenerArgument* arg)
{
	lua_State* L = ((pcallarg_t*)arg)->ask_thread;
	if (L != NULL) {
		bool ok;
		lua_pushvalue(L, 1);            // f f
		push_event(L, event);           // f f event
		if (lua_pcall(L, 1, 1, 0)) {
			/* if ask_cb fails, we fail the
			 * allocation (maybe pcall returned
			 * LUA_ERRMEM, meaning that Lua could
			 * not allocate memory for itself...) */
			ok = false;                 // f err
		} else {
			/* if ask_cb succeeds, we check if
			 * the return value is true */
			ok = lua_toboolean(L, -1);  // f ret
		}
		lua_pop(L, 1);                  // f
		return ok;
	} else {
		/* if there is no thread of the ask callback,
		 * it means that no ask_cb was given, so we
		 * don't intervene with memory allocations */
		return true;
	}
}

static int lmemdb_pcall(lua_State* L)
{
	int nargs, status, base, nres;
	bool removed_handle;
	pcallarg_t listener_arg;
	lua_State* listen_thread, *ask_thread;
	yadsl_MemDebugListenerHandle* handle;
	luaL_checktype(L, 1, LUA_TTABLE);
	lua_settop(L, 1);
	lua_rawgeti(L, 1, 1);  // t func
	if (lua_isnil(L, -1)) {
		return luaL_argerror(L, 1, "missing function");
	}
	nargs = 0;
	lua_rawgeti(L, 1, 2);  // t func arg1
	while (!lua_isnil(L, -1)) {
		++nargs;
		luaL_checkstack(L, 1, NULL);
		lua_rawgeti(L, 1, nargs+2);  // t func [argi...] argn
	}
	lua_pop(L, 1);  // t func [args...]
	luaL_checkstack(L, 1, NULL);
	lua_getfield(L, 1, "listen_cb");
	if (lua_isnil(L, -1)) {
		listen_thread = NULL;
		lua_pop(L, 1);                     // t func [args...]
	} else {
		luaL_checkstack(L, 1, NULL);
		listen_thread = lua_newthread(L);  // t func [args...] listen_cb listen_thread
		lua_insert(L, 2);                  // t listen_thread func [args...] listen_cb 
		lua_xmove(L, listen_thread, 1);    // t listen_thread func [args...]
	}
	lua_getfield(L, 1, "ask_cb");
	if (lua_isnil(L, -1)) {
		ask_thread = NULL;
		lua_pop(L, 1);                  // t [listen_thread] func [args...]
	} else {
		luaL_checkstack(L, 1, NULL);
		ask_thread = lua_newthread(L);  // t [listen_thread] func [args...] ask_cb ask_thread
		lua_insert(L, -2);              // t [listen_thread] func [args...] ask_thread ask_cb 
		lua_xmove(L, ask_thread, 1);    // t [listen_thread] func [args...] ask_thread
		lua_insert(L, 2);               // t ask_thread [listen_thread] func [args...]
	}
	listener_arg = (pcallarg_t){
		.listen_thread = listen_thread,
		.ask_thread = ask_thread,
	};
	handle = yadsl_memdb_add_listener(generic_ask_cb, generic_listen_cb, &listener_arg);
	if (handle == NULL) {
		return luaL_error(L, "could not allocate listener");
	}
	base = lua_gettop(L) - nargs - 1;              // t [ask_thread] [listen_thread] func [args...]
	status = lua_pcall(L, nargs, LUA_MULTRET, 0);  // t [ask_thread] [listen_thread] [results...]
	nres = lua_gettop(L) - base;                   //                                <---------->
	removed_handle = yadsl_memdb_remove_listener(handle);
	assert(removed_handle && "removed handle from pcall");
	luaL_checkstack(L, 1, NULL);
	lua_pushboolean(L, status == 0);               // t [ask_thread] [listen_thread] [results...] ok
	lua_insert(L, base + 1);                       // t [ask_thread] [listen_thread] ok [results...]
	return nres + 1;                               //                                <------------->
}

static const struct luaL_Reg memdblib[] = {
		{"malloc", lmemdb_malloc},
		{"calloc", lmemdb_calloc},
		{"nullptr", lmemdb_nullptr},
		{"pcall", lmemdb_pcall},
        {NULL, NULL}  /* sentinel */
};

/* memdb.Pointer methods */

static int lmemdb_free(lua_State* L)
{
	ptr_t* ptr = (ptr_t*)luaL_checkudata(L, 1, PTR);
	const char* filename = luaL_checkstring(L, 2);
	int line = validate_line(L, 3);
	yadsl_memdb_free(ptr->ptr, filename, line);
	ptr->ptr = NULL;
	free(ptr->filename);
	ptr->filename = NULL;
	return 0;
}

static int lmemdb_realloc(lua_State* L)
{
	ptr_t* ptr = (ptr_t*)luaL_checkudata(L, 1, PTR);
	size_t size = validate_size(L, 2);
	char* filename;
	int line = validate_line(L, 4);
	void* p;
	filename = copy_string_at(L, 3);
	if (filename == NULL) return luaL_error(L, "could not copy filename");
	p = yadsl_memdb_realloc(ptr->ptr, size, filename, line);
	if (size != 0 && p == NULL) {
		free(filename);
		return luaL_error(L, "bad realloc");
	}
	ptr->ptr = p;
	free(ptr->filename);
	ptr->filename = filename;
	lua_pushvalue(L, 1);
	return 1;
}

static int lmemdb_asconst(lua_State* L)
{
	ptr_t* ptr = (ptr_t*)luaL_checkudata(L, 1, PTR);
	new_constptr(L, ptr->ptr);
	return 1;
}

static const struct luaL_Reg ptrlib[] = {
        {"free", lmemdb_free},
        {"realloc", lmemdb_realloc},
        {"asconst", lmemdb_asconst},
        {NULL, NULL}  /* sentinel */
};

/* memdb.Pointer metamethods */

static int lmemdb_ptr_gc(lua_State* L)
{
	ptr_t* ptr = (ptr_t*)luaL_checkudata(L, 1, PTR);
	yadsl_memdb_free(ptr->ptr, __FILE__, __LINE__);
	ptr->ptr = NULL;
	free(ptr->filename);
	ptr->filename = NULL;
	return 0;
}

static int lmemdb_ptr_eq(lua_State* L)
{
	ptr_t* ptr1 = (ptr_t*)luaL_checkudata(L, 1, PTR);
	ptr_t* ptr2 = (ptr_t*)luaL_checkudata(L, 2, PTR);
	lua_pushboolean(L, ptr1->ptr == ptr2->ptr);
	return 1;
}

static int lmemdb_ptr_tostring(lua_State* L)
{
	ptr_t* ptr = (ptr_t*)luaL_checkudata(L, 1, PTR);
	lua_pushfstring(L, "Pointer to %p", ptr->ptr);
	return 1;
}

static const struct luaL_Reg ptrmetalib[] = {
		{"__gc", lmemdb_ptr_gc},
		{"__eq", lmemdb_ptr_eq},
		{"__tostring", lmemdb_ptr_tostring},
        {NULL, NULL}  /* sentinel */
};

/* memdb.ConstantPointer equality check */

static int lmemdb_constptr_eq(lua_State* L)
{
	constptr_t* p1 = (constptr_t*)luaL_checkudata(L, 1, CONSTPTR);
	constptr_t* p2 = (constptr_t*)luaL_checkudata(L, 2, CONSTPTR);
	lua_pushboolean(L, p1->ptr == p2->ptr);
	return 1;
}

/* entry point */

#ifdef YADSL_DEBUG
#define DEBUG_VAL 1
#else
#define DEBUG_VAL 0
#endif

int yadsl_memdb_openlib(lua_State* L)
{
	/* register the PTR metatable */
	luaL_newlib(L, ptrmetalib);               // mt
    luaL_newlib(L, ptrlib);                   // mt lib
	lua_setfield(L, -2, "__index");           // 
	lua_setfield(L, LUA_REGISTRYINDEX, PTR);

	/* register the CONSTPTR metatable */
	luaL_newmetatable(L, CONSTPTR);            // mt
	lua_pushcfunction(L, lmemdb_constptr_eq);  // mt eq
	lua_setfield(L, -2, "__eq");               // mt
	lua_pop(L, 1);

	/* create library */
    luaL_newlib(L, memdblib);

	/* register the 'debug' field */
	lua_pushboolean(L, DEBUG_VAL);
	lua_setfield(L, -2, "debug");

    return 1;
}
