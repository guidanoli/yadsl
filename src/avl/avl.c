#include <avl/avl.h>

#include <assert.h>
#include <stddef.h>

#ifdef YADSL_DEBUG
#include <memdb/memdb.h>
#else
#include <stdlib.h>
#endif

/**** Macros ****/

#ifndef abs
#define abs(a) (a > 0 ? a : -a)
#endif /* abs */

#ifndef max
#define max(a, b) ((a > b) ? a : b)
#endif /* max */

/* height of a node */
#define YADSL_AVLTREE_NODE_HEIGHT(x) \
(x ? x->height : 0)

/* balance of a node */
#define YADSL_AVLTREE_NODE_BALANCE(x) \
(YADSL_AVLTREE_NODE_HEIGHT(x->right) - YADSL_AVLTREE_NODE_HEIGHT(x->left))

/* update node height */
#define YADSL_AVLTREE_UPDATE_NODE_HEIGHT(x) \
x->height = 1 + max(YADSL_AVLTREE_NODE_HEIGHT(x->left), YADSL_AVLTREE_NODE_HEIGHT(x->right))

/* check AVL tree invariants  */
#ifdef YADSL_DEBUG
#define YADSL_AVLTREE_CHECK_INVARIANTS(x) do { \
    if (x != NULL) { \
        assert(x != x->left); \
        assert(x != x->right); \
        if (x->left != NULL && x->right != NULL) \
            assert(x->left != x->right); \
        assert(YADSL_AVLTREE_NODE_HEIGHT(x) == 1 + max(YADSL_AVLTREE_NODE_HEIGHT(x->left), YADSL_AVLTREE_NODE_HEIGHT(x->right))); \
        assert(abs(YADSL_AVLTREE_NODE_BALANCE(x)) < 2); \
    } \
} while(0)
#else /* ifndef YADSL_DEBUG */
#define YADSL_AVLTREE_CHECK_INVARIANTS(x) ((void) 0)
#endif /* YADSL_DEBUG */

/**** Structs ****/

/** AVL subtree */
typedef struct yadsl_AVLSubtree
{
	struct yadsl_AVLSubtree* left;  /**< Left AVL subtree */
	struct yadsl_AVLSubtree* right; /**< Right AVL subtree */
	int height;                     /**< AVL subtree height */
	yadsl_AVLTreeObject* object;    /**< Subtree root object */
}
yadsl_AVLSubtree;

/** AVL tree */
typedef struct
{
	yadsl_AVLSubtree* root; /**< AVL tree root */
}
yadsl_AVLTree;

/**** Internal functions declarations ****/

static yadsl_AVLTreeComparison yadsl_avltree_compare_nodes_internal(
	yadsl_AVLTreeObject* obj1,
	yadsl_AVLTreeObject* obj2,
	yadsl_AVLTreeCallbacks* callbacks);

static yadsl_AVLSubtree* yadsl_avltree_node_create_internal(
	yadsl_AVLTreeObject* object);

static yadsl_AVLSubtree* yadsl_avltree_subtree_left_rotate_internal(
	yadsl_AVLSubtree* x);

static yadsl_AVLSubtree* yadsl_avltree_subtree_right_rotate_internal(
	yadsl_AVLSubtree* x);

static yadsl_AVLSubtree* yadsl_avl_subtree_rebalance_internal(
	yadsl_AVLSubtree* x);

static yadsl_AVLSubtree* yadsl_avltree_subtree_node_insert_internal(
	yadsl_AVLSubtree* x,
	yadsl_AVLSubtree* node,
    yadsl_AVLTreeCallbacks* callbacks,
	bool* has_duplicate_ptr,
	bool* error);

static bool yadsl_avltree_subtree_node_search_internal(
	yadsl_AVLTreeObject* object,
	yadsl_AVLSubtree* node,
    yadsl_AVLTreeCallbacks* callbacks,
	bool* error);

static yadsl_AVLTreeVisitObjRet* yadsl_avltree_subtree_traverse_post_internal(
	yadsl_AVLSubtree* node,
    yadsl_AVLTreeCallbacks* callbacks);

static yadsl_AVLTreeVisitObjRet* yadsl_avltree_subtree_traverse_in_internal(
	yadsl_AVLSubtree* node,
    yadsl_AVLTreeCallbacks* callbacks);

static yadsl_AVLTreeVisitObjRet* yadsl_avltree_subtree_traverse_pre_internal(
	yadsl_AVLSubtree* node,
    yadsl_AVLTreeCallbacks* callbacks);

static yadsl_AVLSubtree* yadsl_avltree_subtree_get_min_node_internal(
	yadsl_AVLSubtree* node);

static yadsl_AVLSubtree* yadsl_avltree_subtree_node_remove_internal(
	yadsl_AVLTreeObject* object,
	yadsl_AVLSubtree* x,
	bool free_obj /* = true */,
    yadsl_AVLTreeCallbacks* callbacks,
	bool* exists_ptr,
	bool* error);

static void yadsl_avltree_subtree_destroy_internal(
	yadsl_AVLSubtree* x,
	yadsl_AVLTreeCallbacks* callbacks);

/**** External functions definitions ****/

yadsl_AVLTreeHandle* yadsl_avltree_tree_create()
{
	yadsl_AVLTree* tree = malloc(sizeof * tree);
	if (tree) {
		tree->root = NULL;
	}
	return tree;
}

yadsl_AVLTreeRet yadsl_avltree_object_insert(
	yadsl_AVLTreeHandle* tree_handle,
	yadsl_AVLTreeObject* object,
    yadsl_AVLTreeCallbacks* callbacks,
	bool* exists_ptr)
{
	bool exists = false;
	bool error = false;
	yadsl_AVLSubtree* node = yadsl_avltree_node_create_internal(object);
	if (node == NULL)
		return YADSL_AVLTREE_RET_MEMORY;
	yadsl_AVLTree* tree = (yadsl_AVLTree*) tree_handle;
	tree->root = yadsl_avltree_subtree_node_insert_internal(tree->root, node, callbacks, &exists, &error);
	if (exists || error)
		free(node);
	if (error)
		return YADSL_AVLTREE_RET_ERR;
	if (exists_ptr)
		*exists_ptr = exists;
	return YADSL_AVLTREE_RET_OK;
}

yadsl_AVLTreeRet yadsl_avltree_object_search(
	yadsl_AVLTreeHandle* tree_handle,
	yadsl_AVLTreeObject* object,
    yadsl_AVLTreeCallbacks* callbacks,
	bool* exists_ptr)
{
	bool error = false;
	yadsl_AVLTree* tree = (yadsl_AVLTree*) tree_handle;
	bool exists = yadsl_avltree_subtree_node_search_internal(object, tree->root, callbacks, &error);
	if (error)
		return YADSL_AVLTREE_RET_ERR;
	if (exists_ptr)
		*exists_ptr = exists;
	return YADSL_AVLTREE_RET_OK;
}

yadsl_AVLTreeRet yadsl_avltree_tree_traverse(
	yadsl_AVLTreeHandle* tree_handle,
	yadsl_AVLTreeVisitingOrder visit_order,
    yadsl_AVLTreeCallbacks* callbacks,
	yadsl_AVLTreeVisitObjRet** visit_ret_ptr)
{
	yadsl_AVLTree* tree = (yadsl_AVLTree*) tree_handle;
	yadsl_AVLTreeVisitObjRet* ret;
	switch (visit_order) {
	case YADSL_AVLTREE_VISITING_PRE_ORDER:
		ret = yadsl_avltree_subtree_traverse_pre_internal(tree->root, callbacks);
		break;
	case YADSL_AVLTREE_VISITING_IN_ORDER:
		ret = yadsl_avltree_subtree_traverse_in_internal(tree->root, callbacks);
		break;
	case YADSL_AVLTREE_VISITING_POST_ORDER:
		ret = yadsl_avltree_subtree_traverse_post_internal(tree->root, callbacks);
		break;
	default:
		return YADSL_AVLTREE_RET_PARAM;
	}
	if (visit_ret_ptr)
		*visit_ret_ptr = ret;
	return YADSL_AVLTREE_RET_OK;
}

yadsl_AVLTreeRet yadsl_avltree_object_remove(
	yadsl_AVLTreeHandle* tree_handle,
	yadsl_AVLTreeObject* object,
    yadsl_AVLTreeCallbacks* callbacks,
	bool* exists_ptr)
{
	bool error = false;
	bool exists = false;
	yadsl_AVLTree* tree = (yadsl_AVLTree*) tree_handle;
	tree->root = yadsl_avltree_subtree_node_remove_internal(object, tree->root, true, callbacks, &exists, &error);
	if (error)
		return YADSL_AVLTREE_RET_ERR;
	if (exists_ptr)
		*exists_ptr = exists;
	return YADSL_AVLTREE_RET_OK;
}

void yadsl_avltree_destroy(
	yadsl_AVLTreeHandle* tree_handle,
	yadsl_AVLTreeCallbacks* callbacks)
{
	if (tree_handle == NULL)
		return;
	yadsl_AVLTree* tree = (yadsl_AVLTree*) tree_handle;
	yadsl_avltree_subtree_destroy_internal(tree->root, callbacks);
	free(tree);
}

/**** Internal functions definitions ****/

/**
 * @brief Create leaf node with object
 * @param object - object to be stored in node
 * @return pointer to node or NULL, if could nto allocate memory
*/
yadsl_AVLSubtree* yadsl_avltree_node_create_internal(yadsl_AVLTreeObject* object)
{
	yadsl_AVLSubtree* node = malloc(sizeof * node);
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
yadsl_AVLSubtree* yadsl_avltree_subtree_left_rotate_internal(yadsl_AVLSubtree* x)
{
	yadsl_AVLSubtree* y = x->right;
	yadsl_AVLSubtree* T2 = y->left;
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
	YADSL_AVLTREE_UPDATE_NODE_HEIGHT(x);
	YADSL_AVLTREE_UPDATE_NODE_HEIGHT(y);
	return y;
}

/**
 * @brief Right-rotate a subtree around its root
 * @param x subtree root before rotation
 * @attention assumes x->right != NULL
 * @return subtree root after rotation
*/
yadsl_AVLSubtree* yadsl_avltree_subtree_right_rotate_internal(yadsl_AVLSubtree* x)
{
	yadsl_AVLSubtree* y = x->left;
	yadsl_AVLSubtree* T2 = y->right;
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
	YADSL_AVLTREE_UPDATE_NODE_HEIGHT(x);
	YADSL_AVLTREE_UPDATE_NODE_HEIGHT(y);
	return y;
}

/**
 * @brief Balance subtree
 * @param x subtree root before balancing
 * @return subtree root after balancing
*/
yadsl_AVLSubtree* yadsl_avl_subtree_rebalance_internal(yadsl_AVLSubtree* x)
{
	int balance;
	if (x == NULL)
		return NULL;
	x->height = 1 + max(YADSL_AVLTREE_NODE_HEIGHT(x->left), YADSL_AVLTREE_NODE_HEIGHT(x->right));
	balance = YADSL_AVLTREE_NODE_BALANCE(x);
	if (balance < -1) {
		/**
		 *    |
		 *    x
		 *   / \
		 * T1*  T2
		 */
		assert(x->left);
		balance = YADSL_AVLTREE_NODE_BALANCE(x->left);
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
		balance = YADSL_AVLTREE_NODE_BALANCE(x->right);
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
	YADSL_AVLTREE_CHECK_INVARIANTS(x);
	return x;
}

/**
 * @brief Insert node in subtree
 * @param x subtree root before insertion
 * @param node node to be inserted
 * @param callbacks only 'compare' callbacks are used
 * @param has_duplicate_ptr whether subtree already has a duplicate
 * @return subtree root after insertion
*/
yadsl_AVLSubtree* yadsl_avltree_subtree_node_insert_internal(
	yadsl_AVLSubtree* x,
	yadsl_AVLSubtree* node,
    yadsl_AVLTreeCallbacks* callbacks,
	bool* has_duplicate_ptr,
	bool* error)
{
	if (x == NULL)
		return node;
	switch (yadsl_avltree_compare_nodes_internal(node->object, x->object, callbacks))
	{
		case YADSL_AVLTREE_COMP_LT:
			/**
			 *    |
			 *    x
			 *   / \
			 * T1*  T2
			 */
			x->left = yadsl_avltree_subtree_node_insert_internal(x->left, node, callbacks, has_duplicate_ptr, error);
			break;
		case YADSL_AVLTREE_COMP_GT:
			/**
			 *    |
			 *    x
			 *   / \
			 * T1   T2*
			 */
			x->right = yadsl_avltree_subtree_node_insert_internal(x->right, node, callbacks, has_duplicate_ptr, error);
			break;
		case YADSL_AVLTREE_COMP_EQ:
			/* Duplicate object */
			*has_duplicate_ptr = true;
			break;
		default:
			/* Error */
			*error = true;
			break;
	}
	if (*has_duplicate_ptr || *error)
		return x;
	return yadsl_avl_subtree_rebalance_internal(x);
}

/**
 * @brief Search for node containing object in subtree
 * @param object object to be searched
 * @param node subtree root
 * @param callbacks only 'compare' callbacks are used
 * @return whether node was found or not
*/
bool yadsl_avltree_subtree_node_search_internal(
	yadsl_AVLTreeObject* object,
	yadsl_AVLSubtree* node,
    yadsl_AVLTreeCallbacks* callbacks,
	bool* error)
{
	int cmp;
	if (node == NULL)
		return false;
	YADSL_AVLTREE_CHECK_INVARIANTS(node);
	switch (yadsl_avltree_compare_nodes_internal(object, node->object, callbacks))
	{
		case YADSL_AVLTREE_COMP_LT:
			return yadsl_avltree_subtree_node_search_internal(object, node->left, callbacks, error);
		case YADSL_AVLTREE_COMP_GT:
			return yadsl_avltree_subtree_node_search_internal(object, node->right, callbacks, error);
		case YADSL_AVLTREE_COMP_EQ:
			return true;
		default:
			*error = true;
			return false;
	}
}

/**
 * @brief Remove node containing object from subtree
 * @param object object to be deleted
 * @param x subtree root before removal
 * @param free_obj whether to free object or not (Hint: unles you know what
 * you are doing, pass true).
 * @param callbacks only 'compare' and 'free' callbacks are used
 * @param exists_ptr whether node existed or not
 * @return subtree root after removal
*/
yadsl_AVLSubtree* yadsl_avltree_subtree_node_remove_internal(
	yadsl_AVLTreeObject* object,
	yadsl_AVLSubtree* x,
	bool free_obj /* = true */,
    yadsl_AVLTreeCallbacks* callbacks,
	bool* exists_ptr,
	bool* error)
{
	if (x == NULL)
		return NULL;
	switch (yadsl_avltree_compare_nodes_internal(object, x->object, callbacks))
	{
		case YADSL_AVLTREE_COMP_LT:
			/**
			 *    |
			 *    x
			 *   / \
			 * T1*  T2
			 */
			x->left = yadsl_avltree_subtree_node_remove_internal(object, x->left, free_obj, callbacks, exists_ptr, error);
			break;
		case YADSL_AVLTREE_COMP_GT:
			/**
			 *    |
			 *    x
			 *   / \
			 * T1   T2*
			 */
			x->right = yadsl_avltree_subtree_node_remove_internal(object, x->right, free_obj, callbacks, exists_ptr, error);
			break;
		case YADSL_AVLTREE_COMP_EQ:
		{
			yadsl_AVLSubtree* retnode;
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
				retnode->right = yadsl_avltree_subtree_node_remove_internal(retnode->object, x->right, false, callbacks, exists_ptr, error);
				retnode->left = x->left;
			}
			if (free_obj) {
				if (callbacks->free_cb)
					callbacks->free_cb(x->object, callbacks->free_arg);
				free(x);
			}
			x = retnode;
			break;
		}
		default:
			/* Error */
			*error = true;
			return x;
	}
	return yadsl_avl_subtree_rebalance_internal(x);
}

/**
 * @brief Traverse subtree pre-order
 * @param node subtree root
 * @param callbacks uses 'visit' callback only
 * @return first value returned by visit_func, or 0 if it was never called
*/
yadsl_AVLTreeVisitObjRet* yadsl_avltree_subtree_traverse_pre_internal(
	yadsl_AVLSubtree* node,
    yadsl_AVLTreeCallbacks* callbacks)
{
	yadsl_AVLTreeVisitObjRet* ret;
	if (node == NULL)
		return NULL;
	YADSL_AVLTREE_CHECK_INVARIANTS(node);
	if (ret = callbacks->visit_cb(node->object, callbacks->visit_arg))
		return ret;
	if (ret = yadsl_avltree_subtree_traverse_in_internal(node->left, callbacks))
		return ret;
	if (ret = yadsl_avltree_subtree_traverse_in_internal(node->right, callbacks))
		return ret;
	return NULL;
}

/**
 * @brief Traverse subtree in-order
 * @param node subtree root
 * @param callbacks uses 'visit' callback only
 * @return first value returned by visit_func, or 0 if it was never called
*/
yadsl_AVLTreeVisitObjRet* yadsl_avltree_subtree_traverse_in_internal(
	yadsl_AVLSubtree* node,
    yadsl_AVLTreeCallbacks* callbacks)
{
	yadsl_AVLTreeVisitObjRet* ret;
	if (node == NULL)
		return NULL;
	if (ret = yadsl_avltree_subtree_traverse_in_internal(node->left, callbacks))
		return ret;
	YADSL_AVLTREE_CHECK_INVARIANTS(node);
	if (ret = callbacks->visit_cb(node->object, callbacks->visit_arg))
		return ret;
	if (ret = yadsl_avltree_subtree_traverse_in_internal(node->right, callbacks))
		return ret;
	return NULL;
}

/**
 * @brief Traverse subtree post-order
 * @param node subtree root
 * @param callbacks uses 'visit' callback only
 * @return first value returned by visit_func, or 0 if it was never called
*/
yadsl_AVLTreeVisitObjRet* yadsl_avltree_subtree_traverse_post_internal(
	yadsl_AVLSubtree* node,
    yadsl_AVLTreeCallbacks* callbacks)
{
	yadsl_AVLTreeVisitObjRet* ret;
	if (node == NULL)
		return NULL;
	if (ret = yadsl_avltree_subtree_traverse_in_internal(node->left, callbacks))
		return ret;
	if (ret = yadsl_avltree_subtree_traverse_in_internal(node->right, callbacks))
		return ret;
	YADSL_AVLTREE_CHECK_INVARIANTS(node);
	if (ret = callbacks->visit_cb(node->object, callbacks->visit_arg))
		return ret;
	return NULL;
}

/**
 * @brief Get subtree minimum node
 * @param node subtree root
 * @attention assumes node != NULL
 * @return subtree minimum node
*/
yadsl_AVLSubtree* yadsl_avltree_subtree_get_min_node_internal(yadsl_AVLSubtree* node)
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
	yadsl_AVLSubtree* x,
	yadsl_AVLTreeCallbacks* callbacks)
{
	yadsl_AVLSubtree* left, * right;
	if (x == NULL)
		return;
	left = x->left;
	right = x->right;
	if (callbacks->free_cb)
		callbacks->free_cb(x->object, callbacks->free_arg);
	free(x);
	yadsl_avltree_subtree_destroy_internal(left, callbacks);
	yadsl_avltree_subtree_destroy_internal(right, callbacks);
}

/**
 * @brief Compares nodes according to comparison callback
 * @param obj1 first object
 * @param obj2 second object
 * @param callbacks only the 'compare' callback is used (optional)
 * @return comparison between the two or error
*/
yadsl_AVLTreeComparison yadsl_avltree_compare_nodes_internal(
	yadsl_AVLTreeObject* obj1,
	yadsl_AVLTreeObject* obj2,
	yadsl_AVLTreeCallbacks* callbacks)
{
	yadsl_AVLTreeCmpObjsFunc func = callbacks->compare_cb;
	if (func == NULL) {
		if (obj1 == obj2) {
			return YADSL_AVLTREE_COMP_EQ;
		} else if (obj1 < obj2) {
			return YADSL_AVLTREE_COMP_LT;
		} else {
			return YADSL_AVLTREE_COMP_GT;
		}
	} else {
		return func(obj1, obj2, callbacks->compare_arg);
	}
}
