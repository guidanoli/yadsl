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

typedef struct AVLTreeNode
{
    /* Left AVL tree node */
    struct AVLTreeNode *left;

    /* Right AVL tree node */
    struct AVLTreeNode *right;

    /* AVL tree node height */
    int height;

    /* AVL tree node object */
    yadsl_AVLTreeObject *object;
}
/* AVL tree node */
AVLTreeNode;

typedef struct
{
    /* AVL tree root node */
    AVLTreeNode *root;

    /* AVL tree object comparison function */
    yadsl_AVLTreeCmpObjsFunc cmp_objs_func;

    /* AVL tree object comparison function user argument */
    yadsl_AVLTreeCmpObjsArg *cmp_objs_arg;

    /* AVL tree object freeing function */
    yadsl_AVLTreeFreeObjFunc free_object_func;
}
/* AVL tree */
AVLTree;

/**** Internal functions declarations ****/

static AVLTreeNode* yadsl_avltree_node_create_internal(yadsl_AVLTreeObject* object);

static AVLTreeNode* yadsl_avltree_subtree_left_rotate_internal(AVLTreeNode* x);

static AVLTreeNode* yadsl_avltree_subtree_right_rotate_internal(AVLTreeNode* x);

static AVLTreeNode* yadsl_avl_subtree_rebalance_internal(AVLTreeNode* x);

static AVLTreeNode* yadsl_avltree_subtree_node_insert_internal(
    AVLTree* tree,
    AVLTreeNode* x,
    AVLTreeNode* node,
    int* has_duplicate_ptr);

static int yadsl_avltree_subtree_node_search_internal(
    AVLTree* tree,
    yadsl_AVLTreeObject* object,
    AVLTreeNode* node);

static yadsl_AVLTreeVisitObjRet* yadsl_avltree_subtree_traverse_internal(
    AVLTreeNode* node,
    yadsl_AVLTreeVisitObjFunc visit_func,
    yadsl_AVLTreeVisitObjArg* visit_arg);

static AVLTreeNode* yadsl_avltree_subtree_get_min_node_internal(AVLTreeNode* node);

static AVLTreeNode* yadsl_avltree_subtree_node_remove_internal(
    AVLTree* tree,
    yadsl_AVLTreeObject* object,
    AVLTreeNode* x,
    int free_obj /* = 1 */,
    int* exists_ptr);

static void yadsl_avltree_subtree_destroy_internal(
    AVLTree* tree,
    AVLTreeNode * x);

/**** External functions definitions ****/

yadsl_AVLTreeRet yadsl_avltree_tree_create(
    yadsl_AVLTreeCmpObjsFunc cmp_objs_func,
    yadsl_AVLTreeCmpObjsArg *cmp_objs_arg,
    yadsl_AVLTreeFreeObjFunc free_object_func,
    yadsl_AVLTreeHandle **tree_handle_ptr)
{
    AVLTree *tree = malloc(sizeof *tree);
    if (tree == NULL)
        return YADSL_AVLTREE_RET_MEMORY;
    tree->root = NULL;
    tree->cmp_objs_func = cmp_objs_func;
    tree->free_object_func = free_object_func;
    tree->cmp_objs_arg = cmp_objs_arg;
    *tree_handle_ptr = tree;
    return YADSL_AVLTREE_RET_OK;
}

yadsl_AVLTreeRet yadsl_avltree_object_insert(
    yadsl_AVLTreeHandle* tree_handle,
    yadsl_AVLTreeObject* object,
    int* exists_ptr)
{
    int exists = 0;
    AVLTreeNode *node = yadsl_avltree_node_create_internal(object);
    if (node == NULL)
        return YADSL_AVLTREE_RET_MEMORY;
    AVLTree* tree = (AVLTree*) tree_handle;
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
    int *exists_ptr)
{
    AVLTree* tree = (AVLTree*) tree_handle;
    int exists = yadsl_avltree_subtree_node_search_internal(tree, object, tree->root);
    if (exists_ptr)
        *exists_ptr = exists;
    return YADSL_AVLTREE_RET_OK;
}

yadsl_AVLTreeRet yadsl_avltree_tree_traverse(
    yadsl_AVLTreeHandle *tree_handle,
    yadsl_AVLTreeVisitObjFunc visit_func,
    yadsl_AVLTreeVisitObjArg *visit_arg,
    yadsl_AVLTreeVisitObjRet **visit_ret_ptr)
{
    AVLTree* tree = (AVLTree*) tree_handle;
    yadsl_AVLTreeVisitObjRet *ret = yadsl_avltree_subtree_traverse_internal(tree->root, visit_func, visit_arg);
    if (visit_ret_ptr)
        *visit_ret_ptr = ret;
    return YADSL_AVLTREE_RET_OK;
}

yadsl_AVLTreeRet yadsl_avltree_object_remove(
    yadsl_AVLTreeHandle *tree_handle,
    yadsl_AVLTreeObject *object,
    int *exists_ptr)
{
    if (exists_ptr)
        *exists_ptr = 0;
    AVLTree* tree = (AVLTree*) tree_handle;
    tree->root = yadsl_avltree_subtree_node_remove_internal(tree, object, tree->root, 1, exists_ptr);
    return YADSL_AVLTREE_RET_OK;
}

void yadsl_avltree_destroy(yadsl_AVLTreeHandle *tree_handle)
{
    if (tree_handle == NULL)
        return;
    AVLTree* tree = (AVLTree*) tree_handle;
    yadsl_avltree_subtree_destroy_internal(tree, tree->root);
    free(tree);
}

/**** Internal functions definitions ****/

/**
 * @brief Create leaf node with object
 * @param object - object to be stored in node
 * @return pointer to node or NULL, if could nto allocate memory
*/
AVLTreeNode *yadsl_avltree_node_create_internal(yadsl_AVLTreeObject *object)
{
    AVLTreeNode *node = malloc(sizeof *node);
    if (node) {
        node->height = 1;
        node->left = NULL;
        node->right = NULL;
        node->object = object;
    }
    return node;
}

/*
      |                   |
      x                   y
     / \                 / \
   T1   y      ==>      x   T3
       / \             / \
     T2   T3         T1   T2
*/

/**
 * @brief Left-rotate a subtree around its root
 * @param x - subtree root before rotation
 * @attention assumes x->left != NULL
 * @return subtree root after rotation
*/
AVLTreeNode *yadsl_avltree_subtree_left_rotate_internal(AVLTreeNode *x)
{
    AVLTreeNode *y = x->right;
    AVLTreeNode *T2 = y->left;
    x->right = T2;
    y->left = x;
    update_height(x);
    update_height(y);
    return y;
}

/* Right-rotate a subtree around its root

        |                   |
        x                   y
       / \                 / \
      y   T3      ==>    T1   x
     / \                     / \
   T1   T2                 T2   T3

   Parameters:
     * x - subtree root before rotation
           [!] assumes x->right != NULL

   Returns:
     * subtree root after rotation
*/
AVLTreeNode *yadsl_avltree_subtree_right_rotate_internal(AVLTreeNode *x)
{
    AVLTreeNode *y = x->left;
    AVLTreeNode *T2 = y->right;
    x->left = T2;
    y->right = x;
    update_height(x);
    update_height(y);
    return y;
}

/* Balance subtree

   Parameters:
     * x - subtree root before balancing

   Returns:
     * subtree root after balancing
*/
AVLTreeNode *yadsl_avl_subtree_rebalance_internal(AVLTreeNode *x)
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

/* Insert node in subtree

   Parameters:
     * x - subtree root before insertion
     * node - node to be inserted

   Returns:
     * subtree root after insertion
     * if *has_duplicate_ptr == 1, then subtree has duplicate of node
*/
AVLTreeNode *yadsl_avltree_subtree_node_insert_internal(
    AVLTree *tree,
    AVLTreeNode *x,
    AVLTreeNode *node,
    int *has_duplicate_ptr)
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
        *has_duplicate_ptr = 1;
    }
    if (*has_duplicate_ptr)
        return x;
    return yadsl_avl_subtree_rebalance_internal(x);
}

/* Search for node containing object in subtree

   Parameters:
     * object - object to be searched
     * node - subtree root

   Returns:
     * boolean indicating whether node was found
*/
int yadsl_avltree_subtree_node_search_internal(
    AVLTree *tree,
    yadsl_AVLTreeObject *object,
    AVLTreeNode *node)
{
    int cmp;
    if (node == NULL)
        return 0;
    check_invariants(node);
    cmp = compare_objects(object, node->object, tree);
    if (cmp < 0)
        return yadsl_avltree_subtree_node_search_internal(tree, object, node->left);
    else if (cmp > 0)
        return yadsl_avltree_subtree_node_search_internal(tree, object, node->right);
    else
        return 1;
}

/* Delete node containing object from subtree

   Parameters:
     * object - object to be deleted
     * x - subtree root before removal
     * free_obj - whether to free object or not
                  Hint: unless you know what you're doing, pass 1.
   Returns:
     * subtree root after removal
     * if *exists_ptr == 1, then node was removed
*/
AVLTreeNode *yadsl_avltree_subtree_node_remove_internal(
    AVLTree *tree,
    yadsl_AVLTreeObject *object,
    AVLTreeNode *x,
    int free_obj /* = 1 */,
    int *exists_ptr)
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
        AVLTreeNode *retnode;
        if (exists_ptr)
            *exists_ptr = 1;
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
            retnode->right = yadsl_avltree_subtree_node_remove_internal(tree, retnode->object, x->right, 0, exists_ptr);
            retnode->left = x->left;
        }
        if (free_obj) {
            if (tree->free_object_func)
                tree->free_object_func(x->object);
            free(x);
        }
        x = retnode;
    }
    return yadsl_avl_subtree_rebalance_internal(x);
}

/* Traverse subtree in-order

   Parameters:
     * node - subtree root
     * visit_func - visitation function
     * visit_arg - argument passed to visit_func

   Returns:
     * first value returned by visit_func, or 0, if never
*/
yadsl_AVLTreeVisitObjRet *yadsl_avltree_subtree_traverse_internal(
    AVLTreeNode *node,
    yadsl_AVLTreeVisitObjFunc visit_func,
    yadsl_AVLTreeVisitObjArg *visit_arg)
{
    yadsl_AVLTreeVisitObjRet *ret;
    if (node == NULL)
        return NULL;
    if (ret = yadsl_avltree_subtree_traverse_internal(node->left, visit_func, visit_arg))
        return ret;
    check_invariants(node);
    if (visit_func && (ret = visit_func(node->object, visit_arg)))
        return ret;
    if (ret = yadsl_avltree_subtree_traverse_internal(node->right, visit_func, visit_arg))
        return ret;
    return NULL;
}

/* Get subtree minimum node

   Parameters:
     * node - subtree root
       [!] assumes node != NULL

   Returns:
     * subtree minimum node
*/
AVLTreeNode *yadsl_avltree_subtree_get_min_node_internal(AVLTreeNode *node)
{
    for (; node->left; node = node->left);
    return node;
}

/* Destroy subtree

   Parameters:
     * x - subtree root
*/
void yadsl_avltree_subtree_destroy_internal(
    AVLTree *tree,
    AVLTreeNode *x)
{
    AVLTreeNode *left, *right;
    if (x == NULL)
        return;
    left = x->left;
    right = x->right;
    if (tree->free_object_func)
        tree->free_object_func(x->object);
    free(x);
    yadsl_avltree_subtree_destroy_internal(tree, left);
    yadsl_avltree_subtree_destroy_internal(tree, right);
}
