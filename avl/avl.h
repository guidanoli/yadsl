#ifndef __AVL_H__
#define __AVL_H__

//
//      ___ _    ____ 
//     /   | |  / / / 
//    / /| | | / / /  
//   / ___ | |/ / /___
//  /_/  |_|___/_____/
//                    
// An AVL tree is a self-balancing binary tree, that
// guarantees that the tree does not degenerate into a
// single-linked list, and assuring that its height is
// less or equal to 1.44*log2(n+1.065)-0.328.
//
// It is a better option for storing ordered and unique
// data than linked lists in matter of time complexity
// for most actions.
//
// +-----------+-----------+------------+
// | Algorithm |  Average  | Worst Case |
// +-----------+-----------+------------+
// | Space     | O(n)      | O(n)       |
// | Search    | O(log(n)) | O(log(n))  |
// | Insert    | O(log(n)) | O(log(n))  |
// | Delete    | O(log(n)) | O(log(n))  |
// +-----------+-----------+------------+
//

typedef enum
{
	/* All went ok */
	AVL_RETURN_OK = 0,

	/* Unexpected parameter value */
	AVL_RETURN_INVALID_PARAMETER,

	/* Not enough memory */
	AVL_RETURN_MEMORY,
}
AVLReturnId;

typedef struct AVL AVL;

/**
* Create an empty tree
* ppTree   (return) pointer to tree
* cmpObjs  function that compares two objects
* freeObj  function that deallocates object
* arg      argument passed to cmpObjs
* Possible errors:
* AVL_RETURN_INVALID_PARAMETER
*	- "ppTree" is NULL
* AVL_RETURN_MEMORY
*/
AVLReturnId avlCreate(AVL **ppTree,
	int (*cmpObjs)(void *obj1, void *obj2, void *arg),
	void (*freeObj)(void *object), void *arg);

/**
* Insert object in tree
* pTree    pointer to tree
* object   object to be inserted
* pExists  (return) object was in tree before
* Possible errors:
* AVL_RETURN_INVALID_PARAMETER
*	- "pTree" is NULL
* AVL_RETURN_MEMORY
* [!] Only takes the ownership of object if
*     AVL_RETURN_OK && *pExists == 0
*/
AVLReturnId avlInsert(AVL *pTree, void *object, int *pExists);

/**
* Search for object in tree
* pTree    pointer to tree
* object   object to be searched for
* pExists  (return) object is in tree
* Possible errors:
* AVL_RETURN_INVALID_PARAMETER
*	- "pTree" is NULL
* [!] Does not take ownership of object
*/
AVLReturnId avlSearch(AVL *pTree, void *object, int *pExists);

/**
* Traverse through tree
* pTree      pointer to tree
* visit_cb   function called for every visted object
*            if it returns a value other than NULL (0), traversing stops
*            and this value is returned by reference on *pReturn
* arg        argument passed to visit_cb
* pReturn    (return) last return value by visit_cb
* Possible errors:
* AVL_RETURN_INVALID_PARAMETER
*	- "pTree" is NULL
*/
AVLReturnId avlTraverse(AVL *pTree,
	void * (*visit_cb)(void *object, void *arg),
	void *arg, void **pReturn);

/**
* Delete object from tree
* pTree    pointer to tree
* object   object to be deleted
* pExists  (return) object was in tree before
* Possible errors:
* AVL_RETURN_INVALID_PARAMETER
*	- "pTree" is NULL
* [!] Does not take ownership of object
*/
AVLReturnId avlDelete(AVL *pTree, void *object, int *pExists);

/**
* Destroys tree
*/
void avlDestroy(AVL *pTree);

#endif
