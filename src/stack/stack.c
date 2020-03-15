#include "stack.h"

#include <stdlib.h>

#include "memdb.h"

struct StackItem
{
	void *object;
	struct StackItem *next;
};

struct Stack
{
	struct StackItem *first;
};

StackReturnID stackCreate(Stack **ppStack)
{
	Stack *st;
	st = malloc(sizeof(struct Stack));
	if (st == NULL)
		return STACK_MEMORY;
	st->first = NULL;
	*ppStack = st;
	return STACK_OK;
}

struct StackItem *alloc_item(void *object, struct StackItem *first)
{
	struct StackItem *item;
	item = malloc(sizeof(struct StackItem));
	if (item) {
		item->object = object;
		item->next = first;
	}
	return item;
}

StackReturnID stackAdd(Stack *pStack, void *object)
{
	struct StackItem *item = alloc_item(object, pStack->first);
	if (object == NULL)
		return STACK_MEMORY;
	pStack->first = item;
	return STACK_OK;
}

StackReturnID stackEmpty(Stack *pStack, int *pIsEmpty)
{
	*pIsEmpty = !pStack->first;
	return STACK_OK;
}

StackReturnID stackRemove(Stack *pStack, void **pObject)
{
	struct StackItem *first = pStack->first;
	if (!first) return STACK_EMPTY;
	*pObject = first->object;
	pStack->first = first->next;
	free(first);
	return STACK_OK;
}

StackReturnID stackDestroy(Stack *pStack, void freeObject(void *))
{
	struct StackItem *current, *next;
	for (current = pStack->first; current; current = next) {
		next = current->next;
		if (freeObject)
			freeObject(current->object);
		free(current);
	}
	free(pStack);
	return STACK_OK;
}
