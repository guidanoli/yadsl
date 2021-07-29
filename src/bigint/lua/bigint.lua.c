#include "lauxlib.h"

#include <assert.h>

#include <yadsl/dll.h>

#include <bigint/bigint.h>
#include <memdb/lua/memdb.h>

#include <yadsl/stdlib.h>

#define BIGINT "BigInt"

typedef struct
{
    yadsl_BigIntHandle* bigint;
}
bigint_udata;

static int bigint_new(lua_State* L)
{
    yadsl_BigIntHandle* bigint = NULL;
    int type = lua_type(L, 1);
    bigint_udata* udata = lua_newuserdata(L, sizeof(bigint_udata));
    udata->bigint = NULL;
    if (type == LUA_TNUMBER) {
        lua_Integer integer = luaL_checkinteger(L, 1);
        if (integer < INTMAX_MIN || integer > INTMAX_MAX)
            return luaL_error(L, "integer overflow");
        bigint = yadsl_bigint_from_int((intmax_t)integer);
        if (bigint == NULL)
            return luaL_error(L, "bad malloc");
    } else if (type == LUA_TSTRING) {
        size_t size;
        const char* str = lua_tolstring(L, 1, &size);
        yadsl_BigIntStatus status;
        for (size_t i = 0; i < size; ++i)
            if (str[i] == '\0')
                return luaL_argerror(L, 1, "string contains embedded zeros");
        status = yadsl_bigint_from_string(str, &bigint);
        switch (status) {
            case YADSL_BIGINT_STATUS_OK:
                break;
            case YADSL_BIGINT_STATUS_STRING_FORMAT:
                return luaL_argerror(L, 1, "string is ill-formatted");
            case YADSL_BIGINT_STATUS_MEMORY:
                return luaL_error(L, "bad malloc");
            default:
                assert(0 && "Unknown return value");
                return luaL_error(L, "unknown error");
        }
    } else {
        return luaL_argerror(L, 1, "expected string or number");
    }
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
    yadsl_BigIntStatus status = yadsl_bigint_to_int(bigint, &integer);
    if (status != YADSL_BIGINT_STATUS_OK || integer < LUA_MININTEGER || integer > LUA_MAXINTEGER)
        return luaL_error(L, "integer overflow");
    lua_pushinteger(L, (lua_Integer)integer);
    return 1;
}

static int bigint_tostring(lua_State* L)
{
    yadsl_BigIntHandle* bigint = check_bigint(L, 1);
    char* str = yadsl_bigint_to_string(bigint);
    if (str == NULL)
        return luaL_error(L, "bad malloc");
    lua_pushstring(L, str);
    free(str);
    return 1;
}

static const struct luaL_Reg bigint_methods[] = {
    {"to_integer", bigint_tointeger},
    {"to_string", bigint_tostring},
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
