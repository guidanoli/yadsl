#include "lauxlib.h"

#include <assert.h>

#include <yadsl/dll.h>

#include <bigint/bigint.h>
#include <memdb/lua/memdb.h>

#define BIGINT "BigInt"

typedef struct
{
    yadsl_BigIntHandle* bigint;
}
bigint_udata;

static int bigint_new(lua_State* L)
{
    lua_Integer integer = luaL_checkinteger(L, 1);
    if (integer < INTMAX_MIN || integer > INTMAX_MAX)
        return luaL_error(L, "integer overflow");
    lua_settop(L, 1);
    bigint_udata* udata = lua_newuserdata(L, sizeof(bigint_udata));
    udata->bigint = NULL;
    yadsl_BigIntHandle* bigint = yadsl_bigint_from_int((intmax_t)integer);
    if (bigint == NULL) return luaL_error(L, "bad malloc");
    udata->bigint = bigint;
    luaL_setmetatable(L, BIGINT);
    return 1;
}

static const struct luaL_Reg bigint_lib[] = {
    {BIGINT, bigint_new},
    {NULL, NULL}  /* sentinel */
};

static bigint_udata* check_bigint_udata(lua_State* L, int arg)
{
    bigint_udata* udata;
    udata = (bigint_udata*)luaL_checkudata(L, arg, BIGINT);
    assert(udata != NULL && "Userdata is valid");
    return udata;
}

static int bigint_gc(lua_State* L)
{
    bigint_udata* udata = check_bigint_udata(L, 1);
    yadsl_BigIntHandle* bigint = udata->bigint;
    if (bigint != NULL) {
        udata->bigint = NULL;
        yadsl_bigint_destroy(bigint);
    }
}

static yadsl_BigIntHandle* check_bigint(lua_State* L, int arg)
{
    bigint_udata* udata;
    yadsl_BigIntHandle* bigint;
    udata = check_bigint_udata(L, arg);
    bigint = udata->bigint;
    assert(yadsl_bigint_check(bigint) == YADSL_BIGINT_STATUS_OK && "BigInt is valid");
    return bigint;
}

static int bigint_tointeger(lua_State* L)
{
    yadsl_BigIntHandle* bigint = check_bigint(L, 1);
    intmax_t integer;
    bool ok = yadsl_bigint_to_int(bigint, &integer);
    if (!ok || integer < LUA_MININTEGER || integer > LUA_MAXINTEGER)
        return luaL_error(L, "integer overflow");
    lua_pushinteger(L, (lua_Integer)integer);
    return 1;
}

static const struct luaL_Reg bigint_methods[] = {
    {"to_integer", bigint_tointeger},
    {NULL, NULL}  /* sentinel */
};

YADSL_EXPORT int luaopen_bigint(lua_State* L)
{
    /* register library */
    luaL_newlib(L, bigint_lib);

    /* register memdb submodule */
    yadsl_memdb_openlib(L);
    lua_setfield(L, -2, "memdb");

    /* register BigInt metatable */
    luaL_newmetatable(L, BIGINT);
    lua_pushcfunction(L, bigint_gc);
    lua_setfield(L, -2, "__gc");
    luaL_newlib(L, bigint_methods);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);

    return 1;
}
