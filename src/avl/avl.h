#ifndef __AA_AVL_H__
#define __AA_AVL_H__

/* Implementation of AVL tree with explicit heights
   Version 0.0 */

typedef enum
{
    /* All went well */
    AA_AVLTREE_RET_OK = 0,

    /* Could not allocate memory */
    AA_AVLTREE_RET_MEMORY,
}
AVLTreeRet;

typedef enum
{
    /* Pre-order */
    AA_AVLTREE_TRAVERSE_PRE_ORDER,

    /* In-order */
    AA_AVLTREE_TRAVERSE_IN_ORDER,

    /* Post-order */
    AA_AVLTREE_TRAVERSE_POST_ORDER,
}
AVLTreeTraversalOrder;

/* AVL tree handle */
typedef void AVLTreeHandle;

/* AVL tree object (user data) */
typedef void AVLTreeObject;

/* AVL tree object comparison function user argument */
typedef void AVLTreeCmpObjsArg;

/* AVL tree object comparison function

   Called with two objects and an user argument.

   When returns...
     * a positive number, then obj1 > obj2
     * zero, then obj1 == obj2
     * a negative number, then obj1 < obj2
*/
typedef int (*AVLTreeCmpObjsFunc)(AVLTreeObject*, AVLTreeObject*, AVLTreeCmpObjsArg*);

/* AVL tree object freeing function

   Called for every object in the tree upon destruction
*/
typedef void (*AVLTreeFreeObjFunc)(AVLTreeObject*);

/* AVL tree visiting function user argument */
typedef void AVLTreeVisitObjArg;

/* AVL tree visiting function return type */
typedef void AVLTreeVisitObjRet;

/* AVL tree visiting function

   Called with an object and an user argument.

   When returns...
      * zero, then visitation continues
      * non-zero, then visitation stops
*/
typedef AVLTreeVisitObjRet* (*AVLTreeVisitObjFunc)(AVLTreeObject*, AVLTreeVisitObjArg*);

/* Create an empty tree

   Parameters:
     * cmp_objs_func - tree object comparison function
     * cmp_objs_arg - argument passed to cmp_objs_func
     * free_obj_func - tree object freeing function

   Returns:
     * OK - *tree_ptr now points to the newly created tree
     * MEMORY
*/
AVLTreeRet aa_avltree_tree_create(
    AVLTreeCmpObjsFunc cmp_objs_func,
    AVLTreeCmpObjsArg *cmp_objs_arg,
    AVLTreeFreeObjFunc free_obj_func,
    AVLTreeHandle **tree_ptr);

/* Insert object in tree

   Parameters:
     * object - object to be inserted

   Returns:
     * OK - if *exists_ptr == 0, then the object was inserted
     * MEMORY
*/
AVLTreeRet aa_avltree_object_insert(
    AVLTreeHandle *tree,
    AVLTreeObject *object,
    int *exists_ptr);

/* Search for object in tree

   Parameters:
     * object - object to be searched for

   Returns:
     * OK - if *exists_ptr == 0, then the object is not in the tree
*/
AVLTreeRet aa_avltree_object_search(
    AVLTreeHandle *tree,
    AVLTreeObject *object,
    int *exists_ptr);

/* Delete object from tree

   Parameters:
     * object - object to be deleted

   Returns:
     * OK - if *exists_ptr == 1, then the object was removed
*/
AVLTreeRet aa_avltree_object_remove(
    AVLTreeHandle* tree,
    AVLTreeObject* object,
    int* exists_ptr);

/* Traverse tree in-order

   Parameters:
     * visit_func - tree object visiting function
     * visit_arg - argument passed to visit_func

   Returns:
     * OK - *visit_ret_ptr points to the last value returned by visit_func
*/
AVLTreeRet aa_avltree_tree_traverse(
    AVLTreeHandle *tree,
    AVLTreeVisitObjFunc visit_func,
    AVLTreeVisitObjArg *visit_arg,
    AVLTreeVisitObjRet **visit_ret_ptr);

/* Destroy tree and its objects, passing them to the free function */
void aa_avltree_destroy(AVLTreeHandle *tree);

#endif
