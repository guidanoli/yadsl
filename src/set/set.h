#ifndef __SET_H__
#define __SET_H__

//
//     _____      __ 
//    / ___/___  / /_
//    \__ \/ _ \/ __/
//   ___/ /  __/ /_  
//  /____/\___/\__/  
//  
//
//  A Set starts empty.
// You are able to add and remove items (opaque pointers),
// check if items are contained within a set or not,
// and iterate through them.
//
//  It does not acquire the ownership of the items it
// stores and, therefore, does not deallocates them
// when destroyed.
//
// HINTS
// -----
//
//  Items can assume NULL (0) value.
//
//  The filtering function takes an item and the additional
// argument as parameters and should return a boolean value
// indicating if the item is the one to be filtered. If 'True'
// is returned, the iteration stops and the item is returned
// by reference (yet, the ownership is still the set's).
//
//  If the set state is changed while filtering, the function
// will be recursively. No guarantee is given that the call
// will terminate.
//

typedef enum
{
	SET_OK = 0,
	SET_MEMORY,
	SET_CONTAINS,
	SET_DOES_NOT_CONTAIN,
	SET_EMPTY,
	SET_OUT_OF_BOUNDS,
}
SetRet;

typedef struct Set Set;

#include <stddef.h>

//  =========== ============================ 
//   setCreate   Create an empty set         
//  =========== ============================ 
//   ppSet       (owned ret) pointer to set  
//  =========== ============================ 
//  [!] SET_MEMORY

SetRet setCreate(Set **ppSet);

//  ================= ======================================== 
//   setContainsItem   Check whether set contains item or not  
//  ================= ======================================== 
//   pSet              pointer to set                          
//   item              item to be checked                      
//  ================= ======================================== 
//  [!] SET_CONTAINS
//  [!] SET_DOES_NOT_CONTAIN

SetRet setContainsItem(Set *pSet, void *item);

//  =============== ==================================== 
//   setFilterItem      Filter item from set             
//  =============== ==================================== 
//   pSet            pointer to set                     
//   func            filtering function                
//   arg             (opt) additional argument to func   
//   pItem           (ret) filtered item                 
//  =============== ==================================== 
//  [!] SET_DOES_NOT_CONTAIN: no item filtered

SetRet setFilterItem(Set *pSet, int (*func) (void *item, void *arg),
	void *arg, void **pItem);

//  ============ ========================== 
//   setAddItem   Add item to set           
//  ============ ========================== 
//   pSet         pointer to set           
//   item         (owned) item to be added  
//  ============ ========================== 
//  [!] SET_CONTAINS
//  [!] SET_MEMORY

SetRet setAddItem(Set *pSet, void *item);

//  =============== ====================== 
//   setRemoveItem   Remove item from set  
//  =============== ====================== 
//   pSet            pointer to set        
//   item            item to be removed    
//  =============== ====================== 
//  [!] SET_DOES_NOT_CONTAIN

SetRet setRemoveItem(Set *pSet, void *item);

//  =================== ========================================== 
//   setGetCurrentItem   Get item currently pointed by the cursor  
//  =================== ========================================== 
//   pSet                pointer to set                            
//   pItem               (ret) item pointed by the cursor          
//  =================== ========================================== 
//  [!] SET_EMPTY

SetRet setGetCurrentItem(Set *pSet, void **pItem);

//  ============ ======================= 
//   setGetSize    Get set cardinality   
//  ============ ======================= 
//   pSet         pointer to set         
//   pSize        (ret) set cardinality  
//  ============ ======================= 

SetRet setGetSize(Set *pSet, size_t *pSize);

//  ================= ======================================== 
//   setPreviousItem   Make cursor point to the previous item  
//  ================= ======================================== 
//   pSet              pointer to set                          
//  ================= ======================================== 
//  [!] SET_EMPTY
//  [!] SET_OUT_OF_BOUNDS: current item is the first in the set

SetRet setPreviousItem(Set *pSet);

//  ============= ==================================== 
//   setNextItem   Make cursor point to the next item  
//  ============= ==================================== 
//   pSet          pointer to set                      
//  ============= ==================================== 
//  [!] SET_EMPTY
//  [!] SET_OUT_OF_BOUNDS: current item is the last in the set

SetRet setNextItem(Set *pSet);

//  ============== ===================================== 
//   setFirstItem   Make cursor point to the first item  
//  ============== ===================================== 
//   pSet           pointer to set                       
//  ============== ===================================== 
//  [!] SET_EMPTY

SetRet setFirstItem(Set *pSet);

//  ============= ==================================== 
//   setLastItem   Make cursor point to the last item  
//  ============= ==================================== 
//   pSet          pointer to set                      
//  ============= ==================================== 
//  [!] SET_EMPTY

SetRet setLastItem(Set *pSet);

//  ============ ======================================== 
//   setDestroy   Deallocate set and its remaining items  
//  ============ ======================================== 
//   pSet         pointer to set                          
//  ============ ======================================== 

void setDestroy(Set *pSet);

//  ============ ======================================== 
//   setDestroy   Deallocate set and its remaining items  
//  ============ ======================================== 
//   pSet         pointer to set                          
//   freeItem     item deallocation function              
//   arg          additional argument passed to freeItem  
//  ============ ======================================== 

void setDestroyDeep(Set *pSet, void (*freeItem)(void *item, void *arg), void *arg);

#endif
