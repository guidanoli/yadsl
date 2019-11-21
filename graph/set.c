#include <stdlib.h>
#include "set.h"

struct SetItem
{
    struct SetItem *next;
    struct SetItem *previous;
    size_t value;
};

struct Set
{
    struct SetItem *current;
    struct SetItem *first;
    struct SetItem *last;
};

SetReturnID setCreate(Set **ppSet)
{
    if (ppSet == NULL)
        return SET_RETURN_INVALID_PARAMETER;
    struct Set *pSet = malloc(sizeof(Set));
    if (pSet == NULL)
        return SET_RETURN_MEMORY;
    pSet->current = NULL;
    pSet->first = NULL;
    pSet->last = NULL;
    *ppSet = pSet;
    return SET_RETURN_OK;
}

// If set contains item, sets it as current item
SetReturnID setContains(Set *pSet, size_t value)
{
    if (pSet == NULL)
        return SET_RETURN_INVALID_PARAMETER;
    char direction = 0;
    struct SetItem *p = pSet->current;
    SetReturnID contains = SET_RETURN_DOES_NOT_CONTAIN;
    while (p != NULL) {
        char current_direction;
        if (p->value > value) {
            p = p->previous;
            current_direction = -1;
        } else if (p->value < value) {
            p = p->next;
            current_direction = 1;
        } else {
            pSet->current = p;
            return SET_RETURN_CONTAINS;
        }
        if (current_direction == -direction)
            return SET_RETURN_DOES_NOT_CONTAIN;
        direction = current_direction;
    }
    return contains;
}

SetReturnID setAdd(Set *pSet, size_t value)
{
    if (pSet == NULL)
        return SET_RETURN_INVALID_PARAMETER;
    if (setContains(pSet, value) == SET_RETURN_CONTAINS)
        return SET_RETURN_CONTAINS;
    struct SetItem *pItem = malloc(sizeof(struct SetItem));
    if (pItem == NULL)
        return SET_RETURN_MEMORY;
    pItem->next = NULL;
    pItem->previous = NULL;
    pItem->value = value;
    struct SetItem *p = pSet->current;
    pSet->current = pItem;
    if (p == NULL) {
        // empty set
        pSet->first = pItem;
        pSet->last = pItem;
    } else {
        char direction = 0;
        char current_direction;
        do {
            current_direction = p->value > value ? -1 : 1;
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
    return SET_RETURN_OK;
}

SetReturnID setRemove(Set *pSet, size_t value)
{
    if (pSet == NULL)
        return SET_RETURN_INVALID_PARAMETER;
    if (setContains(pSet, value) == SET_RETURN_DOES_NOT_CONTAIN)
        return SET_RETURN_DOES_NOT_CONTAIN;
    struct SetItem *p = pSet->current;
    if (p->next == NULL)
        pSet->last = p->previous;
    if (p->previous != NULL) {
        pSet->current = p->previous;
        p->previous->next = p->next;
    } else {
        pSet->current = p->next;
        pSet->first = p->next;
        if (p->next != NULL)
            p->next->previous = p->previous;
    }
    return SET_RETURN_OK;
}

void setDestroy(Set *pSet)
{
    if (pSet == NULL)
        return;
    struct SetItem *current = pSet->first;
    struct SetItem *next = NULL;
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    free(pSet);
}
