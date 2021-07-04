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

static void push_avl_tree_object(lua_State* L, yadsl_AVLTreeObject* obj)
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

static int avl_tree_compare_cb(yadsl_AVLTreeObject* obj1,
                               yadsl_AVLTreeObject* obj2,
                               yadsl_AVLTreeCmpObjsArg* arg)
{
    compare_arg_t* cmp_arg = (compare_arg_t*)arg;
    lua_State* L = cmp_arg->lua_state;
    assert(L != NULL && "Lua state is valid");
    lua_pushcfunction(L, avl_tree_compare_lua_cfunction);
    push_avl_tree_object(L, obj1);
    push_avl_tree_object(L, obj2);
    if (lua_pcall(L, 2, 1, 0)) {
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);
            lua_pushfstring(L, "comparison failed");
        }
        lua_replace(L, cmp_arg->error);
        return 0;
    }
    lua_Integer integer = lua_tointeger(L, -1);
    assert(integer >= INT_MIN && integer <= INT_MAX && "integer in range");
    lua_pop(L, 1);
    return (int)integer;
}

static void free_avl_tree_object(lua_State* L, yadsl_AVLTreeObject* obj)
{
    int* ref_ptr = (int*)obj;
    assert(ref_ptr != NULL && "Reference is valid");
    int ref = *ref_ptr;
    free(ref_ptr);
    luaL_unref(L, LUA_REGISTRYINDEX, ref);
}

static void avl_tree_free_cb(yadsl_AVLTreeObject* obj,
                             yadsl_AVLTreeFreeObjArg* arg)
{
    lua_State* L = (lua_State*)arg;
    assert(L != NULL && "Lua state is valid");
    free_avl_tree_object(L, obj);
}

static int avl_tree_constructor(lua_State* L)
{
    avl_tree_udata* udata = lua_newuserdata(L, sizeof(avl_tree_udata));
    udata->avl = NULL;
    udata->lock = 0;
    yadsl_AVLTreeHandle* avl = yadsl_avltree_tree_create(); 
    if (avl == NULL) return luaL_error(L, "bad malloc");
    udata->avl = avl;
    luaL_setmetatable(L, AVLTREE);
    return 1;
}

static avl_tree_udata* check_avl_tree_udata(lua_State* L, int arg)
{
    avl_tree_udata* udata;
    udata = (avl_tree_udata*)luaL_checkudata(L, arg, AVLTREE);
    if (udata->lock) {
        luaL_error(L, "%s is locked", luaL_tolstring(L, arg, NULL));
        return NULL; /* rever returns */
    }
    return udata;
}

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

static void setup_compare_arg(lua_State* L, compare_arg_t* compare_arg)
{
    lua_pushnil(L);
    compare_arg->lua_state = L;
    compare_arg->error = lua_gettop(L);
}

static void check_compare_error(lua_State* L, compare_arg_t* compare_arg)
{
    if (!lua_isnil(L, compare_arg->error)) {
        lua_pushvalue(L, compare_arg->error);
        lua_error(L);
    }
}

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
    compare_arg_t compare_arg;
    setup_compare_arg(L, &compare_arg);
    yadsl_AVLTreeCallbacks callbacks = {
        .compare_cb = avl_tree_compare_cb,
        .compare_arg = &compare_arg};
    bool exists;
    udata->lock = 1;
    ret = yadsl_avltree_object_insert(
        udata->avl, ref_ptr, &callbacks, &exists);
    udata->lock = 0;
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
    check_compare_error(L, &compare_arg);
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
        compare_arg_t compare_arg;
        setup_compare_arg(L, &compare_arg);
        yadsl_AVLTreeCallbacks callbacks = {
            .compare_cb = avl_tree_compare_cb,
            .compare_arg = &compare_arg};
        udata->lock = 1;
        ret = yadsl_avltree_object_search(
            udata->avl, &ref, &callbacks, &exists);
        udata->lock = 0;
        assert(ret == YADSL_AVLTREE_RET_OK && "Cannot fail");
        luaL_unref(L, LUA_REGISTRYINDEX, ref);
        check_compare_error(L, &compare_arg);
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
        compare_arg_t compare_arg;
        setup_compare_arg(L, &compare_arg);
        yadsl_AVLTreeCallbacks callbacks = {
            .compare_cb = avl_tree_compare_cb,
            .compare_arg = &compare_arg,
            .free_cb = avl_tree_free_cb,
            .free_arg = L};
        udata->lock = 1;
        ret = yadsl_avltree_object_remove(
            udata->avl, &ref, &callbacks, &exists);
        udata->lock = 0;
        assert(ret == YADSL_AVLTREE_RET_OK && "Cannot fail");
        luaL_unref(L, LUA_REGISTRYINDEX, ref);
        check_compare_error(L, &compare_arg);
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

static yadsl_AVLTreeVisitObjRet* avl_tree_visit_cb(
	yadsl_AVLTreeObject* obj,
	yadsl_AVLTreeVisitObjArg* arg)
{
    visit_arg_t* visit_arg = (visit_arg_t*) arg;
    assert(visit_arg != NULL && "Visit argument is valid");
    lua_State* L = visit_arg->lua_state;
    lua_pushvalue(L, visit_arg->visit_cb);
    push_avl_tree_object(L, obj);
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

static int avl_tree_traverse(lua_State* L)
{
    avl_tree_udata* udata = check_avl_tree_udata(L, 1);
    luaL_argexpected(L, lua_isfunction(L, 2), 2, "function");
    int order = luaL_checkoption(L, 3, "in", visit_orders);
    lua_settop(L, 2); /* udata visit_cb */
    visit_arg_t visit_arg;
    setup_visit_arg(L, &visit_arg, 2);
    yadsl_AVLTreeCallbacks callbacks = {
        .visit_cb = avl_tree_visit_cb,
        .visit_arg = &visit_arg};
   udata->lock = 1;
   assert(!yadsl_avltree_tree_traverse(udata->avl, order, &callbacks, NULL));
   udata->lock = 0;
   check_visit_error(L, &visit_arg);
   return push_visited_object(L, &visit_arg);
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
