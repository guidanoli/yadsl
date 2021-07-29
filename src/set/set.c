#include <set/set.h>

#include <yadsl/stdlib.h>

struct yadsl_SetItem_s
{
	struct yadsl_SetItem_s* next;
	struct yadsl_SetItem_s* previous;
	yadsl_SetItemObj* item;
};

typedef struct yadsl_SetItem_s yadsl_SetItem;

typedef struct
{
	yadsl_SetItem* external_cursor;
	yadsl_SetItem* internal_cursor;
	yadsl_SetItem* first;
	yadsl_SetItem* last;
	size_t size;
}
yadsl_Set;


// Private functions prototypes

static yadsl_SetRet
yadsl_set_item_contains_check_internal(
	yadsl_SetHandle* set,
	void* item,
	yadsl_SetItem** set_item_ptr);

// Public functions

yadsl_SetHandle*
yadsl_set_create()
{
	yadsl_Set* set = malloc(sizeof(*set));
	if (set) {
		set->internal_cursor = NULL;
		set->external_cursor = NULL;
		set->first = NULL;
		set->last = NULL;
		set->size = 0;
	}
	return set;
}

yadsl_SetRet
yadsl_set_item_contains_check(
	yadsl_SetHandle* set,
	void* item)
{
	return yadsl_set_item_contains_check_internal(set, item, NULL);
}

yadsl_SetRet yadsl_set_item_filter(
	yadsl_SetHandle* set,
	yadsl_SetItemFilterFunc item_filter_func,
	yadsl_SetItemFilterArg* item_filter_arg,
	yadsl_SetItemObj** item_ptr)
{
	yadsl_SetItem* p;
	yadsl_Set* set_ = (yadsl_Set*) set;
	size_t size = set_->size;

	for (
		p = set_->internal_cursor; /* Start at current position */
		size-- && p; /* Until all items have been visited */
		p = p->next ? p->next : set_->first /* Cycle through */
		) {
		if (item_filter_func(p->item, item_filter_arg)) {
			*item_ptr = p->item;
			return YADSL_SET_RET_OK;
		}
	}

	return YADSL_SET_RET_DOES_NOT_CONTAIN;
}

yadsl_SetRet
yadsl_set_item_add(
	yadsl_SetHandle* set,
	yadsl_SetItemObj* item)
{
	yadsl_SetItem* set_item, * p;
	yadsl_Set* set_ = (yadsl_Set*) set;

	if (yadsl_set_item_contains_check(set, item) == YADSL_SET_RET_CONTAINS)
		return YADSL_SET_RET_CONTAINS;

	set_item = malloc(sizeof(*set_item));
	if (set_item == NULL)
		return YADSL_SET_RET_MEMORY;

	set_item->next = NULL;
	set_item->previous = NULL;
	set_item->item = item;

	p = set_->internal_cursor;
	set_->internal_cursor = set_item;

	if (p == NULL) {
		// empty set
		set_->external_cursor = set_item;
		set_->first = set_item;
		set_->last = set_item;
	} else {
		char direction = 0;
		char current_direction;

		do {
			current_direction = p->item > item ? -1 : 1;
			if (current_direction == -1)
				p = p->previous;
			if (current_direction == -direction) {
				// insert pItem after p
				set_item->previous = p;
				set_item->next = p->next;
				if (p->next != NULL)
					p->next->previous = set_item;
				else
					set_->last = set_item;
				p->next = set_item;
				goto exit;
			}
			if (current_direction == 1)
				p = p->next;
			direction = current_direction;
		} while (p != NULL);

		if (direction == -1) {
			// insert pItem at the start
			set_item->next = set_->first;
			set_->first->previous = set_item;
			set_->first = set_item;
		} else if (direction == 1) {
			// insert pItem at the end
			set_item->previous = set_->last;
			set_->last->next = set_item;
			set_->last = set_item;
		}
	}
exit:
	(set_->size)++;

	return YADSL_SET_RET_OK;
}

yadsl_SetRet
yadsl_set_item_remove(
	yadsl_SetHandle* set,
	yadsl_SetItemObj* item)
{
	yadsl_SetItem* p;
	yadsl_Set* set_ = (yadsl_Set*) set;

	if (yadsl_set_item_contains_check_internal(set, item, &p) == YADSL_SET_RET_DOES_NOT_CONTAIN)
		return YADSL_SET_RET_DOES_NOT_CONTAIN;

	if (p->next == NULL)
		set_->last = p->previous;
	if (p->previous != NULL) {
		if (p == set_->internal_cursor)
			set_->internal_cursor = p->previous;
		if (p == set_->external_cursor)
			set_->external_cursor = p->previous;
		p->previous->next = p->next;
	} else {
		if (p == set_->internal_cursor)
			set_->internal_cursor = p->next;
		if (p == set_->external_cursor)
			set_->external_cursor = p->next;
		set_->first = p->next;
	}
	if (p->next != NULL) {
		p->next->previous = p->previous;
	}

	free(p);
	set_->size = set_->size - 1;

	return YADSL_SET_RET_OK;
}

yadsl_SetRet
yadsl_set_cursor_get(
	yadsl_SetHandle* set,
	yadsl_SetItemObj** item_ptr)
{
	yadsl_Set* set_ = (yadsl_Set*) set;

	if (set_->external_cursor == NULL)
		return YADSL_SET_RET_EMPTY;

	*item_ptr = set_->external_cursor->item;
	return YADSL_SET_RET_OK;
}

yadsl_SetRet
yadsl_set_size_get(
	yadsl_SetHandle* set,
	size_t* size_ptr)
{
	*size_ptr = ((yadsl_Set*) set)->size;
	return YADSL_SET_RET_OK;
}

yadsl_SetRet
yadsl_set_cursor_previous(
	yadsl_SetHandle* set)
{
	yadsl_Set* set_ = (yadsl_Set*) set;

	if (set_->external_cursor == NULL)
		return YADSL_SET_RET_EMPTY;

	if (set_->external_cursor->previous == NULL)
		return YADSL_SET_RET_OUT_OF_BOUNDS;

	set_->external_cursor = set_->external_cursor->previous;

	return YADSL_SET_RET_OK;
}

yadsl_SetRet
yadsl_set_cursor_next(
	yadsl_SetHandle* set)
{
	yadsl_Set* set_ = (yadsl_Set*) set;

	if (set_->external_cursor == NULL)
		return YADSL_SET_RET_EMPTY;

	if (set_->external_cursor->next == NULL)
		return YADSL_SET_RET_OUT_OF_BOUNDS;

	set_->external_cursor = set_->external_cursor->next;

	return YADSL_SET_RET_OK;
}

yadsl_SetRet
yadsl_set_cursor_first(
	yadsl_SetHandle* set)
{
	yadsl_Set* set_ = (yadsl_Set*) set;

	if (set_->external_cursor == NULL)
		return YADSL_SET_RET_EMPTY;

	set_->external_cursor = set_->first;

	return YADSL_SET_RET_OK;
}

yadsl_SetRet
yadsl_set_cursor_last(
	yadsl_SetHandle* set)
{
	yadsl_Set* set_ = (yadsl_Set*) set;

	if (set_->external_cursor == NULL)
		return YADSL_SET_RET_EMPTY;

	set_->external_cursor = set_->last;

	return YADSL_SET_RET_OK;
}

void
yadsl_set_destroy(
	yadsl_SetHandle* set,
	yadsl_SetItemFreeFunc free_item_func,
	yadsl_SetItemFreeArg* free_item_arg)
{
	yadsl_SetItem* internal_cursor, * next;
	yadsl_Set* set_ = (yadsl_Set*) set;

	if (set_ == NULL)
		return;

	internal_cursor = set_->first;
	next = NULL;

	while (internal_cursor != NULL) {
		if (free_item_func)
			free_item_func(internal_cursor->item, free_item_arg);
		next = internal_cursor->next;
		free(internal_cursor);
		internal_cursor = next;
	}

	free(set_);
}

// Private functions

// Checks if item is contained in the set and if it is, makes
// the pointer of address "set_item_ptr" point to it, if not NULL
yadsl_SetRet
yadsl_set_item_contains_check_internal(
	yadsl_SetHandle* set,
	void* item,
	yadsl_SetItem** set_item_ptr)
{
	char direction = 0;
	yadsl_SetItem* p = ((yadsl_Set*) set)->internal_cursor;
	while (p != NULL) {
		char current_direction;
		if (p->item > item) {
			p = p->previous;
			current_direction = -1;
		} else if (p->item < item) {
			p = p->next;
			current_direction = 1;
		} else {
			if (set_item_ptr)
				*set_item_ptr = p;
			return YADSL_SET_RET_CONTAINS;
		}
		if (current_direction == -direction)
			break;
		direction = current_direction;
	}
	return YADSL_SET_RET_DOES_NOT_CONTAIN;
}
