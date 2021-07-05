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

#define ARGVP "ArgumentParser"

typedef struct
{
    yadsl_ArgvParserHandle* argvp;
    char** argv;
    int argc;
}
argvp_udata;

/* Utility functions */

static int check_int(lua_State* L, int i)
{
    lua_Integer integer = luaL_checkinteger(L, i);
    if (integer < INT_MIN || integer > INT_MAX)
        return luaL_error(L, "integer overflow (%I too large for int)", integer);
    return (int)integer;
}

/* Module functions */

static int copy_string(lua_State* L, int s, int i, char** argv)
{
    const char* str = lua_tostring(L, s);
    size_t size = strlen(str)+1;
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
    assert(t > 0);
    lua_Unsigned size = lua_rawlen(L, t);
    if (size > INT_MAX) {
        lua_pushstring(L, "table too large");
        return 1;
    }
    int argc = (int)size;
    char** argv = calloc(argc, sizeof(char*));
    if (argv == NULL) {
        lua_pushstring(L, "bad calloc");
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
        lua_pop(L, 1);
    }
    *argc_ptr = argc;
    *argv_ptr = argv;
    return 0;
}

static int argvp_new(lua_State* L)
{
    yadsl_ArgvParserHandle* argvp;
    int argc;
    char** argv;
    argvp_udata* udata;

    luaL_argexpected(L, lua_istable(L, 1), 1, "table");
    udata = lua_newuserdata(L, sizeof(argvp_udata));
    udata->argvp = NULL;
    udata->argv = NULL;
    luaL_setmetatable(L, ARGVP);
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

static const struct luaL_Reg argvp_lib[] =
{
    {ARGVP, argvp_new},
    {NULL, NULL}  /* sentinel */
};

/* ArgumentParser metatable fields */

static argvp_udata* check_argvp(lua_State* L, int index)
{
    return (argvp_udata*)luaL_checkudata(L, index, ARGVP);
}

static int argvp_gc(lua_State* L)
{
    argvp_udata* udata = check_argvp(L, 1);
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

/* ArgumentParser methods */

static int validate_kw(lua_State* L, int kw)
{
    if (!lua_isstring(L, kw)) {
        lua_pushfstring(L, "expected keyword to be string, not %s", luaL_typename(L, kw));
        return 1;
    }
    return 0;
}

static int validate_valc(lua_State* L, int valc)
{
    if (!lua_isinteger(L, valc)) {
        lua_pushfstring(L, "expected value count to be an integer, not %s", luaL_typename(L, valc));
        return 1;
    }
    lua_Integer integer = lua_tointeger(L, valc);
    if (integer < INT_MIN || integer > INT_MAX) {
        lua_pushstring(L, "integer overflow");
        return 1;
    }
    return 0;
}

static int add_kwarg(lua_State* L, yadsl_ArgvParserHandle* argvp, int kwidx, int valcidx)
{
    const char* kw = lua_tostring(L, kwidx);
    int valc = (int)lua_tointeger(L, valcidx);
    yadsl_argvp_add_keyword_argument(argvp, kw, valc);
    return 0;
}

static int add_kwargs_from_table(lua_State* L, yadsl_ArgvParserHandle* argvp, int t)
{
    assert(t > 0 && "t must be an absolute index");
    lua_pushnil(L);
    while (lua_next(L, t)) {
        if (validate_kw(L, -2)) return 1;
        if (validate_valc(L, -1)) return 1;
        lua_pop(L, 1);
    }
    lua_pushnil(L);
    while (lua_next(L, t)) {
        if (add_kwarg(L, argvp, -2, -1)) return 1;
        lua_pop(L, 1);
    }
    return 0;
}

static int argvp_add_kwargs(lua_State* L)
{
    yadsl_ArgvKeywordArgumentDef* kwargsdef;
    argvp_udata* udata = check_argvp(L, 1);
    luaL_argexpected(L, lua_istable(L, 2), 2, "table");
    if (add_kwargs_from_table(L, udata->argvp, 2))
        return lua_error(L);
    return 0;
}

static int argvp_add_kwarg(lua_State* L)
{
    yadsl_ArgvKeywordArgumentDef* kwargsdef;
    argvp_udata* udata = check_argvp(L, 1);
    if (validate_kw(L, 2) || validate_valc(L, 3) || add_kwarg(L, udata->argvp, 2, 3))
        return lua_error(L);
    return 0;
}

static int argvp_posarg_cnt(lua_State* L)
{
    yadsl_ArgvKeywordArgumentDef* kwargsdef;
    argvp_udata* udata = check_argvp(L, 1);
    int cnt = yadsl_argvp_get_positional_argument_count(udata->argvp);
    lua_pushinteger(L, (lua_Integer)cnt);
    return 1;
}

static int argvp_posarg(lua_State* L)
{
    yadsl_ArgvKeywordArgumentDef* kwargsdef;
    argvp_udata* udata = check_argvp(L, 1);
    int argi = check_int(L, 2);
    const char* posarg = yadsl_argvp_get_positional_argument(udata->argvp, argi-1);
    if (posarg == NULL)
    {
        lua_pushnil(L);
        return 1;
    }
    else
    {
        lua_pushstring(L, posarg);
        return 1;
    }
}

static int argvp_kwarg_val(lua_State* L)
{
    argvp_udata* udata = check_argvp(L, 1);
    const char* kw = luaL_checkstring(L, 2);
    int vali = check_int(L, 3);
    const char* val;
    val = yadsl_argvp_get_keyword_argument_value(udata->argvp, kw, vali-1);
    if (val == NULL)
    {
        lua_pushnil(L);
        return 1;
    }
    else
    {
        lua_pushstring(L, val);
        return 1;
    }
}

static int argvp_has_kwarg(lua_State* L)
{
    argvp_udata* udata = check_argvp(L, 1);
    const char* kw = luaL_checkstring(L, 2);
    int has = yadsl_argvp_has_keyword_argument(udata->argvp, kw);
    lua_pushboolean(L, has);
    return 1;
}

static const struct luaL_Reg argvp_methods[] =
{
    {"addKeywordArguments", argvp_add_kwargs},
    {"addKeywordArgument", argvp_add_kwarg},
    {"getPositionalArgumentCount", argvp_posarg_cnt},
    {"getPositionalArgument", argvp_posarg},
    {"getKeywordArgumentValue", argvp_kwarg_val},
    {"hasKeywordArgument", argvp_has_kwarg},
    {NULL, NULL} /* sentinel */
};

/* Library loader */

YADSL_EXPORT int luaopen_argvp(lua_State* L)
{

    /* register library */
    luaL_newlib(L, argvp_lib);

    /* register parser metatable */
    luaL_newmetatable(L, ARGVP);
    luaL_newlib(L, argvp_methods);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, argvp_gc);
    lua_setfield(L, -2, "__gc");

    lua_pop(L, 1);

    /* register memdb submodule */
    yadsl_memdb_openlib(L);
    lua_setfield(L, -2, "memdb");

    return 1;
}
