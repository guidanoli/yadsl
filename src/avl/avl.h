#ifndef __AVL_H__
#define __AVL_H__

//
//      ___ _    ____ 
//     /   | |  / / / 
//    / /| | | / / /  
//   / ___ | |/ / /___
//  /_/  |_|___/_____/
//                    
// 	An AVL tree is a self-balancing binary tree
// that guarantees that its height is always in the
// magnitude of O(log(n)), being n the number of nodes.
//
// 	It is a better option for storing ordered and
// unique data than linked lists in the matter of time
// complexity for the following actions:
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
// HINTS
// -----
//
// 	- You may use AVLReturnId to check for errors
// by using it as a boolean value, since AVL_RETURN_OK
// will always assume the value 0. This does not exclude
// the importance to check other values returned by ref.
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
* ppTree (ret): pointer to tree
* cmpObjs (opt): function that compares two objects
*          	       if NULL, does shallow address comparison
* freeObj (opt): function that deallocates object
*          	       if NULL, does not take ownership over objects
* arg: argument passed to cmpObjs
* -> AVL_RETURN_INVALID_PARAMETER
* -> AVL_RETURN_MEMORY
*/
AVLReturnId avlCreate(AVL **ppTree,
	int (*cmpObjs)(void *obj1, void *obj2, void *arg),
	void (*freeObj)(void *object), void *arg);

/**
* Insert object in tree
* pTree (ret): pointer to tree
* object: object to be inserted
* pExists (opt ret): object was in tree before
*          	         if NULL, won't be able to check whether
*          	         object was truly inserted in tree or not.
* -> AVL_RETURN_INVALID_PARAMETER
* -> AVL_RETURN_MEMORY
* [!] Only takes the ownership of object if
*     AVL_RETURN_OK && *pExists == 0
*/
AVLReturnId avlInsert(AVL *pTree, void *object, int *pExists);

/**
* Search for object in tree
* pTree: pointer to tree
* object: object to be searched for
* pExists (opt ret): whether object is in tree
*          	         if NULL, won't be able to check whether
*          	         object is in tree or not.
* -> AVL_RETURN_INVALID_PARAMETER
* [!] Does not take ownership of object
*/
AVLReturnId avlSearch(AVL *pTree, void *object, int *pExists);

/**
* Traverse through tree
* pTree: pointer to tree
* visit_cb (opt): function called for every visted object
*            	  if NULL, it will simply not be called.
*            	  if it returns a value other than NULL (0), traversing
*            	  stops and this value is returned by reference on *pReturn.
* arg: argument passed to visit_cb
* pReturn (opt ret): last return value by visit_cb
* -> AVL_RETURN_INVALID_PARAMETER
*/
AVLReturnId avlTraverse(AVL *pTree,
	void * (*visit_cb)(void *object, void *arg),
	void *arg, void **pReturn);

/**
* Delete object from tree
* pTree: pointer to tree
* object: object to be deleted
* pExists (opt ret): object was in tree before
*          	         if NULL, won't be able to check whether
*          	         it was in tree before or not.
* -> AVL_RETURN_INVALID_PARAMETER
* [!] Does not take ownership of object
*/
AVLReturnId avlDelete(AVL *pTree, void *object, int *pExists);

/**
* Destroys tree
*/
void avlDestroy(AVL *pTree);

#endif
