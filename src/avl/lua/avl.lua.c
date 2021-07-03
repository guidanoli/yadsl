#include <yadsl/dll.h>

#include "lauxlib.h"

#include <assert.h>

#include <avl/avl.h>
#include <memdb/lua/memdb.h>

#define AVLTREE "AVLTree"
#define CMPFUNC "YADSL_AVL_TREE_COMPARE_FUNCTION"

typedef struct
{
    yadsl_AVLTreeHandle* avl;
}
avl_tree_udata;

static int avl_tree_compare_lua_cfunction(lua_State* L)
{
    int result;
    assert(lua_gettop(L) >= 2 && "Called with 2 arguments");
    if (lua_compare(L, 1, 2, LUA_OPEQ))
    {
        result = 0;
    }
    else if (lua_compare(L, 1, 2, LUA_OPLT))
    {
        result = -1;
    }
    else
    {
        result = 1;
    }
    lua_pushinteger(L, result);
    return 1;
}

static int avl_tree_compare_cb(yadsl_AVLTreeObject* obj1,
                               yadsl_AVLTreeObject* obj2,
                               yadsl_AVLTreeCmpObjsArg* arg)
{
    lua_State* L = (lua_State*)arg;
    assert(L != NULL && "Lua state is valid");
    assert(lua_checkstack(L, 3) && "Slots are pre-allocated");
    lua_pushcfunction(L, avl_tree_compare_lua_cfunction);
    int* ref1 = (int*)obj1;
    assert(ref1 != NULL && "Reference is valid");
    lua_rawgeti(L, LUA_REGISTRYINDEX, *ref1);
    assert(!lua_isnil(L, -1) && "Reference exists");
    int* ref2 = (int*)obj2;
    assert(ref2 != NULL && "Reference is valid");
    lua_rawgeti(L, LUA_REGISTRYINDEX, *ref2);
    assert(!lua_isnil(L, -1) && "Reference exists");
    if (lua_pcall(L, 2, 1, 0)) {
        if (lua_isstring(L, -1))
            fprintf(stderr, "avl_tree_compare_cb: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
        return 0;
    }
    lua_Integer integer = lua_tointeger(L, -1);
    assert(integer >= INT_MIN && integer <= INT_MAX && "integer in range");
    lua_pop(L, 1);
    return (int)integer;
}

static void avl_tree_free_cb(yadsl_AVLTreeObject* obj,
                             yadsl_AVLTreeFreeObjArg* arg)
{
    lua_State* L = (lua_State*)arg;
    assert(L != NULL && "Lua state is valid");
    int* ref = (int*)obj;
    assert(ref != NULL && "Reference is valid");
    luaL_unref(L, LUA_REGISTRYINDEX, *ref);
}

static int avl_tree_constructor(lua_State* L)
{
    avl_tree_udata* udata = lua_newuserdata(L, sizeof(avl_tree_udata));
    udata->avl = NULL;
    yadsl_AVLTreeHandle* avl = yadsl_avltree_tree_create(
        (yadsl_AVLTreeCmpObjsFunc)avl_tree_compare_cb,
        (yadsl_AVLTreeCmpObjsArg*)L,
        (yadsl_AVLTreeFreeObjFunc)avl_tree_free_cb,
        (yadsl_AVLTreeFreeObjArg*)L); 
    if (avl == NULL) return luaL_error(L, "bad malloc");
    udata->avl = avl;
    luaL_setmetatable(L, AVLTREE);
    return 1;
}

#define check_avl_tree_udata(L, arg) \
    ((avl_tree_udata*)luaL_checkudata(L, arg, AVLTREE))

static int avl_tree_destructor(lua_State* L)
{
    avl_tree_udata* udata = check_avl_tree_udata(L, 1);
    yadsl_AVLTreeHandle* avl = udata->avl;
    if (avl != NULL) {
        udata->avl = NULL;
        yadsl_avltree_destroy(avl);
    }
    return 0;
}

static const struct luaL_Reg avllib[] = {
    {"AVLTree", avl_tree_constructor},
    {NULL, NULL}  /* sentinel */
};

static int avl_tree_insert(lua_State* L)
{
    return 0;
}

static int avl_tree_search(lua_State* L)
{
    return 0;
}

static int avl_tree_remove(lua_State* L)
{
    return 0;
}

static int avl_tree_traverse(lua_State* L)
{
    return 0;
}

static const struct luaL_Reg avltreemethods[] = {
    {"insert", avl_tree_insert},
    {"search", avl_tree_search},
    {"remove", avl_tree_remove},
    {"traverse", avl_tree_traverse},
    {NULL, NULL}  /* sentinel */
};

YADSL_EXPORT int luaopen_avl(lua_State* L) {

    /* register library */
    luaL_newlib(L, avllib);

    /* register memdb submodule */
    yadsl_memdb_openlib(L);
    lua_setfield(L, -2, "memdb");

    /* register AVLTree metatable */
    luaL_newmetatable(L, AVLTREE);
    luaL_newlib(L, avltreemethods);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, avl_tree_destructor);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);

    return 1;
}
