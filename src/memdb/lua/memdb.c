#include <limits.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <memdb/memdb.h>

#include "lauxlib.h"

#define PTR "Pointer"

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
	yadsl_MemDebugListenerHandle* handle; /* null when deleted */
	lua_State* L; /* pointer to exclusive thread used for
	                 callback handling -- null when deleted */
	int accept_cb; /* can be LUA_NOREF -- LUA_NOREF when deleted */
	int ack_cb; /* can be LUA_NOREF -- LUA_NOREF when deleted */
}
listener_t;

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
	if (filename == NULL) return luaL_error(L, "bad malloc");
	ptr->filename = filename;
	p = yadsl_memdb_malloc(size, filename, line);
	if (size != 0 && p == NULL) return luaL_error(L, "bad malloc");
	ptr->ptr = p;
	return 1;
}

static int lmemdb_rawmalloc(lua_State* L)
{
	void* p;
	size_t size = validate_size(L, 1);
	ptr_t* ptr = new_ptr(L);
	p = malloc(size);
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
	if (filename == NULL) return luaL_error(L, "bad malloc");
	ptr->filename = filename;
	p = yadsl_memdb_calloc(nmemb, size, filename, line);
	if (nmemb != 0 && size != 0 && p == NULL) return luaL_error(L, "bad malloc");
	ptr->ptr = p;
	return 1;
}

static int lmemdb_rawcalloc(lua_State* L)
{
	void* p;
	size_t nmemb = validate_size(L, 1);
	size_t size = validate_size(L, 2);
	ptr_t* ptr = new_ptr(L);
	p = calloc(nmemb, size);
	if (nmemb != 0 && size != 0 && p == NULL) return luaL_error(L, "bad malloc");
	ptr->ptr = p;
	return 1;
}

static int lmemdb_nullptr(lua_State* L)
{
	ptr_t* ptr = new_ptr(L);
	return 1;
}

static const struct luaL_Reg memdblib[] = {
		{"malloc", lmemdb_malloc},
		{"rawmalloc", lmemdb_rawmalloc},
		{"calloc", lmemdb_calloc},
		{"rawcalloc", lmemdb_rawcalloc},
		{"nullptr", lmemdb_nullptr},
        {NULL, NULL}  /* sentinel */
};

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

static int lmemdb_rawfree(lua_State* L)
{
	ptr_t* ptr = (ptr_t*)luaL_checkudata(L, 1, PTR);
	free(ptr->ptr);
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
	void* prevp, *p;
	filename = copy_string_at(L, 3);
	if (filename == NULL) return luaL_error(L, "bad malloc");
	prevp = ptr->ptr;
	p = yadsl_memdb_realloc(prevp, size, filename, line);
	if (size != 0 && p == NULL) {
		free(filename);
		lua_pushboolean(L, 0);
	} else {
		ptr->ptr = p;
		free(ptr->filename);
		ptr->filename = filename;
		lua_pushboolean(L, 1);
	}
	return 1;
}

static int lmemdb_rawrealloc(lua_State* L)
{
	ptr_t* ptr = (ptr_t*)luaL_checkudata(L, 1, PTR);
	size_t size = validate_size(L, 2);
	void* prevp = ptr->ptr;
	void* p = realloc(prevp, size);
	if (size != 0 && p == NULL) {
		lua_pushboolean(L, 0);
	} else {
		ptr->ptr = p;
		lua_pushboolean(L, 1);
	}
	return 1;
}

static const struct luaL_Reg ptrlib[] = {
        {"free", lmemdb_free},
        {"rawfree", lmemdb_rawfree},
        {"realloc", lmemdb_realloc},
        {"rawrealloc", lmemdb_rawrealloc},
        {NULL, NULL}  /* sentinel */
};

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

#ifdef YADSL_DEBUG
#define DEBUG_VAL 1
#else
#define DEBUG_VAL 0
#endif

int yadsl_memdb_openlib(lua_State* L)
{
	/* register the PTR metatable */
	luaL_newlib(L, ptrmetalib);
    luaL_newlib(L, ptrlib);
	lua_setfield(L, -2, "__index");
	lua_setfield(L, LUA_REGISTRYINDEX, PTR);

	/* create library */
    luaL_newlib(L, memdblib);

	/* register the 'debug' field */
	lua_pushboolean(L, DEBUG_VAL);
	lua_setfield(L, -2, "debug");

    return 1;
}
