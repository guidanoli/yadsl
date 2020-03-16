#ifndef __MAP_H__
#define __MAP_H__

//
//      __  ___          
//     /  |/  /___ _____ 
//    / /|_/ / __ `/ __ \
//   / /  / / /_/ / /_/ /
//  /_/  /_/\__,_/ .___/ 
//              /_/      
//
// A Map starts empty, with no entries.
// It works as a function K -> V where
// K is the key space and V is the value space.
// It must be defined on construction the ways
// to compare keys and to free entries from memory.
// The Map takes ownership of the data structures
// it is given indirectly when the proper freeEntry
// function is delivered.
//
// HINTS
// -----
//
//  The key comparison function should return a
// boolean value indicating whether the two keys
// are equal (!=0) or not (0). If no function is
// given, shallow comparison will be assumed.
//

typedef enum
{
	MAP_OK = 0,
	MAP_ENTRY_NOT_FOUND,
	MAP_MEMORY,
}
MapRet;

typedef struct Map Map;

#include <stddef.h>

//  =========== =========================================== 
//   mapCreate              create an empty map             
//  =========== =========================================== 
//   ppMap       (ret) pointer to map                       
//   cmpKeys     (opt) key comparison function              
//   freeEntry   (opt) entry deallocation function          
//   arg         additional argument provided to freeEntry  
//  =========== =========================================== 
//  [!] MAP_MEMORY

MapRet mapCreate(Map **ppMap,
	int (*compareKeys)(void *a, void *b),
	void (*freeEntry)(void *key, void *value, void *arg),
	void *arg);

//  ================ =================================================== 
//    mapPutEntry     Put an entry or overwrite an already existing one  
//  ================ =================================================== 
//   pMap             pointer to map                                     
//   key              entry key (always owned by the caller)                        
//   value            entry value (owned by the map on success)              
//   pOverwriten      (ret) whether entry was overwritten or not         
//   pPreviousValue   (ret) old value, if entry previously existed
//                    (owned by the caller if entry is overwritten)
//  ================ =================================================== 
//  [!] MAP_MEMORY

MapRet mapPutEntry(Map *pMap, void *key, void *value,
	int *pOverwritten,
	void **pPreviousValue);

//  ============= ============================================= 
//   mapGetEntry     Get map entry                              
//  ============= ============================================= 
//   pMap          pointer to map                               
//   key           entry key (always owned by the caller)       
//   pValue        (ret) entry value (always owned by the map)  
//  ============= ============================================= 
//  [!] MAP_ENTRY_NOT_FOUND

MapRet mapGetEntry(Map *pMap, void *key, void **pValue);

//  ================ ======================================================= 
//   mapRemoveEntry    Remove map entry and, if found, retrieve its value    
//  ================ ======================================================= 
//   pMap             pointer to map                                         
//   key              entry key (for searching, always owned by the caller)  
//   pKey             (ret) original entry key (always owned by the caller)  
//   pValue           (ret) entry value (always owned by the caller)         
//  ================ ======================================================= 
//  [!] MAP_ENTRY_NOT_FOUND

MapRet mapRemoveEntry(Map *pMap, void *key, void **pKey, void **pValue);

//  ======================= ============================== 
//   mapGetNumberOfEntries   Get number of entries in map  
//  ======================= ============================== 
//   pMap                    pointer to map                
//   pNum                    (ret) number of entries       
//  ======================= ============================== 

MapRet mapGetNumberOfEntries(Map *pMap, size_t *pNum);

//  ============ ======================================= 
//   mapDestroy   Destroy map and its remaining entries  
//  ============ ======================================= 
//   pMap         pointer to map                         
//  ============ ======================================= 

void mapDestroy(Map *pMap);

#endif