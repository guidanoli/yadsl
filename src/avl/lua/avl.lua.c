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
    int lock; /* avoids concurrent access */
}
avltree_udata;

static int avltree_compare_lua_cfunction(lua_State* L)
{
    int result;
    assert(lua_gettop(L) >= 2 && "Called with 2 arguments");
    if (lua_compare(L, 1, 2, LUA_OPEQ))
    {
        result = YADSL_AVLTREE_COMP_EQ;
    }
    else if (lua_compare(L, 1, 2, LUA_OPLT))
    {
        result = YADSL_AVLTREE_COMP_LT;
    }
    else
    {
        result = YADSL_AVLTREE_COMP_GT;
    }
    lua_pushinteger(L, result);
    return 1;
}

static void push_avltree_object(lua_State* L, yadsl_AVLTreeObject* obj)
{
    int* ref_ptr = (int*)obj;
    assert(ref_ptr != NULL && "Reference is valid");
    int ref = *ref_ptr;
    lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
    assert(!lua_isnil(L, -1) && "Reference exists");
}

typedef struct
{
    lua_State* lua_state;
    int error; /* index of the error message */
}
compare_arg_t;

static yadsl_AVLTreeComparison
avltree_compare_cb(yadsl_AVLTreeObject* obj1,
                    yadsl_AVLTreeObject* obj2,
                    yadsl_AVLTreeCmpObjsArg* arg)
{
    compare_arg_t* cmp_arg = (compare_arg_t*)arg;
    lua_State* L = cmp_arg->lua_state;
    assert(L != NULL && "Lua state is valid");
    lua_pushcfunction(L, avltree_compare_lua_cfunction);
    push_avltree_object(L, obj1);
    push_avltree_object(L, obj2);
    if (lua_pcall(L, 2, 1, 0)) {
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);
            lua_pushfstring(L, "comparison failed");
        }
        lua_replace(L, cmp_arg->error);
        return YADSL_AVLTREE_COMP_ERR;
    }
    lua_Integer integer = lua_tointeger(L, -1);
    assert(integer >= INT_MIN && integer <= INT_MAX && "integer in range");
    lua_pop(L, 1);
    return (yadsl_AVLTreeComparison)integer;
}

static void free_avltree_object(lua_State* L, yadsl_AVLTreeObject* obj)
{
    int* ref_ptr = (int*)obj;
    assert(ref_ptr != NULL && "Reference is valid");
    int ref = *ref_ptr;
    free(ref_ptr);
    luaL_unref(L, LUA_REGISTRYINDEX, ref);
}

static void avltree_free_cb(yadsl_AVLTreeObject* obj,
                             yadsl_AVLTreeFreeObjArg* arg)
{
    lua_State* L = (lua_State*)arg;
    assert(L != NULL && "Lua state is valid");
    free_avltree_object(L, obj);
}

static int avltree_new(lua_State* L)
{
    avltree_udata* udata = lua_newuserdata(L, sizeof(avltree_udata));
    udata->avl = NULL;
    udata->lock = 0;
    yadsl_AVLTreeHandle* avl = yadsl_avltree_tree_create(); 
    if (avl == NULL) return luaL_error(L, "bad malloc");
    udata->avl = avl;
    luaL_setmetatable(L, AVLTREE);
    return 1;
}

static avltree_udata* check_avltree_udata(lua_State* L, int arg)
{
    avltree_udata* udata;
    udata = (avltree_udata*)luaL_checkudata(L, arg, AVLTREE);
    if (udata->lock) {
        luaL_error(L, "%s is locked", luaL_tolstring(L, arg, NULL));
        return NULL; /* rever returns */
    }
    return udata;
}

static int avltree_gc(lua_State* L)
{
    avltree_udata* udata = check_avltree_udata(L, 1);
    yadsl_AVLTreeHandle* avl = udata->avl;
    if (avl != NULL) {
        yadsl_AVLTreeCallbacks callbacks = {.free_cb = avltree_free_cb, .free_arg = L};
        udata->avl = NULL;
        yadsl_avltree_destroy(avl, &callbacks);
    }
    return 0;
}

static const struct luaL_Reg avl_lib[] = {
    {AVLTREE, avltree_new},
    {NULL, NULL}  /* sentinel */
};

static void setup_compare_arg(lua_State* L, compare_arg_t* compare_arg)
{
    lua_pushnil(L);
    compare_arg->lua_state = L;
    compare_arg->error = lua_gettop(L);
}

static int avltree_insert(lua_State* L)
{
    avltree_udata* udata = check_avltree_udata(L, 1);
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
    compare_arg_t compare_arg;
    setup_compare_arg(L, &compare_arg);
    yadsl_AVLTreeCallbacks callbacks = {
        .compare_cb = avltree_compare_cb,
        .compare_arg = &compare_arg};
    bool exists;
    udata->lock = 1;
    ret = yadsl_avltree_object_insert(
        udata->avl, ref_ptr, &callbacks, &exists);
    udata->lock = 0;
    if (ret || exists) {
        free(ref_ptr);
        luaL_unref(L, LUA_REGISTRYINDEX, ref);
    }
    switch (ret) {
        case YADSL_AVLTREE_RET_OK:
            break;
        case YADSL_AVLTREE_RET_MEMORY:
            return luaL_error(L, "bad malloc");
        case YADSL_AVLTREE_RET_ERR:
            lua_pushvalue(L, compare_arg.error);
            return lua_error(L);
        default:
            assert(0 && "Unreachable");
    }
    lua_pushboolean(L, exists);
    return 1;
}

static int avltree_search(lua_State* L)
{
    avltree_udata* udata = check_avltree_udata(L, 1);
    lua_settop(L, 2);
    bool exists;
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    if (ref == LUA_REFNIL) {
        exists = false;
    } else {
        yadsl_AVLTreeRet ret;
        assert(udata->avl != NULL && "AVL tree is valid");
        compare_arg_t compare_arg;
        setup_compare_arg(L, &compare_arg);
        yadsl_AVLTreeCallbacks callbacks = {
            .compare_cb = avltree_compare_cb,
            .compare_arg = &compare_arg};
        udata->lock = 1;
        ret = yadsl_avltree_object_search(
            udata->avl, &ref, &callbacks, &exists);
        udata->lock = 0;
        luaL_unref(L, LUA_REGISTRYINDEX, ref);
        switch (ret) {
        case YADSL_AVLTREE_RET_OK:
            break;
        case YADSL_AVLTREE_RET_ERR:
            lua_pushvalue(L, compare_arg.error);
            return lua_error(L);
        default:
            assert(0 && "Unreachable");
        }
    }
    lua_pushboolean(L, exists);
    return 1;
}

static int avltree_remove(lua_State* L)
{
    avltree_udata* udata = check_avltree_udata(L, 1);
    lua_settop(L, 2);
    bool exists;
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    if (ref == LUA_REFNIL) {
        exists = false;
    } else {
        yadsl_AVLTreeRet ret;
        assert(udata->avl != NULL && "AVL tree is valid");
        compare_arg_t compare_arg;
        setup_compare_arg(L, &compare_arg);
        yadsl_AVLTreeCallbacks callbacks = {
            .compare_cb = avltree_compare_cb,
            .compare_arg = &compare_arg,
            .free_cb = avltree_free_cb,
            .free_arg = L};
        udata->lock = 1;
        ret = yadsl_avltree_object_remove(
            udata->avl, &ref, &callbacks, &exists);
        udata->lock = 0;
        luaL_unref(L, LUA_REGISTRYINDEX, ref);
        switch (ret) {
        case YADSL_AVLTREE_RET_OK:
            break;
        case YADSL_AVLTREE_RET_ERR:
            lua_pushvalue(L, compare_arg.error);
            return lua_error(L);
        default:
            assert(0 && "Unreachable");
        }
    }
    lua_pushboolean(L, exists);
    return 1;
}

typedef struct
{
    lua_State* lua_state;
    int error; /* error message */
    int visit_cb; /* visiting callback */
    int visit_ret; /* visiting callback return */
}
visit_arg_t;

static const char* visit_orders[] = {
    "pre", "in", "post"
};

static yadsl_AVLTreeVisitObjRet* avltree_visit_cb(
	yadsl_AVLTreeObject* obj,
	yadsl_AVLTreeVisitObjArg* arg)
{
    visit_arg_t* visit_arg = (visit_arg_t*) arg;
    assert(visit_arg != NULL && "Visit argument is valid");
    lua_State* L = visit_arg->lua_state;
    lua_pushvalue(L, visit_arg->visit_cb);
    push_avltree_object(L, obj);
    if (lua_pcall(L, 1, 1, 0)) {
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);
            lua_pushfstring(L, "traversing callback failed");
        }
        lua_replace(L, visit_arg->error);
        return obj; /* stop iteration */
    }
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return NULL;
    } else {
        lua_replace(L, visit_arg->visit_ret);
        return obj; /* stop iteration */
    }
}

static void setup_visit_arg(lua_State* L, visit_arg_t* visit_arg, int visit_cb)
{
    visit_arg->lua_state = L;
    visit_arg->visit_cb = visit_cb;
    lua_pushnil(L);
    visit_arg->visit_ret = lua_gettop(L);
    lua_pushnil(L);
    visit_arg->error = lua_gettop(L);
}

static void check_visit_error(lua_State* L, visit_arg_t* visit_arg)
{
    int visit_error = visit_arg->error;
    if (!lua_isnil(L, visit_error)) {
        lua_pushvalue(L, visit_error);
        lua_error(L);
    }
}

static int push_visited_object(lua_State* L, visit_arg_t* visit_arg)
{
   int visit_ret = visit_arg->visit_ret;
   if (lua_isnil(L, visit_ret))
       return 0;
   else {
       lua_pushvalue(L, visit_ret);
       return 1;
   }
}

static int avltree_traverse(lua_State* L)
{
    avltree_udata* udata = check_avltree_udata(L, 1);
    luaL_argexpected(L, lua_isfunction(L, 2), 2, "function");
    int order = luaL_checkoption(L, 3, "in", visit_orders);
    lua_settop(L, 2); /* udata visit_cb */
    visit_arg_t visit_arg;
    setup_visit_arg(L, &visit_arg, 2);
    yadsl_AVLTreeCallbacks callbacks = {
        .visit_cb = avltree_visit_cb,
        .visit_arg = &visit_arg};
   yadsl_AVLTreeRet ret;
   udata->lock = 1;
   ret = yadsl_avltree_tree_traverse(udata->avl, order, &callbacks, NULL);
   udata->lock = 0;
   assert(ret == YADSL_AVLTREE_RET_OK && "Cannot fail");
   check_visit_error(L, &visit_arg);
   return push_visited_object(L, &visit_arg);
}

static const struct luaL_Reg avltree_methods[] = {
    {"insert", avltree_insert},
    {"search", avltree_search},
    {"remove", avltree_remove},
    {"traverse", avltree_traverse},
    {NULL, NULL}  /* sentinel */
};

YADSL_EXPORT int luaopen_avl(lua_State* L)
{
    /* register library */
    luaL_newlib(L, avl_lib);

    /* register memdb submodule */
    yadsl_memdb_openlib(L);
    lua_setfield(L, -2, "memdb");

    /* register AVLTree metatable */
    luaL_newmetatable(L, AVLTREE);
    luaL_newlib(L, avltree_methods);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, avltree_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);

    return 1;
}
