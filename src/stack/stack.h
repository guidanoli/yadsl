#ifndef __STACK_H__
#define __STACK_H__

//     _____ __             __  
//    / ___// /_____ ______/ /__
//    \__ \/ __/ __ `/ ___/ //_/
//   ___/ / /_/ /_/ / /__/ ,<   
//  /____/\__/\__,_/\___/_/|_|  
//                              
// A stack starts empty. You can add and
// remove objects (if not empty).
//

typedef enum
{
	STACK_OK = 0,
	STACK_EMPTY,
	STACK_MEMORY,
}
stack_return;

typedef struct stack stack;

// Create stack at *stack_ptr
// stack_ptr (ret): new stack
// -> STACK_MEMORY
stack_return stack_create(stack **stack_ptr);

// Add object to stack
// stack
// object (opt): object to be added
// -> STACK_MEMORY
stack_return stack_add(stack *stack, void *object);

// Check if stack is empty
// stack
// is_empty_ptr (ret): stack is empty or not
stack_return stack_empty(stack *stack, int *is_empty_ptr);

// Remove object from stack
// stack
// object_ptr (ret): removed object
// -> STACK_EMPTY
stack_return stack_remove(stack *stack, void **object_ptr);

// Destroy stack
// stack
// free_object (opt): function that frees remaining objects
stack_return stack_destroy(stack *stack, void free_object(void *));

#endif
