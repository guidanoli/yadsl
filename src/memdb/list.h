#ifndef yadsl_memdb_list_h
#define yadsl_memdb_list_h

#include <stdbool.h>

/* Generic interface for single-linked headless lists
 * Every node should have as its first field a pointer to a 'next' node
 * We use pointers to pointers to nodes as a way to be able to override
 * the head (which is a node too)
 *
 * This generic interface is safe because C standard guarantees that a
 * pointer to a struct is equivalent to a pointer to the first member
 * of the struct (ergo, the 'next' member of a ListNode has the same
 * address as the ListNode itself). */

struct ListNode
{
    struct ListNode* next;
};

typedef struct ListNode ListNode;

/**
 * @brief Append node to beggining of list
 * @param head_ptr address of head node (not NULL)
 * @param node node to be appended (not NULL)
 * @note After calling ::list_append, head_ptr will be
 * pointing to node
*/
void list_append(ListNode** head_ptr, ListNode* node);

/**
 * @brief Remove node from list
 * @param node_ptr address of node to be removed (not NULL)
 * @note After calling ::list_remove, node_ptr will be pointing
 * to the node after the node pointed by node_ptr
 * @return removed node (not NULL)
*/
ListNode* list_remove(ListNode** node_ptr);

/**
 * @brief Find node in list
 * @param head_ptr address of head node (not NULL)
 * @param node node to be found
 * @return address of 'next' field pointing to node (not NULL)
 * or NULL if could not find node
*/
ListNode** list_find(ListNode** head_ptr, ListNode* node);

#endif 
