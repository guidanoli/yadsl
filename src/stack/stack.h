#ifndef __STACK_H__
#define __STACK_H__

//     _____ __             __  
//    / ___// /_____ ______/ /__
//    \__ \/ __/ __ `/ ___/ //_/
//   ___/ / /_/ /_/ / /__/ ,<   
//  /____/\__/\__,_/\___/_/|_|  
//                              
//  A Stack starts empty. You can add and
// remove objects (if not empty), and check
// whether the stack is empty or not.
//
// HINTS
// -----
//
// Objects can assume the value NULL (0).
//

typedef enum
{
	STACK_OK = 0,
	STACK_EMPTY,
	STACK_MEMORY,
}
StackRet;

typedef struct Stack Stack;

//  ============= ============================== 
//   stackCreate           Create stack          
//  ============= ============================== 
//   ppStack       (owned ret) pointer to stack  
//  ============= ============================== 
//  [!] STACK_MEMORY

StackRet stackCreate(Stack **ppStack);

//  ========== ===================== 
//   stackAdd   Add object to stack  
//  ========== ===================== 
//   pStack     pointer to stack     
//   object     object to be added   
//  ========== ===================== 
//  [!] STACK_MEMORY

StackRet stackAdd(Stack *pStack, void *object);

//  ============ ===================================== 
//   stackEmpty         Check if stack is empty        
//  ============ ===================================== 
//   pStack       pointer to stack                     
//   pIsEmpty     (ret) whether stack is empty or not  
//  ============ ===================================== 

StackRet stackEmpty(Stack *pStack, int *pIsEmpty);

//  ============= ============================ 
//   stackRemove    Remove object from stack   
//  ============= ============================ 
//   pStack        pointer to stack            
//   pObject       (owned ret) removed object  
//  ============= ============================ 
//  [!] STACK_EMPTY

StackRet stackRemove(Stack *pStack, void **pObject);

//  ============== ============================================ 
//   stackDestroy   Deallocate stack and its remaining objects  
//  ============== ============================================ 
//   pStack         pointer to stack                            
//   freeObject     object deallocation function                
//  ============== ============================================ 

void stackDestroy(Stack *pStack, void freeObject(void *));

#endif
