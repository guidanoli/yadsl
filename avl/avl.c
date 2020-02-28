#include "avl.h"

#include <stddef.h>
#include <stdlib.h>
#include <assert.h>

#ifdef _DEBUG
#include "memdb.h"
#endif

// If positive, o1 < o2. If negative, o1 >= o2.
#define CMP(o1, o2, t) (t->cmpObjs ? t->cmpObjs(o1, o2, t->arg) : \
 (o1 < o2 ? -1 : (o1 == o2 ? 0 : 1)))

// height of a node
#define height(node) (node ? node->height : 0)

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
		node->height = 0;
		node->left = NULL;
		node->right = NULL;
		node->object = object;
	}
	return node;
}

// Assumes x->left != NULL
struct AVLNode *leftRotate(struct AVLNode *x)
{
	struct AVLNode *y = x->left;
	struct AVLNode *T2 = y->right;
	//     |                   |
	//     x                   y
	//    / \                 / \
	//  T1   y      ==>      x   T3
	//      / \             / \
	//    T2   T3         T1   T2
	x->right = T2;
	y->left = x;
	return y;
}

// Assumes x->right != NULL
struct AVLNode *rightRotate(struct AVLNode *x)
{
	struct AVLNode *y = x->right;
	struct AVLNode *T2 = y->left;
	//       |                   |
	//       x                   y
	//      / \                 / \
	//     y   T3      ==>    T1   x
	//    / \                     / \
	//  T1   T2                 T2   T3
	x->left = T2;
	y->right = x;
	return y;
}

struct AVLNode *insertNode(struct AVLNode *node, struct AVLNode *x,
	AVLTree *pTree, int *pDup)
{
	int cmp, balance;
	if (x == NULL)
		return node;
	cmp = CMP(x->object, node->object, pTree);
	if (cmp > 0) {
		//    |
		//    x
		//   / \
		// T1*  T2
		x->left = insertNode(node, x->left, pTree, pDup);
	} else if (cmp < 0) {
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
	x->height = 1 + max(height(x->left), height(x->right));
	balance = height(x->right) - height(x->left);
	if (balance < -1) {
		//    |
		//    x
		//   / \
		// T1*  T2
		assert(x->left);
		cmp = CMP(x->left->object, node->object, pTree);
		if (cmp > 0) {
			//      |
			//      x
			//     / \
			//    y   T3
			//   / \
			// T1*  T2
			return rightRotate(x);
		} if (cmp < 0) {
			//      |
			//      x
			//     / \
			//    y   T3
			//   / \
			// T1   T2*
			x->left = leftRotate(x->left);
			return rightRotate(x);
		}
	} else if (balance > 1) {
		//    |
		//    x
		//   / \
		// T1   T2*
		assert(x->right);
		cmp = CMP(x->right->object, x->object, pTree);
		if (cmp < 0) {
			//    |
			//    x
			//   / \
			// T1   y
			//     / \
			//   T2   T3*
			return leftRotate(x);
		} if (cmp > 0) {
			//    |
			//    x
			//   / \
			// T1   y
			//     / \
			//   T2*  T3
			x->right = rightRotate(x->right);
			return leftRotate(x);
		}
	}
	return x;
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

// assumes node isn't NULL
struct AVLNode *get_max(struct AVLNode *node, struct AVLNode **pParent)
{
	for (; node->right; *pParent = node, node = node->right);
	return node;
}

// assumes node isn't NULL
struct AVLNode *get_min(struct AVLNode *node, struct AVLNode **pParent)
{
	for (; node->left; *pParent = node, node = node->left);
	return node;
}

//struct AVLNode *deleteNode(struct AVLNode *node, void *object,
//	struct AVLNode *parent, AVLTree *pTree, int *pExists)
//{
//	if (node) {
//		int cmp = CMP(object, node->object, pTree);
//		if (cmp < 0) {
//			node->left = deleteNode(node->left, object, node, pTree, pExists);
//		} else if (cmp > 0) {
//			node->right = deleteNode(node->right, object, node, pTree, pExists);
//		} else {
//			struct AVLNode *retnode;
//			*pExists = 1;
//			if (node->left == NULL && node->right == NULL) {
//				retnode = NULL;
//			} else if (node->left == NULL) {
//				retnode = node->right;
//			} else if (node->right == NULL) {
//				retnode = node->left;
//			} else {
//				struct AVLNode *parent = NULL;
//				if (node->left->height > node->right->height) {
//					retnode = get_max(node->left, &parent);
//					retnode->right = node->right;
//					if (retnode == node->left)
//						retnode->left = NULL;
//					else
//						retnode->left = node->left;
//				} else {
//					retnode = get_min(node->right, &parent);
//					retnode->left = node->left;
//					if (retnode == node->right)
//						retnode->right = NULL;
//					else
//						retnode->right = node->right;
//				}
//				if (parent) {
//					if (parent->left == retnode)
//						parent->left = NULL;
//					else
//						parent->right = NULL;
//				}
//			}
//			if (pTree->freeObj)
//				pTree->freeObj(node->object);
//			free(node);
//			return retnode;
//		}
//	}
//	return node;
//}
//
//AVLReturnId avlDelete(AVLTree *pTree, void *object)
//{
//	int exists = 0;
//	if (pTree == NULL)
//		return AVL_RETURN_INVALID_PARAMETER;
//	pTree->root = deleteNode(pTree->root, object, NULL, pTree, &exists);
//	if (!exists)
//		return AVL_RETURN_NODE_NOT_FOUND;
//
//}

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