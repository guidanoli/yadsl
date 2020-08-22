#include <set/set.h>

#include <stdlib.h>

#include <memdb/memdb.h>

// A set is represented by an ordered double-linked list
// in which each item is an opaque pointer.
// It keeps track of the number of items in the set and
// makes sure no duplicates are added.

struct SetItem
{
	struct SetItem *next;
	struct SetItem *previous;
	void *item;
};

struct Set
{
	struct SetItem *cursor;  /* For external use */
	struct SetItem *current; /* For internal use */
	struct SetItem *first;
	struct SetItem *last;
	size_t size;
	size_t modificationCount; /* Modification count */
	size_t callbackDepth; /* Callback depth */
};

// Modification logging

#define SetLogModification(pSet) do { \
	if (pSet->callbackDepth) \
		++pSet->modificationCount; \
} while(0)

#define SetGetModificationCount(pSet) pSet->modificationCount
#define SetEnableLog(pSet) ++pSet->callbackDepth
#define SetDisableLog(pSet) --pSet->callbackDepth

// Private functions prototypes

static SetRet _setContains(Set *pSet, void *item,
	struct SetItem **pSetItem);

// Public functions

SetRet setCreate(Set **ppSet)
{
	struct Set *pSet = malloc(sizeof(struct Set));
	if (pSet == NULL)
		return SET_MEMORY;
	pSet->current = NULL;
	pSet->cursor = NULL;
	pSet->first = NULL;
	pSet->last = NULL;
	pSet->size = 0;
	pSet->callbackDepth = 0;
	pSet->modificationCount = 0;
	*ppSet = pSet;
	return SET_OK;
}

SetRet setContainsItem(Set *pSet, void *item)
{
	struct SetItem *p; // does nothing with p
	return _setContains(pSet, item, &p);
}

SetRet setFilterItem(Set *pSet, int (*func) (void *item, void *arg),
	void *arg, void **pItem)
{
	struct SetItem *p;
	size_t size, matches, mod_cnt;
	size = pSet->size;
	for (p = pSet->current;
		size-- && p;
		p = p->next ? p->next : pSet->first) {
		mod_cnt = SetGetModificationCount(pSet);
		SetEnableLog(pSet);
		matches = func(p->item, arg);
		SetDisableLog(pSet);
		if (mod_cnt != SetGetModificationCount(pSet))
			return setFilterItem(pSet, func, arg, pItem);
		if (matches) {
			*pItem = p->item;
			return SET_OK;
		}
	}
	return SET_DOES_NOT_CONTAIN;
}

SetRet setAddItem(Set *pSet, void *item)
{
	struct SetItem *pItem, *p;
	if (setContainsItem(pSet, item) == SET_CONTAINS)
		return SET_CONTAINS;
	pItem = malloc(sizeof(struct SetItem));
	if (pItem == NULL)
		return SET_MEMORY;
	pItem->next = NULL;
	pItem->previous = NULL;
	pItem->item = item;
	p = pSet->current;
	pSet->current = pItem;
	if (p == NULL) {
		// empty set
		pSet->cursor = pItem;
		pSet->first = pItem;
		pSet->last = pItem;
	} else {
		char direction = 0;
		char current_direction;
		do {
			current_direction = p->item > item ? -1 : 1;
			if (current_direction == -1)
				p = p->previous;
			if (current_direction == -direction) {
				// insert pItem after p
				pItem->previous = p;
				pItem->next = p->next;
				if (p->next != NULL)
					p->next->previous = pItem;
				else
					pSet->last = pItem;
				p->next = pItem;
				goto exit;
			}
			if (current_direction == 1)
				p = p->next;
			direction = current_direction;
		} while (p != NULL);
		if (direction == -1) {
			// insert pItem at the start
			pItem->next = pSet->first;
			pSet->first->previous = pItem;
			pSet->first = pItem;
		} else if (direction == 1) {
			// insert pItem at the end
			pItem->previous = pSet->last;
			pSet->last->next = pItem;
			pSet->last = pItem;
		}
	}
exit:
	pSet->size = pSet->size + 1;
	SetLogModification(pSet);
	return SET_OK;
}

SetRet setRemoveItem(Set *pSet, void *item)
{
	struct SetItem *p;
	if (_setContains(pSet, item, &p) == SET_DOES_NOT_CONTAIN)
		return SET_DOES_NOT_CONTAIN;
	if (p->next == NULL)
		pSet->last = p->previous;
	if (p->previous != NULL) {
		if (p == pSet->current)
			pSet->current = p->previous;
		if (p == pSet->cursor)
			pSet->cursor = p->previous;
		p->previous->next = p->next;
	} else {
		if (p == pSet->current)
			pSet->current = p->next;
		if (p == pSet->cursor)
			pSet->cursor = p->next;
		pSet->first = p->next;
	}
	if (p->next != NULL) {
		p->next->previous = p->previous;
	}
	free(p);
	pSet->size = pSet->size - 1;
	SetLogModification(pSet);
	return SET_OK;
}

SetRet setGetCurrentItem(Set *pSet, void **pItem)
{
	if (pSet->cursor == NULL)
		return SET_EMPTY;
	*pItem = pSet->cursor->item;
	return SET_OK;
}

SetRet setGetSize(Set *pSet, size_t *pSize)
{
	*pSize = pSet->size;
	return SET_OK;
}

SetRet setPreviousItem(Set *pSet)
{
	if (pSet->cursor == NULL)
		return SET_EMPTY;
	if (pSet->cursor->previous == NULL)
		return SET_OUT_OF_BOUNDS;
	pSet->cursor = pSet->cursor->previous;
	return SET_OK;
}

SetRet setNextItem(Set *pSet)
{
	if (pSet->cursor == NULL)
		return SET_EMPTY;
	if (pSet->cursor->next == NULL)
		return SET_OUT_OF_BOUNDS;
	pSet->cursor = pSet->cursor->next;
	return SET_OK;
}

SetRet setFirstItem(Set *pSet)
{
	if (pSet->cursor == NULL)
		return SET_EMPTY;
	pSet->cursor = pSet->first;
	return SET_OK;
}

SetRet setLastItem(Set *pSet)
{
	if (pSet->cursor == NULL)
		return SET_EMPTY;
	pSet->cursor = pSet->last;
	return SET_OK;
}

void setDestroy(Set *pSet)
{
	setDestroyDeep(pSet, NULL, NULL);
}

void setDestroyDeep(Set *pSet, void (*freeItem)(void *item, void *arg),
	void *arg)
{
	struct SetItem *current, *next;
	if (pSet == NULL)
		return;
	current = pSet->first;
	next = NULL;
	while (current != NULL) {
		if (freeItem)
			freeItem(current->item, arg);
		next = current->next;
		free(current);
		current = next;
	}
	free(pSet);
}

// Private functions

// Checks if item is contained in the set and if it is, makes
// the pointer of address "pSetItem" point to it
static SetRet _setContains(Set *pSet, void *item,
	struct SetItem **pSetItem)
{
	char direction = 0;
	struct SetItem *p = pSet->current;
	while (p != NULL) {
		char current_direction;
		if (p->item > item) {
			p = p->previous;
			current_direction = -1;
		} else if (p->item < item) {
			p = p->next;
			current_direction = 1;
		} else {
			*pSetItem = p;
			return SET_CONTAINS;
		}
		if (current_direction == -direction)
			break;
		direction = current_direction;
	}
	return SET_DOES_NOT_CONTAIN;
}