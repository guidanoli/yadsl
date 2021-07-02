#include <yadsl/dll.h>

#include "lauxlib.h"

#include <assert.h>
#include <string.h>

#ifdef YADSL_DEBUG
#include <memdb/memdb.h>
#else
#include <stdlib.h>
#endif

#include <argvp/argvp.h>
#include <memdb/lua/memdb.h>

#define ARGVP_PARSER "ArgumentParser"

struct argvp_udata
{
    yadsl_ArgvParserHandle* argvp;
    char** argv;
    int argc;
};

static int copy_string(lua_State* L, int s, int i, char** argv)
{
    const char* str = lua_tostring(L, s);
    size_t size = (strlen(str)+1)*sizeof(char);
    char* dup = malloc(size);
    if (dup == NULL) return 1;
    argv[i] = memcpy(dup, str, size);
    return 0;
}

static void free_argv_until(char** argv, int i)
{
    for (int j = 0; j < i; ++j)
        free(argv[j]);
    free(argv);
}

static int arg_table_copy(lua_State* L, int t, int* argc_ptr, char*** argv_ptr)
{
    lua_Unsigned size = lua_rawlen(L, t);
    if (size > INT_MAX) {
        lua_pushstring(L, "table too large");
        return 1;
    }
    int argc = (int)size;
    char** argv = malloc(argc*sizeof(char*));
    if (argv == NULL) {
        lua_pushstring(L, "bad malloc");
        return 1;
    }
    for (int i = 0; i < argc; ++i) {
        lua_rawgeti(L, t, i+1);
        if (!lua_isstring(L, -1)) {
            free_argv_until(argv, i);
            lua_pushfstring(L, "value at index %d is not a string", i+1);
            return 1;
        }
        if (copy_string(L, -1, i, argv)) {
            free_argv_until(argv, i);
            lua_pushfstring(L, "could not copy string at index %d", i+1);
            return 1;
        }
    }
    *argc_ptr = argc;
    *argv_ptr = argv;
    return 0;
}

static int argvp_constructor(lua_State* L)
{
    yadsl_ArgvParserHandle* argvp;
    int argc;
    char** argv;
    struct argvp_udata* udata;

    luaL_argexpected(L, lua_istable(L, 1), 1, "table");
    udata = lua_newuserdata(L, sizeof(struct argvp_udata));
    udata->argvp = NULL;
    udata->argv = NULL;
    luaL_setmetatable(L, ARGVP_PARSER);
    if (arg_table_copy(L, 1, &argc, &argv))
        return lua_error(L);
    argvp = yadsl_argvp_create(argc, argv);
    if (argvp == NULL) {
        free_argv_until(argv, argc);
        return luaL_error(L, "could not allocate parser");
    }
    udata->argvp = argvp;
    udata->argv = argv;
    udata->argc = argc;
    return 1;
}

static const struct luaL_Reg argvplib[] =
{
    {"ArgumentParser", argvp_constructor},
    {NULL, NULL}  /* sentinel */
};

static int argvp_finalizer(lua_State* L)
{
    struct argvp_udata* udata;
    udata = (struct argvp_udata*)luaL_checkudata(L, 1, ARGVP_PARSER);
    if (udata->argv != NULL) {
        free_argv_until(udata->argv, udata->argc);
        udata->argv = NULL;
    }
    if (udata->argvp != NULL) {
        yadsl_argvp_destroy(udata->argvp);
        udata->argv = NULL;
    }
    return 0;
}

YADSL_EXPORT int luaopen_argvp(lua_State* L)
{

    /* register library */
    luaL_newlib(L, argvplib);

    /* register parser metatable */
    luaL_newmetatable(L, ARGVP_PARSER);
    lua_pushcfunction(L, argvp_finalizer);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);

    /* register memdb submodule */
    yadsl_memdb_openlib(L);
    lua_setfield(L, -2, "memdb");

    return 1;
}
