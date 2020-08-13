#ifndef __HASHMAP_H__
#define __HASHMAP_H__

typedef enum
{
	HASHMAP_OK = 0,
	HASHMAP_MEMORY,
	HASHMAP_EXISTS,
	HASHMAP_DOESNT_EXIST,
}
HashMapRet;

typedef struct HashMap HashMap;
typedef void (*delete_entry_func)(const char* key, void* value);

// Creates
HashMapRet hashMapCreate(HashMap** ppHashMap,
                         int size_exponent,
                         delete_entry_func delete_func);

// Adds
HashMapRet hashMapAddEntry(HashMap* pHashMap, const char* key, void* value);

// Get
HashMapRet hashMapGetEntry(HashMap* pHashMap, const char* key, void** pValue);

// Removes
HashMapRet hashMapRemoveEntry(HashMap* pHashMap, const char* key);

// Print (debug)
void hashMapPrint(HashMap* pHashMap);

// Destroys
void hashMapDestroy(HashMap* pHashMap);

#endif
