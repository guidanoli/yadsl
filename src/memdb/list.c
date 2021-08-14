#include <memdb/list.h>

#include <stddef.h>
#include <assert.h>

void list_append(ListNode** head_ptr, ListNode* node)
{
	assert(head_ptr != NULL);
	assert(node != NULL);
	node->next = *head_ptr;
	*head_ptr = node;
}

ListNode** list_find(ListNode** head_ptr, ListNode* node)
{
	ListNode* current, *previous;
	assert(head_ptr != NULL);
	current = *head_ptr;
	previous = NULL;
	while (current != NULL) {
		if (current == node) {
			if (previous == NULL) {
				return head_ptr;
			} else {
				return &previous->next;
			}
		}
		previous = current;
		current = current->next;
	}
	return NULL;
}

ListNode* list_remove(ListNode** node_ptr)
{
	ListNode* node;
	assert(node_ptr != NULL);
	node = *node_ptr;
	assert(node != NULL);
	*node_ptr = node->next;
	node->next = NULL;
	return node;
}
