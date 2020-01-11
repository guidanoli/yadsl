#include <stdlib.h>
#include "set.h"

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
    struct SetItem *current;
    struct SetItem *first;
    struct SetItem *last;
    unsigned long size;
};

// Private functions prototypes

static SetReturnID _setContains(Set *pSet, void *item, struct SetItem **pSetItem);

// Public functions

SetReturnID setCreate(Set **ppSet)
{
    struct Set *pSet;
    if (ppSet == NULL)
        return SET_RETURN_INVALID_PARAMETER;
    pSet = (struct Set*) malloc(sizeof(struct Set));
    if (pSet == NULL)
        return SET_RETURN_MEMORY;
    pSet->current = NULL;
    pSet->first = NULL;
    pSet->last = NULL;
    pSet->size = 0;
    *ppSet = pSet;
    return SET_RETURN_OK;
}

SetReturnID setContainsItem(Set *pSet, void *item)
{
    struct SetItem *p; // does nothing with p
    return _setContains(pSet, item, &p);
}

SetReturnID setAddItem(Set *pSet, void *item)
{
    struct SetItem *pItem, *p;
    if (pSet == NULL)
        return SET_RETURN_INVALID_PARAMETER;
    if (setContainsItem(pSet, item) == SET_RETURN_CONTAINS)
        return SET_RETURN_CONTAINS;
    pItem = (struct SetItem *) malloc(sizeof(struct SetItem));
    if (pItem == NULL)
        return SET_RETURN_MEMORY;
    pItem->next = NULL;
    pItem->previous = NULL;
    pItem->item = item;
    p = pSet->current;
    pSet->current = pItem;
    if (p == NULL) {
        // empty set
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
                pSet->size = pSet->size + 1;
                return SET_RETURN_OK;
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
    pSet->size = pSet->size + 1;
    return SET_RETURN_OK;
}

SetReturnID setRemoveItem(Set *pSet, void *item)
{
    struct SetItem *p;
    if (pSet == NULL)
        return SET_RETURN_INVALID_PARAMETER;
    if (_setContains(pSet, item, &p) == SET_RETURN_DOES_NOT_CONTAIN)
        return SET_RETURN_DOES_NOT_CONTAIN;
    if (p->next == NULL)
        pSet->last = p->previous;
    if (p->previous != NULL) {
        if (p == pSet->current)
            pSet->current = p->previous;
        p->previous->next = p->next;
    } else {
        if (p == pSet->current)
            pSet->current = p->next;
        pSet->first = p->next;
    }
    if (p->next != NULL) {
        p->next->previous = p->previous;
    }
    free(p);
    pSet->size = pSet->size - 1;
    return SET_RETURN_OK;
}

SetReturnID setGetCurrentItem(Set *pSet, void **pItem)
{
    if (pSet == NULL || pItem == NULL)
        return SET_RETURN_INVALID_PARAMETER;
    if (pSet->current == NULL)
        return SET_RETURN_EMPTY;
    *pItem = pSet->current->item;
    return SET_RETURN_OK;
}

SetReturnID setGetSize(Set *pSet, unsigned long *pValue)
{
    if (pSet == NULL || pValue == NULL)
        return SET_RETURN_INVALID_PARAMETER;
    *pValue = pSet->size;
    return SET_RETURN_OK;
}

SetReturnID setPreviousItem(Set *pSet)
{
    if (pSet == NULL)
        return SET_RETURN_INVALID_PARAMETER;
    if (pSet->current == NULL)
        return SET_RETURN_EMPTY;
    if (pSet->current->previous == NULL)
        return SET_RETURN_OUT_OF_BOUNDS;
    pSet->current = pSet->current->previous;
    return SET_RETURN_OK;
}

SetReturnID setNextItem(Set *pSet)
{
    if (pSet == NULL)
        return SET_RETURN_INVALID_PARAMETER;
    if (pSet->current == NULL)
        return SET_RETURN_EMPTY;
    if (pSet->current->next == NULL)
        return SET_RETURN_OUT_OF_BOUNDS;
    pSet->current = pSet->current->next;
    return SET_RETURN_OK;
}

SetReturnID setFirstItem(Set *pSet)
{
    if (pSet == NULL)
        return SET_RETURN_INVALID_PARAMETER;
    if (pSet->current == NULL)
        return SET_RETURN_EMPTY;
    pSet->current = pSet->first;
    return SET_RETURN_OK;
}

SetReturnID setLastItem(Set *pSet)
{
    if (pSet == NULL)
        return SET_RETURN_INVALID_PARAMETER;
    if (pSet->current == NULL)
        return SET_RETURN_EMPTY;
    pSet->current = pSet->last;
    return SET_RETURN_OK;
}

void setDestroy(Set *pSet)
{
    struct SetItem *current, *next;
    if (pSet == NULL)
        return;
    current = pSet->first;
    next = NULL;
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    free(pSet);
}

// Private functions

// Checks if item is contained in the set and if it is, makes
// the pointer of address "pSetItem" point to it
static SetReturnID _setContains(Set *pSet, void *item, struct SetItem **pSetItem)
{
    char direction = 0;
    struct SetItem *p;
    if (pSet == NULL || pSetItem == NULL)
        return SET_RETURN_INVALID_PARAMETER;
    p = pSet->current;
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
            return SET_RETURN_CONTAINS;
        }
        if (current_direction == -direction)
            break;
        direction = current_direction;
    }
    return SET_RETURN_DOES_NOT_CONTAIN;
}
