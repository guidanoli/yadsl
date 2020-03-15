#ifndef __STACK_H__
#define __STACK_H__

//     _____ __             __  
//    / ___// /_____ ______/ /__
//    \__ \/ __/ __ `/ ___/ //_/
//   ___/ / /_/ /_/ / /__/ ,<   
//  /____/\__/\__,_/\___/_/|_|  
//                              
// A stack starts empty. You can add and
// remove objects (if not empty), and check
// whether the stack is empty or not.
//

typedef enum
{
	STACK_OK = 0,
	STACK_EMPTY,
	STACK_MEMORY,
}
StackReturnID;

typedef struct Stack Stack;

// Create stack at *ppStack
// ppStack (ret): new stack
// -> STACK_MEMORY
StackReturnID stackCreate(Stack **ppStack);

// Add object to stack
// stack
// object (opt): object to be added
// -> STACK_MEMORY
StackReturnID stackAdd(Stack *pStack, void *object);

// Check if stack is empty
// stack
// pIsEmpty (ret): stack is empty or not
StackReturnID stackEmpty(Stack *pStack, int *pIsEmpty);

// Remove object from stack
// stack
// pObject (ret): removed object
// -> STACK_EMPTY
StackReturnID stackRemove(Stack *pStack, void **pObject);

// Destroy stack
// stack
// freeObject (opt): function that frees remaining objects
void stackDestroy(Stack *pStack, void freeObject(void *));

#endif
