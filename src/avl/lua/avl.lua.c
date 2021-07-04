#include <yadsl/dll.h>

#include "lauxlib.h"

#include <assert.h>

#ifdef YADSL_DEBUG
#include <memdb/memdb.h>
#else
#include <stdlib.h>
#endif

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
    int* ref_ptr = (int*)obj;
    assert(ref_ptr != NULL && "Reference is valid");
    int ref = *ref_ptr;
    free(ref_ptr);
    luaL_unref(L, LUA_REGISTRYINDEX, ref);
}

static int avl_tree_constructor(lua_State* L)
{
    avl_tree_udata* udata = lua_newuserdata(L, sizeof(avl_tree_udata));
    udata->avl = NULL;
    yadsl_AVLTreeHandle* avl = yadsl_avltree_tree_create(); 
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
        yadsl_AVLTreeCallbacks callbacks = {.free_cb = avl_tree_free_cb, .free_arg = L};
        udata->avl = NULL;
        yadsl_avltree_destroy(avl, &callbacks);
    }
    return 0;
}

static const struct luaL_Reg avllib[] = {
    {"AVLTree", avl_tree_constructor},
    {NULL, NULL}  /* sentinel */
};

static int avl_tree_insert(lua_State* L)
{
    avl_tree_udata* udata = check_avl_tree_udata(L, 1);
    lua_settop(L, 2);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    if (ref == LUA_REFNIL)
        return luaL_argerror(L, 2, "cannot insert nil");
    int* ref_ptr = (int*) malloc(sizeof(int));
    if (ref_ptr == NULL)
        return luaL_error(L, "bad malloc");
    *ref_ptr = ref;
    yadsl_AVLTreeRet ret;
    assert(udata->avl != NULL && "AVL tree is valid");
    yadsl_AVLTreeCallbacks callbacks = {
        .compare_cb = avl_tree_compare_cb,
        .compare_arg = L};
    bool exists;
    ret = yadsl_avltree_object_insert(
        udata->avl, ref_ptr, &callbacks, &exists);
    switch (ret) {
        case YADSL_AVLTREE_RET_OK:
            if (exists) {
                free(ref_ptr);
                luaL_unref(L, LUA_REGISTRYINDEX, ref);
            }
            break;
        case YADSL_AVLTREE_RET_MEMORY:
            free(ref_ptr);
            return luaL_error(L, "bad malloc");
        default:
            assert(0 && "Unreachable");
    }
    lua_pushboolean(L, exists);
    return 1;
}

static int avl_tree_search(lua_State* L)
{
    avl_tree_udata* udata = check_avl_tree_udata(L, 1);
    lua_settop(L, 2);
    bool exists;
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    if (ref == LUA_REFNIL) {
        exists = false;
    } else {
        yadsl_AVLTreeRet ret;
        assert(udata->avl != NULL && "AVL tree is valid");
        yadsl_AVLTreeCallbacks callbacks = {
            .compare_cb = avl_tree_compare_cb,
            .compare_arg = L};
        ret = yadsl_avltree_object_search(
            udata->avl, &ref, &callbacks, &exists);
        assert(ret == YADSL_AVLTREE_RET_OK && "Cannot fail");
        luaL_unref(L, LUA_REGISTRYINDEX, ref);
    }
    lua_pushboolean(L, exists);
    return 1;
}

static int avl_tree_remove(lua_State* L)
{
    avl_tree_udata* udata = check_avl_tree_udata(L, 1);
    lua_settop(L, 2);
    bool exists;
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    if (ref == LUA_REFNIL) {
        exists = false;
    } else {
        yadsl_AVLTreeRet ret;
        assert(udata->avl != NULL && "AVL tree is valid");
        yadsl_AVLTreeCallbacks callbacks = {
            .compare_cb = avl_tree_compare_cb,
            .compare_arg = L,
            .free_cb = avl_tree_free_cb,
            .free_arg = L};
        ret = yadsl_avltree_object_remove(
            udata->avl, &ref, &callbacks, &exists);
        assert(ret == YADSL_AVLTREE_RET_OK && "Cannot fail");
        luaL_unref(L, LUA_REGISTRYINDEX, ref);
    }
    lua_pushboolean(L, exists);
    return 1;
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
