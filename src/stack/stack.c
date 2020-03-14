#include "stack.h"

#include <stdlib.h>

struct stack_item
{
	void *object;
	struct stack_item *next;
};

struct stack
{
	struct stack_item *first;
};

stack_return stack_create(stack **stack_ptr)
{
	stack *st;
	st = malloc(sizeof(struct stack));
	if (st == NULL)
		return STACK_MEMORY;
	st->first = NULL;
	*stack_ptr = st;
	return STACK_OK;
}

struct stack_item *alloc_item(void *object, struct stack_item *first)
{
	struct stack_item *item;
	item = malloc(sizeof(struct stack_item));
	if (item) {
		item->object = object;
		item->next = first;
	}
	return item;
}

stack_return stack_add(stack *stack, void *object)
{
	struct stack_item *item = alloc_item(object, stack->first);
	if (object == NULL)
		return STACK_MEMORY;
	stack->first = item;
	return STACK_OK;
}

stack_return stack_empty(stack *stack, int *is_empty_ptr)
{
	*is_empty_ptr = !stack->first;
	return STACK_OK;
}

stack_return stack_remove(stack *stack, void **object_ptr)
{
	struct stack_item *first = stack->first;
	if (!first) return STACK_EMPTY;
	*object_ptr = first->object;
	stack->first = first->next;
	free(first);
	return STACK_OK;
}

stack_return stack_destroy(stack *stack, void free_object(void *))
{
	struct stack_item *current, *next;
	for (current = stack->first; current; current = next) {
		next = current->next;
		if (free_object)
			free_object(current->object);
		free(current);
	}
	free(stack);
	return STACK_OK;
}
