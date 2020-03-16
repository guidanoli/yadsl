#include "avl.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "memdb.h"

#ifndef abs
#define abs(a) (a > 0 ? a : -a)
#endif /* abs */

#ifndef max
#define max(a, b) ((a > b) ? a : b)
#endif /* max */

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

// check AVL tree invariants
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

struct Node
{
	struct Node *left;
	struct Node *right;
	int height; // 2^INT_MAX nodes
	void *object;
};

struct AVL
{
	struct Node *root;
	int (*cmpObjs)(void *obj1, void *obj2, void *arg);
	void (*freeObj)(void *object);
	void *arg;
};

AVLRet avlCreate(AVL **ppTree,
	int (*cmpObjs)(void *obj1, void *obj2, void *arg),
	void (*freeObj)(void *object), void *arg)
{
	AVL *pTree = malloc(sizeof(struct AVL));
	if (pTree == NULL)
		return AVL_MEMORY;
	pTree->root = NULL;
	pTree->cmpObjs = cmpObjs;
	pTree->freeObj = freeObj;
	pTree->arg = arg;
	*ppTree = pTree;
	return AVL_OK;
}

struct Node *create_node(void *object)
{
	struct Node *node = malloc(sizeof(struct Node));
	if (node) {
		node->height = 1;
		node->left = NULL;
		node->right = NULL;
		node->object = object;
	}
	return node;
}

// Assumes x->left != NULL
struct Node *leftRotate(struct Node *x)
{
	struct Node *y = x->right;
	struct Node *T2 = y->left;
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
struct Node *rightRotate(struct Node *x)
{
	struct Node *y = x->left;
	struct Node *T2 = y->right;
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

struct Node *rebalanceTree(void *object, struct Node *x,
	AVL *pTree)
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
		if (balance <= 0) {
			//      |
			//      x
			//     / \
			//    y   T3
			//   / \
			// T1*  T2
			assert(x->left->left);
			return rightRotate(x);
		} else {
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
		if (balance >= 0) {
			//    |
			//    x
			//   / \
			// T1   y
			//     / \
			//   T2   T3*
			assert(x->right->right);
			return leftRotate(x);
		} else {
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
	check_invariants(x);
	return x;
}

// node: node to be inserted
// x: current node being analysed
// pTree: avl tree object
// pDup: (return) whether there is a duplicate or not
// return: node to be in position of x
struct Node *insertNode(struct Node *node, struct Node *x,
	AVL *pTree, int *pDup)
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

AVLRet avlInsert(AVL *pTree, void *object, int *pExists)
{
	int exists = 0;
	struct Node *node = create_node(object);
	if (node == NULL)
		return AVL_MEMORY;
	pTree->root = insertNode(node, pTree->root, pTree, &exists);
	if (exists)
		free(node);
	if (pExists)
		*pExists = exists;
	return AVL_OK;
}

int searchNode(struct Node *node, void *object, AVL *pTree)
{
	int cmp;
	if (node == NULL)
		return 0;
	check_invariants(node);
	cmp = CMP(object, node->object, pTree);
	if (cmp < 0)
		return searchNode(node->left, object, pTree);
	else if (cmp > 0)
		return searchNode(node->right, object, pTree);
	else
		return 1;
}

AVLRet avlSearch(AVL *pTree, void *object, int *pExists)
{
	int exists = searchNode(pTree->root, object, pTree);
	if (pExists)
		*pExists = exists;
	return AVL_OK;
}

void *nodeTraverse(struct Node *node,
	void * (*visit_cb)(void *object, void *arg), void *arg)
{
	void *ret;
	if (node == NULL)
		return NULL;
	if (ret = nodeTraverse(node->left, visit_cb, arg))
		return ret;
	check_invariants(node);
	if (visit_cb && (ret = visit_cb(node->object, arg)))
		return ret;
	if (ret = nodeTraverse(node->right, visit_cb, arg))
		return ret;
	return NULL;
}

AVLRet avlTraverse(AVL *pTree,
	void * (*visit_cb)(void *object, void *arg),
	void *arg, void **pReturn)
{
	void *ret = nodeTraverse(pTree->root, visit_cb, arg);
	if (pReturn)
		*pReturn = ret;
	return AVL_OK;
}

// assumes node isn't NULL
struct Node *treeMin(struct Node *node)
{
	for (; node->left; node = node->left);
	return node;
}

struct Node *deleteNode(struct Node *x, void *object,
	AVL *pTree, int *pExists, int freeObject)
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
		struct Node *retnode;
		if (pExists)
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
			retnode = treeMin(x->right);
			retnode->right = deleteNode(x->right,
				retnode->object, pTree, pExists, 0);
			retnode->left = x->left;
		}
		if (freeObject) {
			if (pTree->freeObj)
				pTree->freeObj(x->object);
			free(x);
		}
		x = retnode;
	}
	return rebalanceTree(object, x, pTree);
}

AVLRet avlDelete(AVL *pTree, void *object, int *pExists)
{
	if (pExists)
		*pExists = 0;
	pTree->root = deleteNode(pTree->root, object, pTree, pExists, 1);
	return AVL_OK;
}

void avlDestroyNode(AVL *pTree, struct Node *node)
{
	struct Node *left, *right;
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

void avlDestroy(AVL *pTree)
{
	if (pTree == NULL)
		return;
	avlDestroyNode(pTree, pTree->root);
	free(pTree);
}
