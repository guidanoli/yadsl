#ifndef __YADSL_HASHMAP_H__
#define __YADSL_HASHMAP_H__

/**
* \defgroup hashmap Hash Map
* @brief A hash map that takes strings as keys and generic data as values
*
* @{
*/

/**
 * @brief Return value of Hash Map functions
*/
typedef enum
{
	YADSL_HASHMAP_RET_OK = 0, /**< All went ok */
	YADSL_HASHMAP_RET_MEMORY, /**< Could not allocate memory */
	YADSL_HASHMAP_RET_EXISTS, /**< Entry already exists */
	YADSL_HASHMAP_RET_DOESNT_EXIST, /**< Entry doesn't exist */
}
yadsl_HashMapRet;

typedef void yadsl_HashMapHandle; /**< Hash Map handle */
typedef unsigned int yadsl_HashMapSizeExponent; /**< Hash Map size exponent */
typedef const char* yadsl_HashMapKey; /**< Hash Map key */
typedef void yadsl_HashMapValue; /**< Hash Map value */

/**
 * @brief Function responsible for freeing each entry
 * @param key key associated to entry (you don't own it)
 * @param value value associated to entry (you own it)
*/
typedef void
(*yadsl_HashMapFreeEntryFunc)(
	yadsl_HashMapKey key,
	yadsl_HashMapValue* value);

/**
 * @brief Creates a hash map
 * @param size_exponent hash map size = 2 ^ (size_exponent)
 * @param free_entry_func entry freeing function
 * @return newly created hash map or NULL if could not allocate memory
*/
yadsl_HashMapHandle*
yadsl_hashmap_create(
	yadsl_HashMapSizeExponent size_exponent,
	yadsl_HashMapFreeEntryFunc free_entry_func);

/**
 * @brief Adds entry to hash map
 * @param hashmap hash map
 * @param key entry key (will be copied)
 * @param value entry value (will be owned)
 * @return
 * * ::YADSL_HASHMAP_RET_OK, and entry is added
 * * ::YADSL_HASHMAP_RET_EXISTS
 * * ::YADSL_HASHMAP_RET_MEMORY
*/
yadsl_HashMapRet
yadsl_hashmap_entry_add(
	yadsl_HashMapHandle* hashmap,
	yadsl_HashMapKey key,
	yadsl_HashMapValue* value);

/**
 * @brief Get value from hash map
 * @param hashmap hash map
 * @param key entry key (not owned)
 * @param value_ptr entry value (still owned)
 * @return
 * * ::YADSL_HASHMAP_RET_OK, and *value_ptr is updated
 * * ::YADSL_HASHMAP_RET_DOESNT_EXIST
*/
yadsl_HashMapRet
yadsl_hashmap_entry_value_get(
	yadsl_HashMapHandle* hashmap,
	yadsl_HashMapKey key,
	yadsl_HashMapValue** value_ptr);

/**
 * @brief Remove entry from map
 * @param hashmap hash map
 * @param key entry key
 * @return
 * * ::YADSL_HASHMAP_RET_OK, and entry is removed
 * * ::YADSL_HASHMAP_RET_DOESNT_EXIST
*/
yadsl_HashMapRet
yadsl_hashmap_entry_remove(
	yadsl_HashMapHandle* hashmap,
	yadsl_HashMapKey key);

/**
 * @brief Print hash map to stdout
 * @param hashmap hash map
*/
void
yadsl_hashmap_print(
	yadsl_HashMapHandle* hashmap);

/**
 * @brief Destroy hash map
 * @param hashmap hash map
*/
void
yadsl_hashmap_destroy(
	yadsl_HashMapHandle* hashmap);

/** @} */

#endif
