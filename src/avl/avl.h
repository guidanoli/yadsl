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
aa_AVLTreeRet;

typedef enum
{
    /* Pre-order */
    AA_AVLTREE_TRAVERSE_PRE_ORDER,

    /* In-order */
    AA_AVLTREE_TRAVERSE_IN_ORDER,

    /* Post-order */
    AA_AVLTREE_TRAVERSE_POST_ORDER,
}
aa_AVLTreeTraversalOrder;

/* AVL tree handle */
typedef void aa_AVLTreeHandle;

/* AVL tree object (user data) */
typedef void aa_AVLTreeObject;

/* AVL tree object comparison function user argument */
typedef void aa_AVLTreeCmpObjsArg;

/* AVL tree object comparison function

   Called with two objects and an user argument.

   When returns...
     * a positive number, then obj1 > obj2
     * zero, then obj1 == obj2
     * a negative number, then obj1 < obj2
*/
typedef int (*aa_AVLTreeCmpObjsFunc)(aa_AVLTreeObject*, aa_AVLTreeObject*, aa_AVLTreeCmpObjsArg*);

/* AVL tree object freeing function

   Called for every object in the tree upon destruction
*/
typedef void (*aa_AVLTreeFreeObjFunc)(aa_AVLTreeObject*);

/* AVL tree visiting function user argument */
typedef void aa_AVLTreeVisitObjArg;

/* AVL tree visiting function return type */
typedef void aa_AVLTreeVisitObjRet;

/* AVL tree visiting function

   Called with an object and an user argument.

   When returns...
      * zero, then visitation continues
      * non-zero, then visitation stops
*/
typedef aa_AVLTreeVisitObjRet* (*aa_AVLTreeVisitObjFunc)(aa_AVLTreeObject*, aa_AVLTreeVisitObjArg*);

/* Create an empty tree

   Parameters:
     * cmp_objs_func - tree object comparison function
     * cmp_objs_arg - argument passed to cmp_objs_func
     * free_obj_func - tree object freeing function

   Returns:
     * OK - *tree_ptr now points to the newly created tree
     * MEMORY
*/
aa_AVLTreeRet aa_avltree_tree_create(
    aa_AVLTreeCmpObjsFunc cmp_objs_func,
    aa_AVLTreeCmpObjsArg *cmp_objs_arg,
    aa_AVLTreeFreeObjFunc free_obj_func,
    aa_AVLTreeHandle **tree_ptr);

/* Insert object in tree

   Parameters:
     * object - object to be inserted

   Returns:
     * OK - if *exists_ptr == 0, then the object was inserted
     * MEMORY
*/
aa_AVLTreeRet aa_avltree_object_insert(
    aa_AVLTreeHandle *tree,
    aa_AVLTreeObject *object,
    int *exists_ptr);

/* Search for object in tree

   Parameters:
     * object - object to be searched for

   Returns:
     * OK - if *exists_ptr == 0, then the object is not in the tree
*/
aa_AVLTreeRet aa_avltree_object_search(
    aa_AVLTreeHandle *tree,
    aa_AVLTreeObject *object,
    int *exists_ptr);

/* Delete object from tree

   Parameters:
     * object - object to be deleted

   Returns:
     * OK - if *exists_ptr == 1, then the object was removed
*/
aa_AVLTreeRet aa_avltree_object_remove(
    aa_AVLTreeHandle* tree,
    aa_AVLTreeObject* object,
    int* exists_ptr);

/* Traverse tree in-order

   Parameters:
     * visit_func - tree object visiting function
     * visit_arg - argument passed to visit_func

   Returns:
     * OK - *visit_ret_ptr points to the last value returned by visit_func
*/
aa_AVLTreeRet aa_avltree_tree_traverse(
    aa_AVLTreeHandle *tree,
    aa_AVLTreeVisitObjFunc visit_func,
    aa_AVLTreeVisitObjArg *visit_arg,
    aa_AVLTreeVisitObjRet **visit_ret_ptr);

/* Destroy tree and its objects, passing them to the free function */
void aa_avltree_destroy(aa_AVLTreeHandle *tree);

#endif
