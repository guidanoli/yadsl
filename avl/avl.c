#include "avl.h"

#include <stddef.h>
#include <stdlib.h>
#include <assert.h>

#ifdef _DEBUG
#include "memdb.h"
#endif

// For every x, x->left <= x <= x->right
// -1: o1 < o2
//  0: o1 = o2
// +1: o1 > o2
#define CMP(o1, o2, t) (t->cmpObjs ? t->cmpObjs(o1, o2, t->arg) : \
 (o1 < o2 ? -1 : (o1 == o2 ? 0 : 1)))

// height of a node
#define height(x) (x ? x->height : 0)

// balance of a node
#define balance(x) (height(x->right) - height(x->left))

// update node height
#define update_height(x) x->height = 1 + max(height(x->left), height(x->right))

struct AVLNode
{
	struct AVLNode *left;
	struct AVLNode *right;
	int height; // 2^INT_MAX nodes
	void *object;
};

struct AVLTree
{
	struct AVLNode *root;
	int (*cmpObjs)(void *obj1, void *obj2, void *arg);
	void (*freeObj)(void *object);
	void *arg;
};

AVLReturnId avlCreate(AVLTree **ppTree,
	int (*cmpObjs)(void *obj1, void *obj2, void *arg),
	void (*freeObj)(void *object), void *arg)
{
	AVLTree *pTree;
	if (ppTree == NULL)
		return AVL_RETURN_INVALID_PARAMETER;
	pTree = malloc(sizeof(struct AVLTree));
	if (pTree == NULL)
		return AVL_RETURN_MEMORY;
	pTree->root = NULL;
	pTree->cmpObjs = cmpObjs;
	pTree->freeObj = freeObj;
	pTree->arg = arg;
	*ppTree = pTree;
	return AVL_RETURN_OK;
}

struct AVLNode *create_node(void *object)
{
	struct AVLNode *node = malloc(sizeof(struct AVLNode));
	if (node) {
		node->height = 1;
		node->left = NULL;
		node->right = NULL;
		node->object = object;
	}
	return node;
}

// Assumes x->left != NULL
struct AVLNode *leftRotate(struct AVLNode *x)
{
	struct AVLNode *y = x->right;
	struct AVLNode *T2 = y->left;
	//     |                   |
	//     x                   y
	//    / \                 / \
	//  T1   y      ==>      x   T3
	//      / \             / \
	//    T2   T3         T1   T2
	x->right = T2;
	y->left = x;
	update_height(x);
	update_height(y);
	return y;
}

// Assumes x->right != NULL
struct AVLNode *rightRotate(struct AVLNode *x)
{
	struct AVLNode *y = x->left;
	struct AVLNode *T2 = y->right;
	//       |                   |
	//       x                   y
	//      / \                 / \
	//     y   T3      ==>    T1   x
	//    / \                     / \
	//  T1   T2                 T2   T3
	x->left = T2;
	y->right = x;
	update_height(x);
	update_height(y);
	return y;
}

struct AVLNode *rebalanceTree(void *object, struct AVLNode *x,
	AVLTree *pTree)
{
	int balance;
	if (x == NULL)
		return NULL;
	x->height = 1 + max(height(x->left), height(x->right));
	balance = balance(x);
	if (balance < -1) {
		//    |
		//    x
		//   / \
		// T1*  T2
		assert(x->left);
		balance = balance(x->left);
		if (balance < -1) {
			//      |
			//      x
			//     / \
			//    y   T3
			//   / \
			// T1*  T2
			assert(x->left->left);
			return rightRotate(x);
		} else if (balance > 1) {
			//      |
			//      x
			//     / \
			//    y   T3
			//   / \
			// T1   T2*
			assert(x->left->right);
			x->left = leftRotate(x->left);
			return rightRotate(x);
		}
	} else if (balance > 1) {
		//    |
		//    x
		//   / \
		// T1   T2*
		assert(x->right);
		balance = balance(x->right);
		if (balance > 1) {
			//    |
			//    x
			//   / \
			// T1   y
			//     / \
			//   T2   T3*
			assert(x->right->right);
			return leftRotate(x);
		} else if (balance < -1) {
			//    |
			//    x
			//   / \
			// T1   y
			//     / \
			//   T2*  T3
			assert(x->right->left);
			x->right = rightRotate(x->right);
			return leftRotate(x);
		}
	}
	return x;
}

// node: node to be inserted
// x: current node being analysed
// pTree: avl tree object
// pDup: (return) whether there is a duplicate or not
// return: node to be in position of x
struct AVLNode *insertNode(struct AVLNode *node, struct AVLNode *x,
	AVLTree *pTree, int *pDup)
{
	int cmp;
	if (x == NULL)
		return node;
	cmp = CMP(node->object, x->object, pTree);
	if (cmp < 0) {
		//    |
		//    x
		//   / \
		// T1*  T2
		x->left = insertNode(node, x->left, pTree, pDup);
	} else if (cmp > 0) {
		//    |
		//    x
		//   / \
		// T1   T2*
		x->right = insertNode(node, x->right, pTree, pDup);
	} else {
		// Duplicate object
		*pDup = 1;
	}
	if (*pDup) return x;
	return rebalanceTree(node->object, x, pTree);
}

AVLReturnId avlInsert(AVLTree *pTree, void *object)
{
	struct AVLNode *node;
	int duplicate = 0;
	if (pTree == NULL)
		return AVL_RETURN_INVALID_PARAMETER;
	node = create_node(object);
	if (node == NULL)
		return AVL_RETURN_MEMORY;
	pTree->root = insertNode(node, pTree->root, pTree, &duplicate);
	if (duplicate) {
		free(node);
		return AVL_RETURN_DUPLICATE;
	}
	return AVL_RETURN_OK;
}

int searchNode(struct AVLNode *node, void *object, AVLTree *pTree)
{
	int cmp;
	if (node == NULL)
		return 0;
	cmp = CMP(object, node->object, pTree);
	if (cmp < 0)
		return searchNode(node->left, object, pTree);
	else if (cmp > 0)
		return searchNode(node->right, object, pTree);
	else
		return 1;
}

AVLReturnId avlSearch(AVLTree *pTree, void *object, int *pExists)
{
	if (pTree == NULL || pExists == NULL)
		return AVL_RETURN_INVALID_PARAMETER;
	*pExists = searchNode(pTree->root, object, pTree);
	return AVL_RETURN_OK;
}

void nodeTraverse(struct AVLNode *node, void (*visit_cb)(void *object))
{
	if (node == NULL)
		return;
	nodeTraverse(node->left, visit_cb);
	visit_cb(node->object);
	nodeTraverse(node->right, visit_cb);
}

AVLReturnId avlTraverse(AVLTree *pTree, void (*visit_cb)(void *object))
{
	if (pTree == NULL || visit_cb == NULL)
		return AVL_RETURN_INVALID_PARAMETER;
	nodeTraverse(pTree->root, visit_cb);
	return AVL_RETURN_OK;
}

// assumes node isn't NULL
struct AVLNode *treeMin(struct AVLNode *node)
{
	for (; node->left; node = node->left);
	return node;
}

struct AVLNode *deleteNode(struct AVLNode *x, void *object,
	AVLTree *pTree, int *pExists, int freeObject)
{
	if (x == NULL)
		return NULL;
	int cmp = CMP(object, x->object, pTree);
	if (cmp < 0) {
		//    |
		//    x
		//   / \
		// T1*  T2
		x->left = deleteNode(x->left, object, pTree, pExists, freeObject);
	} else if (cmp > 0) {
		//    |
		//    x
		//   / \
		// T1   T2*
		x->right = deleteNode(x->right, object, pTree, pExists, freeObject);
	} else {
		struct AVLNode *retnode;
		*pExists = 1;
		if (x->left == NULL && x->right == NULL) {
			//   |            |
			//   x    ==>    NULL
			//
			retnode = NULL;
		} else if (x->left == NULL) {
			//   |            |
			//   x            o
			//    \   ==>   
			//     o
			retnode = x->right;
		} else if (x->right == NULL) {
			//   |            |
			//   x    ==>     o
			//  /
			// o
			retnode = x->left;
		} else {
			//    |           |
			//    x   ==>   min(T2)
			//   / \         / \
			// T1   T2     T1   T2-min(T2)
			struct AVLNode *temp = treeMin(x->right);
			x->object = temp->object;
			x->right = deleteNode(x->right, x->object,
				pTree, pExists, 0);
			retnode = x;
		}
		if (freeObject && pTree->freeObj)
			pTree->freeObj(x->object);
		free(x);
		x = retnode;
	}
	return rebalanceTree(object, x, pTree);
}

AVLReturnId avlDelete(AVLTree *pTree, void *object)
{
	int exists = 0;
	if (pTree == NULL)
		return AVL_RETURN_INVALID_PARAMETER;
	pTree->root = deleteNode(pTree->root, object, pTree, &exists, 1);
	if (!exists)
		return AVL_RETURN_NODE_NOT_FOUND;
	return AVL_RETURN_OK;
}

void avlDestroyNode(AVLTree *pTree, struct AVLNode *node)
{
	struct AVLNode *left, *right;
	if (node == NULL)
		return;
	left = node->left;
	right = node->right;
	if (pTree->freeObj)
		pTree->freeObj(node->object);
	free(node);
	avlDestroyNode(pTree, left);
	avlDestroyNode(pTree, right);
}

void avlDestroy(AVLTree *pTree)
{
	if (pTree == NULL)
		return;
	avlDestroyNode(pTree, pTree->root);
	free(pTree);
}