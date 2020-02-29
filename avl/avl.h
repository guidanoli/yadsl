#ifndef __AVL_H__
#define __AVL_H__

//
//      ___ _    ____ 
//     /   | |  / / / 
//    / /| | | / / /  
//   / ___ | |/ / /___
//  /_/  |_|___/_____/
//                    
// An A.V.L. pTree is a self-balancing binary pTree, that
// guarantees that the pTree does not degenerate into a
// single-linked list, corrupting the O(log(n)) time
// complexity of its main operations such as searching.
//

typedef enum
{
	/* All went ok */
	AVL_RETURN_OK = 0,

	/* Node already on pTree */
	AVL_RETURN_DUPLICATE,

	/* Node not found */
	AVL_RETURN_NODE_NOT_FOUND,

	/* Unexpected parameter value */
	AVL_RETURN_INVALID_PARAMETER,

	/* Not enough memory */
	AVL_RETURN_MEMORY,
}
AVLReturnId;

typedef struct AVLTree AVLTree;

AVLReturnId avlCreate(AVLTree **ppTree,
	int (*cmpObjs)(void *obj1, void *obj2, void *arg),
	void (*freeObj)(void *object), void *arg);

AVLReturnId avlInsert(AVLTree *pTree, void *object);

AVLReturnId avlSearch(AVLTree *pTree, void *object, int *pExists);

AVLReturnId avlTraverse(AVLTree *pTree, void (*visit_cb)(void *object));

AVLReturnId avlDelete(AVLTree *pTree, void *object);

void avlDestroy(AVLTree *pTree);

#endif
