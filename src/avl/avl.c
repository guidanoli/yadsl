#include <avl/avl.h>

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include <memdb/memdb.h>

/**** Macros ****/

#ifndef abs
#define abs(a) (a > 0 ? a : -a)
#endif /* abs */

#ifndef max
#define max(a, b) ((a > b) ? a : b)
#endif /* max */

/* normalized object comparison function */
#define compare_objects(o1, o2, t) \
(t->cmp_objs_func ? t->cmp_objs_func(o1, o2, t->cmp_objs_arg) : (o1 < o2 ? -1 : (o1 == o2 ? 0 : 1)))

/* height of a node */
#define height(x) (x ? x->height : 0)

/* balance of a node */
#define balance(x) (height(x->right) - height(x->left))

/* update node height */
#define update_height(x) x->height = 1 + max(height(x->left), height(x->right))

/* check AVL tree invariants  */
#ifdef _DEBUG
#define check_invariants(x) do { \
    if (x != NULL) { \
        assert(x != x->left); \
        assert(x != x->right); \
        if (x->left != NULL && x->right != NULL) \
            assert(x->left != x->right); \
        assert(height(x) == 1 + max(height(x->left), height(x->right))); \
        assert(abs(balance(x)) < 2); \
    } \
} while(0)
#else /* ifndef _DEBUG */
#define check_invariants(x) ((void) 0)
#endif /* _DEBUG */

/**** Structs ****/

/** AVL subtree */
typedef struct yadsl_AVLSubtree
{
    struct yadsl_AVLSubtree *left;  /**< Left AVL subtree */
    struct yadsl_AVLSubtree *right; /**< Right AVL subtree */
    int height;                     /**< AVL subtree height */
    yadsl_AVLTreeObject *object;    /**< Subtree root object */
}
yadsl_AVLSubtree;

/** AVL tree */
typedef struct
{
    yadsl_AVLSubtree *root;                 /**< AVL tree root */
    yadsl_AVLTreeCmpObjsFunc cmp_objs_func; /**< AVL tree object comparison function */
    yadsl_AVLTreeCmpObjsArg *cmp_objs_arg;  /**< AVL tree object comparison function user argument */
    yadsl_AVLTreeFreeObjFunc free_obj_func; /**< AVL tree object freeing function */
}
yadsl_AVLTree;

/**** Internal functions declarations ****/

static yadsl_AVLSubtree* yadsl_avltree_node_create_internal(yadsl_AVLTreeObject* object);

static yadsl_AVLSubtree* yadsl_avltree_subtree_left_rotate_internal(yadsl_AVLSubtree* x);

static yadsl_AVLSubtree* yadsl_avltree_subtree_right_rotate_internal(yadsl_AVLSubtree* x);

static yadsl_AVLSubtree* yadsl_avl_subtree_rebalance_internal(yadsl_AVLSubtree* x);

static yadsl_AVLSubtree* yadsl_avltree_subtree_node_insert_internal(
    yadsl_AVLTree* tree,
    yadsl_AVLSubtree* x,
    yadsl_AVLSubtree* node,
    bool* has_duplicate_ptr);

static bool yadsl_avltree_subtree_node_search_internal(
    yadsl_AVLTree* tree,
    yadsl_AVLTreeObject* object,
    yadsl_AVLSubtree* node);

static yadsl_AVLTreeVisitObjRet* yadsl_avltree_subtree_traverse_post_internal(
    yadsl_AVLSubtree* node,
    yadsl_AVLTreeVisitObjFunc visit_func,
    yadsl_AVLTreeVisitObjArg* visit_arg);

static yadsl_AVLTreeVisitObjRet* yadsl_avltree_subtree_traverse_in_internal(
    yadsl_AVLSubtree* node,
    yadsl_AVLTreeVisitObjFunc visit_func,
    yadsl_AVLTreeVisitObjArg* visit_arg);

static yadsl_AVLTreeVisitObjRet* yadsl_avltree_subtree_traverse_pre_internal(
    yadsl_AVLSubtree* node,
    yadsl_AVLTreeVisitObjFunc visit_func,
    yadsl_AVLTreeVisitObjArg* visit_arg);

static yadsl_AVLSubtree* yadsl_avltree_subtree_get_min_node_internal(yadsl_AVLSubtree* node);

static yadsl_AVLSubtree* yadsl_avltree_subtree_node_remove_internal(
    yadsl_AVLTree* tree,
    yadsl_AVLTreeObject* object,
    yadsl_AVLSubtree* x,
    bool free_obj /* = true */,
    bool* exists_ptr);

static void yadsl_avltree_subtree_destroy_internal(
    yadsl_AVLTree* tree,
    yadsl_AVLSubtree * x);

/**** External functions definitions ****/

yadsl_AVLTreeHandle* yadsl_avltree_tree_create(
    yadsl_AVLTreeCmpObjsFunc cmp_objs_func,
    yadsl_AVLTreeCmpObjsArg *cmp_objs_arg,
    yadsl_AVLTreeFreeObjFunc free_obj_func)
{
    yadsl_AVLTree *tree = malloc(sizeof *tree);
    if (tree) {
        tree->root = NULL;
        tree->cmp_objs_func = cmp_objs_func;
        tree->free_obj_func = free_obj_func;
        tree->cmp_objs_arg = cmp_objs_arg;
    }
    return tree;
}

yadsl_AVLTreeRet yadsl_avltree_object_insert(
    yadsl_AVLTreeHandle* tree_handle,
    yadsl_AVLTreeObject* object,
    bool* exists_ptr)
{
    bool exists = false;
    yadsl_AVLSubtree *node = yadsl_avltree_node_create_internal(object);
    if (node == NULL)
        return YADSL_AVLTREE_RET_MEMORY;
    yadsl_AVLTree* tree = (yadsl_AVLTree*) tree_handle;
    tree->root = yadsl_avltree_subtree_node_insert_internal(tree, tree->root, node, &exists);
    if (exists)
        free(node);
    if (exists_ptr)
        *exists_ptr = exists;
    return YADSL_AVLTREE_RET_OK;
}

yadsl_AVLTreeRet yadsl_avltree_object_search(
    yadsl_AVLTreeHandle *tree_handle,
    yadsl_AVLTreeObject *object,
    bool *exists_ptr)
{
    yadsl_AVLTree* tree = (yadsl_AVLTree*) tree_handle;
    bool exists = yadsl_avltree_subtree_node_search_internal(tree, object, tree->root);
    if (exists_ptr)
        *exists_ptr = exists;
    return YADSL_AVLTREE_RET_OK;
}

yadsl_AVLTreeRet yadsl_avltree_tree_traverse(
    yadsl_AVLTreeHandle *tree_handle,
    yadsl_AVLTreeVisitingOrder visit_order,
    yadsl_AVLTreeVisitObjFunc visit_func,
    yadsl_AVLTreeVisitObjArg *visit_arg,
    yadsl_AVLTreeVisitObjRet **visit_ret_ptr)
{
    yadsl_AVLTree* tree = (yadsl_AVLTree*) tree_handle;
    yadsl_AVLTreeVisitObjRet* ret;
    switch (visit_order) {
    case YADSL_AVLTREE_VISITING_PRE_ORDER:
        ret = yadsl_avltree_subtree_traverse_pre_internal(tree->root, visit_func, visit_arg);
        break;
    case YADSL_AVLTREE_VISITING_IN_ORDER:
        ret = yadsl_avltree_subtree_traverse_in_internal(tree->root, visit_func, visit_arg);
        break;
    case YADSL_AVLTREE_VISITING_POST_ORDER:
        ret = yadsl_avltree_subtree_traverse_post_internal(tree->root, visit_func, visit_arg);
        break;
    default:
        return YADSL_AVLTREE_RET_PARAM;
    }
    if (visit_ret_ptr)
        *visit_ret_ptr = ret;
    return YADSL_AVLTREE_RET_OK;
}

yadsl_AVLTreeRet yadsl_avltree_object_remove(
    yadsl_AVLTreeHandle *tree_handle,
    yadsl_AVLTreeObject *object,
    bool *exists_ptr)
{
    if (exists_ptr)
        *exists_ptr = false;
    yadsl_AVLTree* tree = (yadsl_AVLTree*) tree_handle;
    tree->root = yadsl_avltree_subtree_node_remove_internal(tree, object, tree->root, true, exists_ptr);
    return YADSL_AVLTREE_RET_OK;
}

void yadsl_avltree_destroy(yadsl_AVLTreeHandle *tree_handle)
{
    if (tree_handle == NULL)
        return;
    yadsl_AVLTree* tree = (yadsl_AVLTree*) tree_handle;
    yadsl_avltree_subtree_destroy_internal(tree, tree->root);
    free(tree);
}

/**** Internal functions definitions ****/

/**
 * @brief Create leaf node with object
 * @param object - object to be stored in node
 * @return pointer to node or NULL, if could nto allocate memory
*/
yadsl_AVLSubtree *yadsl_avltree_node_create_internal(yadsl_AVLTreeObject *object)
{
    yadsl_AVLSubtree *node = malloc(sizeof *node);
    if (node) {
        node->height = 1;
        node->left = NULL;
        node->right = NULL;
        node->object = object;
    }
    return node;
}

/**
 * @brief Left-rotate a subtree around its root
 * @param x - subtree root before rotation
 * @attention assumes x->left != NULL
 * @return subtree root after rotation
*/
yadsl_AVLSubtree *yadsl_avltree_subtree_left_rotate_internal(yadsl_AVLSubtree *x)
{
    yadsl_AVLSubtree *y = x->right;
    yadsl_AVLSubtree *T2 = y->left;
    /**
     *       |                   |
     *       x                   y
     *      / \                 / \
     *    T1   y      ==>      x   T3
     *        / \             / \
     *      T2   T3         T1   T2
    */
    x->right = T2;
    y->left = x;
    update_height(x);
    update_height(y);
    return y;
}

/**
 * @brief Right-rotate a subtree around its root
 * @param x subtree root before rotation
 * @attention assumes x->right != NULL
 * @return subtree root after rotation
*/
yadsl_AVLSubtree *yadsl_avltree_subtree_right_rotate_internal(yadsl_AVLSubtree *x)
{
    yadsl_AVLSubtree *y = x->left;
    yadsl_AVLSubtree *T2 = y->right;
    /**
     *        |                   |
     *        x                   y
     *       / \                 / \
     *      y   T3      ==>    T1   x
     *     / \                     / \
     *   T1   T2                 T2   T3
    */
    x->left = T2;
    y->right = x;
    update_height(x);
    update_height(y);
    return y;
}

/**
 * @brief Balance subtree
 * @param x subtree root before balancing
 * @return subtree root after balancing
*/
yadsl_AVLSubtree *yadsl_avl_subtree_rebalance_internal(yadsl_AVLSubtree *x)
{
    int balance;
    if (x == NULL)
        return NULL;
    x->height = 1 + max(height(x->left), height(x->right));
    balance = balance(x);
    if (balance < -1) {
        /**
         *    |
         *    x
         *   / \
         * T1*  T2 
         */
        assert(x->left);
        balance = balance(x->left);
        if (balance <= 0) {
            /**
             *      |
             *      x
             *     / \
             *    y   T3
             *   / \
             * T1*  T2
             */
            assert(x->left->left);
            return yadsl_avltree_subtree_right_rotate_internal(x);
        } else {
            /**
             *      |
             *      x
             *     / \
             *    y   T3
             *   / \
             * T1   T2*
             */
            assert(x->left->right);
            x->left = yadsl_avltree_subtree_left_rotate_internal(x->left);
            return yadsl_avltree_subtree_right_rotate_internal(x);
        }
    } else if (balance > 1) {
        /**
         *    |
         *    x
         *   / \
         * T1   T2*
         */
        assert(x->right);
        balance = balance(x->right);
        if (balance >= 0) {
            /**
             *    |
             *    x
             *   / \
             * T1   y
             *     / \
             *   T2   T3*
             */
            assert(x->right->right);
            return yadsl_avltree_subtree_left_rotate_internal(x);
        } else {
            /**
             *    |
             *    x
             *   / \
             * T1   y
             *     / \
             *   T2*  T3
             */
            assert(x->right->left);
            x->right = yadsl_avltree_subtree_right_rotate_internal(x->right);
            return yadsl_avltree_subtree_left_rotate_internal(x);
        }
    }
    check_invariants(x);
    return x;
}

/**
 * @brief Insert node in subtree
 * @param tree tree where node will be inserted
 * @param x subtree root before insertion
 * @param node node to be inserted
 * @param has_duplicate_ptr whether subtree already has a duplicate
 * @return subtree root after insertion
*/
yadsl_AVLSubtree *yadsl_avltree_subtree_node_insert_internal(
    yadsl_AVLTree *tree,
    yadsl_AVLSubtree *x,
    yadsl_AVLSubtree *node,
    bool *has_duplicate_ptr)
{
    int cmp;
    if (x == NULL)
        return node;
    cmp = compare_objects(node->object, x->object, tree);
    if (cmp < 0) {
        /**
         *    |
         *    x
         *   / \
         * T1*  T2
         */
        x->left = yadsl_avltree_subtree_node_insert_internal(tree, x->left, node, has_duplicate_ptr);
    } else if (cmp > 0) {
        /**
         *    |
         *    x
         *   / \
         * T1   T2*
         */
        x->right = yadsl_avltree_subtree_node_insert_internal(tree, x->right, node, has_duplicate_ptr);
    } else {
        /* Duplicate object */
        *has_duplicate_ptr = true;
    }
    if (*has_duplicate_ptr)
        return x;
    return yadsl_avl_subtree_rebalance_internal(x);
}

/**
 * @brief Search for node containing object in subtree
 * @param tree tree from whose nodes the object will be searched
 * @param object object to be searched
 * @param node subtree root
 * @return whether node was found or not
*/
bool yadsl_avltree_subtree_node_search_internal(
    yadsl_AVLTree *tree,
    yadsl_AVLTreeObject *object,
    yadsl_AVLSubtree *node)
{
    int cmp;
    if (node == NULL)
        return false;
    check_invariants(node);
    cmp = compare_objects(object, node->object, tree);
    if (cmp < 0)
        return yadsl_avltree_subtree_node_search_internal(tree, object, node->left);
    else if (cmp > 0)
        return yadsl_avltree_subtree_node_search_internal(tree, object, node->right);
    else
        return true;
}

/**
 * @brief Remove node containing object from subtree
 * @param tree tree from where node will be removed
 * @param object object to be deleted
 * @param x subtree root before removal
 * @param free_obj whether to free object or not (Hint: unles you know what
 * you are doing, pass true).
 * @param exists_ptr whether node existed or not
 * @return subtree root after removal
*/
yadsl_AVLSubtree *yadsl_avltree_subtree_node_remove_internal(
    yadsl_AVLTree *tree,
    yadsl_AVLTreeObject *object,
    yadsl_AVLSubtree *x,
    bool free_obj /* = true */,
    bool *exists_ptr)
{
    if (x == NULL)
        return NULL;
    int cmp = compare_objects(object, x->object, tree);
    if (cmp < 0) {
        /**
         *    |
         *    x
         *   / \
         * T1*  T2
         */
        x->left = yadsl_avltree_subtree_node_remove_internal(tree, object, x->left, free_obj, exists_ptr);
    } else if (cmp > 0) {
        /**
         *    |
         *    x
         *   / \
         * T1   T2*
         */
        x->right = yadsl_avltree_subtree_node_remove_internal(tree, object, x->right, free_obj, exists_ptr);
    } else {
        yadsl_AVLSubtree *retnode;
        if (exists_ptr)
            *exists_ptr = true;
        if (x->left == NULL && x->right == NULL) {
            /**
             *   |            |
             *   x    ==>    NULL
             *
             */
            retnode = NULL;
        } else if (x->left == NULL) {
            /**
             *   |            |
             *   x            o
             *    \   ==>   
             *     o
             */
            retnode = x->right;
        } else if (x->right == NULL) {
            /**
             *   |            |
             *   x    ==>     o
             *  /
             * o
             */
            retnode = x->left;
        } else {
            /**
             *    |           |
             *    x   ==>   min(T2)
             *   / \         / \
             * T1   T2     T1   T2-min(T2)
             */
            retnode = yadsl_avltree_subtree_get_min_node_internal(x->right);
            retnode->right = yadsl_avltree_subtree_node_remove_internal(tree, retnode->object, x->right, false, exists_ptr);
            retnode->left = x->left;
        }
        if (free_obj) {
            if (tree->free_obj_func)
                tree->free_obj_func(x->object);
            free(x);
        }
        x = retnode;
    }
    return yadsl_avl_subtree_rebalance_internal(x);
}

/**
 * @brief Traverse subtree pre-order
 * @param node subtree root
 * @param visit_func visitation function
 * @param visit_arg argument passed to visit_func
 * @return first value returned by visit_func, or 0 if it was never called
*/
yadsl_AVLTreeVisitObjRet* yadsl_avltree_subtree_traverse_pre_internal(
    yadsl_AVLSubtree* node,
    yadsl_AVLTreeVisitObjFunc visit_func,
    yadsl_AVLTreeVisitObjArg* visit_arg)
{
    yadsl_AVLTreeVisitObjRet* ret;
    if (node == NULL)
        return NULL;
    check_invariants(node);
    if (visit_func && (ret = visit_func(node->object, visit_arg)))
        return ret;
    if (ret = yadsl_avltree_subtree_traverse_in_internal(node->left, visit_func, visit_arg))
        return ret;
    if (ret = yadsl_avltree_subtree_traverse_in_internal(node->right, visit_func, visit_arg))
        return ret;
    return NULL;
}

/**
 * @brief Traverse subtree in-order
 * @param node subtree root
 * @param visit_func visitation function
 * @param visit_arg argument passed to visit_func
 * @return first value returned by visit_func, or 0 if it was never called
*/
yadsl_AVLTreeVisitObjRet *yadsl_avltree_subtree_traverse_in_internal(
    yadsl_AVLSubtree *node,
    yadsl_AVLTreeVisitObjFunc visit_func,
    yadsl_AVLTreeVisitObjArg *visit_arg)
{
    yadsl_AVLTreeVisitObjRet *ret;
    if (node == NULL)
        return NULL;
    if (ret = yadsl_avltree_subtree_traverse_in_internal(node->left, visit_func, visit_arg))
        return ret;
    check_invariants(node);
    if (visit_func && (ret = visit_func(node->object, visit_arg)))
        return ret;
    if (ret = yadsl_avltree_subtree_traverse_in_internal(node->right, visit_func, visit_arg))
        return ret;
    return NULL;
}

/**
 * @brief Traverse subtree post-order
 * @param node subtree root
 * @param visit_func visitation function
 * @param visit_arg argument passed to visit_func
 * @return first value returned by visit_func, or 0 if it was never called
*/
yadsl_AVLTreeVisitObjRet* yadsl_avltree_subtree_traverse_post_internal(
    yadsl_AVLSubtree* node,
    yadsl_AVLTreeVisitObjFunc visit_func,
    yadsl_AVLTreeVisitObjArg* visit_arg)
{
    yadsl_AVLTreeVisitObjRet* ret;
    if (node == NULL)
        return NULL;
    if (ret = yadsl_avltree_subtree_traverse_in_internal(node->left, visit_func, visit_arg))
        return ret;
    if (ret = yadsl_avltree_subtree_traverse_in_internal(node->right, visit_func, visit_arg))
        return ret;
    check_invariants(node);
    if (visit_func && (ret = visit_func(node->object, visit_arg)))
        return ret;
    return NULL;
}

/**
 * @brief Get subtree minimum node
 * @param node subtree root
 * @attention assumes node != NULL
 * @return subtree minimum node
*/
yadsl_AVLSubtree *yadsl_avltree_subtree_get_min_node_internal(yadsl_AVLSubtree *node)
{
    for (; node->left; node = node->left);
    return node;
}

/**
 * @brief Destroy subtree
 * @param tree tree containing x
 * @param x subtree root
*/
void yadsl_avltree_subtree_destroy_internal(
    yadsl_AVLTree *tree,
    yadsl_AVLSubtree *x)
{
    yadsl_AVLSubtree *left, *right;
    if (x == NULL)
        return;
    left = x->left;
    right = x->right;
    if (tree->free_obj_func)
        tree->free_obj_func(x->object);
    free(x);
    yadsl_avltree_subtree_destroy_internal(tree, left);
    yadsl_avltree_subtree_destroy_internal(tree, right);
}
