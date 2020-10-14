#include <stack/stack.h>

#ifdef YADSL_DEBUG
#include <memdb/memdb.h>
#else
#include <stdlib.h>
#endif

struct yadsl_StackItem_s
{
	yadsl_StackItemObj* object;
	struct yadsl_StackItem_s* next;
};

typedef struct yadsl_StackItem_s yadsl_StackItem;

typedef struct
{
	yadsl_StackItem* first;
}
yadsl_Stack;

yadsl_StackHandle*
yadsl_stack_create()
{
	yadsl_Stack* stack = malloc(sizeof(*stack));
	if (stack)
		stack->first = NULL;
	return stack;
}

static yadsl_StackItem*
yadsl_stack_item_create_internal(
	yadsl_StackItemObj* object,
	yadsl_StackItem* first)
{
	yadsl_StackItem* item = malloc(sizeof(*item));
	if (item) {
		item->object = object;
		item->next = first;
	}
	return item;
}

yadsl_StackRet
yadsl_stack_item_add(
	yadsl_StackHandle* stack,
	yadsl_StackItemObj* object)
{
	yadsl_Stack* stack_ = (yadsl_Stack*) stack;
	yadsl_StackItem* item = yadsl_stack_item_create_internal(object, stack_->first);
	if (item == NULL)
		return YADSL_STACK_RET_MEMORY;
	stack_->first = item;
	return YADSL_STACK_RET_OK;
}

yadsl_StackRet
yadsl_stack_empty_check(
	yadsl_StackHandle* stack,
	bool* is_empty_ptr)
{
	*is_empty_ptr = ((yadsl_Stack*) stack)->first == NULL;
	return YADSL_STACK_RET_OK;
}

yadsl_StackRet
yadsl_stack_item_remove(
	yadsl_StackHandle* stack,
	yadsl_StackItemObj** object_ptr)
{
	yadsl_Stack* stack_ = (yadsl_Stack*) stack;
	yadsl_StackItem* first = stack_->first;
	if (first == NULL)
		return YADSL_STACK_RET_EMPTY;
	*object_ptr = first->object;
	stack_->first = first->next;
	free(first);
	return YADSL_STACK_RET_OK;
}

void
yadsl_stack_destroy(
	yadsl_StackHandle* stack,
	yadsl_StackItemFreeFunc free_item_func)
{
	yadsl_StackItem* current, * next;
	for (current = ((yadsl_Stack*) stack)->first; current; current = next) {
		next = current->next;
		if (free_item_func)
			free_item_func(current->object);
		free(current);
	}
	free(stack);
}
