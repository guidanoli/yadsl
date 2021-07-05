#ifndef __YADSL_AVL_H__
#define __YADSL_AVL_H__

/**
 * \defgroup avl AVL tree
 * @brief AVL tree with explicit heights
 * @{
*/

#include <stdbool.h>

/**
 * @brief Status value returned by AVL tree functions
*/
typedef enum
{
	YADSL_AVLTREE_RET_OK = 0, /**< All went well */
	YADSL_AVLTREE_RET_MEMORY, /**< Could not allocate memory */
	YADSL_AVLTREE_RET_PARAM,  /**< Invalid parameter */
    YADSL_AVLTREE_RET_ERR,    /**< Error raised by callback */
}
yadsl_AVLTreeRet;


typedef enum
{
    YADSL_AVLTREE_COMP_GT, /**< a > b */
    YADSL_AVLTREE_COMP_EQ, /**< a == b */
    YADSL_AVLTREE_COMP_LT, /**< a < b */
    YADSL_AVLTREE_COMP_ERR, /**< An error occurred */
}
yadsl_AVLTreeComparison;

/**
 * @brief Order of visiting in AVL trees
*/
typedef enum
{
	YADSL_AVLTREE_VISITING_PRE_ORDER, /**< Pre-order */
	YADSL_AVLTREE_VISITING_IN_ORDER, /**< In-order */
	YADSL_AVLTREE_VISITING_POST_ORDER, /**< Post-order */
}
yadsl_AVLTreeVisitingOrder;

typedef void yadsl_AVLTreeHandle; /**< AVL tree handle */
typedef void yadsl_AVLTreeObject; /**< AVL tree object (user data) */
typedef void yadsl_AVLTreeCmpObjsArg; /**< AVL tree object comparison function user argument */
typedef void yadsl_AVLTreeFreeObjArg; /**< AVL tree object freeing function user argument */

/**
 * @brief AVL tree object comparison function
 * @param obj1 first object
 * @param obj2 second object
 * @param arg user argument
 * @return comparison between obj1 and obj2 or error (interrupts comparisons)
*/
typedef yadsl_AVLTreeComparison
(*yadsl_AVLTreeCmpObjsFunc)(
	yadsl_AVLTreeObject* obj1,
	yadsl_AVLTreeObject* obj2,
	yadsl_AVLTreeCmpObjsArg* arg);

/**
 * @brief AVL tree object freeing function
 * @param obj object
*/
typedef void
(*yadsl_AVLTreeFreeObjFunc)(
	yadsl_AVLTreeObject* obj,
    yadsl_AVLTreeFreeObjArg* arg);

typedef void yadsl_AVLTreeVisitObjArg; /**< AVL tree visiting function user argument */
typedef void yadsl_AVLTreeVisitObjRet; /**< AVL tree visiting function return type */

/**
 * @brief AVL tree visiting function
 * @param obj object
 * @param arg user argument
 * @return a value that, if equal to zero, interrupts visitation
 * @note you may want to interupt visitation if an error occurred
*/
typedef yadsl_AVLTreeVisitObjRet*
(*yadsl_AVLTreeVisitObjFunc)(
	yadsl_AVLTreeObject* obj,
	yadsl_AVLTreeVisitObjArg* arg);

/**
 * @brief Callbacks for comparing and freeing objects
 */
typedef struct
{
	yadsl_AVLTreeCmpObjsFunc compare_cb;
	yadsl_AVLTreeCmpObjsArg* compare_arg;
	yadsl_AVLTreeFreeObjFunc free_cb;
    yadsl_AVLTreeFreeObjArg* free_arg;
	yadsl_AVLTreeVisitObjFunc visit_cb;
	yadsl_AVLTreeVisitObjArg* visit_arg;
}
yadsl_AVLTreeCallbacks;

/**
 * @brief Create an empty tree
 * @return newly created tree or NULL if could not allocate memory
*/
yadsl_AVLTreeHandle*
yadsl_avltree_tree_create();

/**
 * @brief Insert object in tree
 * @param tree tree where object will be inserted
 * @param object object to be inserted
 * @param callbacks uses 'compare' callbacks only
 * @param exists_ptr whether object exists or not
 * @note if returns OK, but object already exists in the tree,
 * you might want to deallocate any memory that might have been
 * allocated specially for the object to be inserted
 * @return
 * * ::YADSL_AVLTREE_RET_OK, and object is inserted in tree
 * * ::YADSL_AVLTREE_RET_MEMORY
 * * ::YADSL_AVLTREE_RET_ERR
*/
yadsl_AVLTreeRet
yadsl_avltree_object_insert(
	yadsl_AVLTreeHandle* tree,
	yadsl_AVLTreeObject* object,
    yadsl_AVLTreeCallbacks* callbacks,
	bool* exists_ptr);

/**
 * @brief Search for object in tree
 * @param tree tree where object will be searched
 * @param object object to be searched
 * @param callbacks uses 'compare' callbacks only
 * @param exists_ptr whether object exists or not
 * @return
 * * ::YADSL_AVLTREE_RET_OK, and *exist_ptr is updated
 * * ::YADSL_AVLTREE_RET_ERR
*/
yadsl_AVLTreeRet
yadsl_avltree_object_search(
	yadsl_AVLTreeHandle* tree,
	yadsl_AVLTreeObject* object,
    yadsl_AVLTreeCallbacks* callbacks,
	bool* exists_ptr);

/**
 * @brief Remove object from tree
 * @param tree tree where object will be removed from
 * @param object object to be removed
 * @param callbacks uses 'compare' and 'free' callbacks only
 * @param exists_ptr whether object existed or not
 * @return
 * * ::YADSL_AVLTREE_RET_OK, and *exists_ptr is updated
 * * ::YADSL_AVLTREE_RET_ERR
*/
yadsl_AVLTreeRet
yadsl_avltree_object_remove(
	yadsl_AVLTreeHandle* tree,
	yadsl_AVLTreeObject* object,
    yadsl_AVLTreeCallbacks* callbacks,
	bool* exists_ptr);

/**
 * @brief Traverses tree in-order calling user function for each object
 * @param tree tree to be traversed
 * @param visit_order visiting order
 * @param callbacks uses 'visit' callback only
 * @param visit_ret_ptr pointer to last value returned by visit_func
 * @return
 * * ::YADSL_AVLTREE_RET_OK, and *visit_ret_ptr is updated
 * * ::YADSL_AVLTREE_RET_PARAM, if visit_order is invalid
*/
yadsl_AVLTreeRet
yadsl_avltree_tree_traverse(
	yadsl_AVLTreeHandle* tree,
	yadsl_AVLTreeVisitingOrder visit_order,
    yadsl_AVLTreeCallbacks* callbacks,
	yadsl_AVLTreeVisitObjRet** visit_ret_ptr);

/**
 * @brief Destroy tree and its objects
 * @param tree tree to be destroyed
 * @param callbacks uses 'free' callbacks only
*/
void
yadsl_avltree_destroy(
	yadsl_AVLTreeHandle* tree,
    yadsl_AVLTreeCallbacks* callbacks);

/** @} */

#endif
