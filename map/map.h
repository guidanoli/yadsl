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

typedef enum
{
	/* All went ok */
	MAP_RETURN_OK = 0,

	/* New entry has been registered on top of another
	* [!] This means that the ownership of the given
	* key pointer is the caller's */
	MAP_RETURN_OVERWROTE_ENTRY,

	/* Entry could not be found */
	MAP_RETURN_ENTRY_NOT_FOUND,

	/* Invalid parameter was provided */
	MAP_RETURN_INVALID_PARAMETER,

	/* Could not allocate memory space */
	MAP_RETURN_MEMORY,

}
MapReturnID;

typedef struct Map Map;

/**
* Create an empty map
* ppMap         address of pointer to map
* compareKeys   function responsible of comparing keys
*               returns if keys are equal (boolean)
*               if NULL, shallow comparisons will be done
* freeEntry     function responsible of freeing entries, taking
*               an extra argument "arg"
*               if NULL, the map does not take the ownership
*               of either keys or values provided
* arg           extra argument to be parsed to freeEntry
* Possible errors:
* MAP_RETURN_INVALID_PARAMETER
* 	- "ppMap" is NULL
* MAP_RETURN_MEMORY
*/
MapReturnID mapCreate(Map **ppMap,
	int (*compareKeys)(void *a, void *b),
	void (*freeEntry)(void *key, void *value, void *arg),
	void *arg);

/**
* Put a new entry or overwrite an already existing one
* pMap              pointer to map
* key               entry key
* value             entry value
* pPreviousValue    (return) previous value mapped to
*                   the key, if such entry already existed.
* Possible errors:
* MAP_RETURN_INVALID_PARAMETER
* 	- "pMap" is NULL
* 	- "pPreviousValue" is NULL
* MAP_RETURN_OVERWROTE_ENTRY
* 	- pointer of address "pPreviousValue" now points to previous value
* MAP_RETURN_MEMORY
* [!] In case of error (return isn't OK or OVERWROTE_ENTRY), the
*   ownership of both entry key and value are kept to be the caller's
* [!] In case of overwritting entry, the ownership of the previous
*   value, returned by reference, is the caller's
*/
MapReturnID mapPutEntry(Map *pMap, void *key, void *value,
	void **pPreviousValue);

/**
* Get entry, if existing
* pMap      pointer to map
* key       entry key for searching
* pValue    (return) entry value
* Possible errors:
* MAP_RETURN_INVALID_PARAMETER
* 	- "pMap" is NULL
* 	- "pValue" is NULL
* MAP_RETURN_ENTRY_NOT_FOUND
* [!] At all circunstances, the map holds the ownership of
*   the entry value, and the ownership of the entry key is
*   the caller's.
*/
MapReturnID mapGetEntry(Map *pMap, void *key, void **pValue);

/**
* Remove an entry, if existing, and retrieve its value
* pMap      pointer to map
* key       entry key for searching
* pKey      (return) entry key
* pValue    (return) entry value
* Possible errors:
* MAP_RETURN_INVALID_PARAMETER
* 	- "pMap" is NULL
* 	- "pKey" is NULL
* 	- "pValue" is NULL
* MAP_RETURN_ENTRY_NOT_FOUND
* [!] At all circunstances, the ownership of the entry keys
*   (the one provided and the one returned) and of the entry
*   value (if returned) are the caller's.
*/
MapReturnID mapRemoveEntry(Map *pMap, void *key, void **pKey, void **pValue);

/**
* Get number of entries
* pMap  pointer to map
* pNum  (return) number of entries
* Possible errors:
* MAP_RETURN_INVALID_PARAMETER
* 	- "pMap" is NULL
* 	- "pNum" is NULL
*/
MapReturnID mapGetNumberOfEntries(Map *pMap, size_t *pNum);

/**
* Destroys map and dependencies
* pMap  pointer to map
* [!] If freeEntry wasn't specified in mapCreate,
* then, only the map data structures will be freed
*/
void mapDestroy(Map *pMap);

#endif