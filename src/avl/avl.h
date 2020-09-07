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
    YADSL_AVLTREE_RET_PARAM   /**< Invalid parameter */
}
yadsl_AVLTreeRet;

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

/**
 * @brief AVL tree object comparison function
 * @param obj1 first object
 * @param obj2 second object
 * @param arg user argument
 * @return an integer *n*, where...
 * * *n* > 0 if obj1 > obj2
 * * *n* = 0 if obj1 = obj2
 * * *n* < 0 if obj1 < obj2
*/
typedef int (*yadsl_AVLTreeCmpObjsFunc)(yadsl_AVLTreeObject *obj1, yadsl_AVLTreeObject *obj2, yadsl_AVLTreeCmpObjsArg *arg);

/**
 * @brief AVL tree object freeing function
 * @param obj object
*/
typedef void (*yadsl_AVLTreeFreeObjFunc)(yadsl_AVLTreeObject *obj);

typedef void yadsl_AVLTreeVisitObjArg; /**< AVL tree visiting function user argument */
typedef void yadsl_AVLTreeVisitObjRet; /**< AVL tree visiting function return type */

/**
 * @brief AVL tree visiting function
 * @param obj object
 * @param arg user argument
 * @return a value that, if equal to zero, interrupts visitation
*/
typedef yadsl_AVLTreeVisitObjRet* (*yadsl_AVLTreeVisitObjFunc)(yadsl_AVLTreeObject *obj, yadsl_AVLTreeVisitObjArg *arg);

/**
 * @brief Create an empty tree
 * @param cmp_objs_func tree object comparison function
 * @param cmp_objs_arg argument passed to cmp_objs_func
 * @param free_obj_func tree object freeing function
 * @return newly created tree or NULL if could not allocate memory
*/
yadsl_AVLTreeHandle* yadsl_avltree_tree_create(
    yadsl_AVLTreeCmpObjsFunc cmp_objs_func,
    yadsl_AVLTreeCmpObjsArg *cmp_objs_arg,
    yadsl_AVLTreeFreeObjFunc free_obj_func);

/**
 * @brief Insert object in tree
 * @param tree tree where object will be inserted
 * @param object object to be inserted
 * @param exists_ptr whether object exists or not
 * @return
 * * ::YADSL_AVLTREE_RET_OK, and object is inserted in tree
 * * ::YADSL_AVLTREE_RET_MEMORY
*/
yadsl_AVLTreeRet yadsl_avltree_object_insert(
    yadsl_AVLTreeHandle *tree,
    yadsl_AVLTreeObject *object,
    bool *exists_ptr);

/**
 * @brief Search for object in tree
 * @param tree tree where object will be searched
 * @param object object to be searched
 * @param exists_ptr whether object exists or not
 * @return
 * * ::YADSL_AVLTREE_RET_OK, and *exist_ptr is updated
*/
yadsl_AVLTreeRet yadsl_avltree_object_search(
    yadsl_AVLTreeHandle *tree,
    yadsl_AVLTreeObject *object,
    bool *exists_ptr);

/**
 * @brief Remove object from tree
 * @param tree tree where object will be removed from
 * @param object object to be removed
 * @param exists_ptr whether object existed or not
 * @return
 * * ::YADSL_AVLTREE_RET_OK, and *exists_ptr is updated
*/
yadsl_AVLTreeRet yadsl_avltree_object_remove(
    yadsl_AVLTreeHandle* tree,
    yadsl_AVLTreeObject* object,
    bool* exists_ptr);

/**
 * @brief Traverses tree in-order calling user function for each object
 * @param tree tree to be traversed
 * @param visit_order visiting order
 * @param visit_func function called for each object
 * @param visit_arg argument passed to visit_func
 * @param visit_ret_ptr pointer to last value returned by visit_func
 * @return
 * * ::YADSL_AVLTREE_RET_OK, and *visit_ret_ptr is updated
 * * ::YADSL_AVLTREE_RET_PARAM, if visit_order is invalid
*/
yadsl_AVLTreeRet yadsl_avltree_tree_traverse(
    yadsl_AVLTreeHandle *tree,
    yadsl_AVLTreeVisitingOrder visit_order,
    yadsl_AVLTreeVisitObjFunc visit_func,
    yadsl_AVLTreeVisitObjArg *visit_arg,
    yadsl_AVLTreeVisitObjRet **visit_ret_ptr);

/**
 * @brief Destroy tree and its objects
 * @param tree tree to be destroyed
*/
void yadsl_avltree_destroy(yadsl_AVLTreeHandle *tree);

/** @} */

#endif
