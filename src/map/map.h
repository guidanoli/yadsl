#ifndef __YADSL_MAP_H__
#define __YADSL_MAP_H__

/**
 * \defgroup map Map
 * @brief Generic mapping structure
 *
 * A Map starts empty, with no entries.
 * It works as a function K -> V where
 * K is the key space and V is the value space.
 * It must be defined on construction the ways
 * to compare keys and to free entries from memory.
 * The Map takes ownership of the data structures
 * it is given indirectly when the proper freeEntry
 * function is delivered.
 *
 * The key comparison function should return a
 * boolean value indicating whether the two keys
 * are equal (!=0) or not (0). If no function is
 * given, shallow comparison will be assumed.
 *
 * @{
*/

#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Value returned by Map functions
*/
typedef enum
{
	YADSL_MAP_RET_OK = 0, /**< All went ok */
	YADSL_MAP_RET_ENTRY_NOT_FOUND, /**< Entry could not be found */
	YADSL_MAP_RET_MEMORY, /**< Could not allocate memory */
}
yadsl_MapRet;

typedef void yadsl_MapHandle; /**< Map handle */
typedef void yadsl_MapEntryKey; /**< Map entry key (userdata) */
typedef void yadsl_MapEntryValue; /**< Map entry value (userdata) */

typedef void yadsl_MapEntryKeyCmpArg; /**< Argument passed to yadsl_MapEntryKeyCmpFunc */
typedef void yadsl_MapEntryFreeArg; /**< Argument passed to yadsl_MapEntryFreeFunc */

/**
 * @brief Function responsible for comparing keys
 * @param key1 first object
 * @param key2 second object
 * @param arg user argument
 * @return an integer *n*, where...
 * * *n* > 0 if key1 > key2
 * * *n* = 0 if key1 = key2
 * * *n* < 0 if key1 < key2
*/
typedef int
(*yadsl_MapEntryKeyCmpFunc)(
	yadsl_MapEntryKey* key1,
	yadsl_MapEntryKey* key2,
	yadsl_MapEntryKeyCmpArg* arg);

/**
 * @brief Function responsible for freeing entries
 * @param key entry key (to be freed)
 * @param value entry value (to be freed)
 * @param free_entry_arg user argument
*/
typedef void
(*yadsl_MapEntryFreeFunc)(
	yadsl_MapEntryKey* key,
	yadsl_MapEntryValue* value,
	yadsl_MapEntryFreeArg* free_entry_arg);

/**
 * @brief Create an empty map
 * @param cmp_keys_func key comparison function
 * @param free_entry_func entry freeing function
 * @param cmp_keys_func_arg key comparison function argument
 * @param free_entry_arg entry freeing function argument
 * @return newly created map or NULL if could not allocate memory
*/
yadsl_MapHandle*
yadsl_map_create(
	yadsl_MapEntryKeyCmpFunc cmp_keys_func,
	yadsl_MapEntryFreeFunc free_entry_func,
	yadsl_MapEntryFreeArg* cmp_keys_func_arg,
	yadsl_MapEntryFreeArg* free_entry_arg);

/**
 * @brief Add or overwrite entry
 * @param map map
 * @param key entry key (always owned by the caller)
 * @param value entry value (owned by the map on success)
 * @param overwritten_ptr whether entry was overwritten or not
 * @param overwritten_value_ptr value previously assigned to key
 * @return
 * * ::YADSL_MAP_RET_OK, and *overwritten_ptr is updated.
 *   If the entry was overwritten, then *overwritten_value_ptr is updated.
 * * ::YADSL_MAP_RET_MEMORY
*/
yadsl_MapRet
yadsl_map_entry_add(
	yadsl_MapHandle* map,
	yadsl_MapEntryKey* key,
	yadsl_MapEntryValue* value,
	bool* overwritten_ptr,
	yadsl_MapEntryValue** overwritten_value_ptr);

/**
 * @brief Get map entry
 * @param map map
 * @param key entry key (always owned by the caller)
 * @param value_ptr entry value (always owned by the map)
 * @return
 * * ::YADSL_MAP_RET_OK, and *value_ptr is updated
 * * ::YADSL_MAP_RET_ENTRY_NOT_FOUND
*/
yadsl_MapRet
yadsl_map_entry_get(
	yadsl_MapHandle* map,
	yadsl_MapEntryKey* key,
	yadsl_MapEntryValue** value_ptr);

/**
 * @brief Remove map entry
 * @param map map
 * @param key entry key (always owned by the caller)
 * @param original_key_ptr original entry key (owned by the caller)
 * @param value_ptr entry value (owned by the caller)
 * @return
 * * ::YADSL_MAP_RET_OK, and *original_key_ptr and *value_ptr are updated
 * * ::YADSL_MAP_RET_ENTRY_NOT_FOUND
*/
yadsl_MapRet
yadsl_map_entry_remove(
	yadsl_MapHandle* map,
	yadsl_MapEntryKey* key,
	yadsl_MapEntryKey** original_key_ptr,
	yadsl_MapEntryValue** value_ptr);

/**
 * @brief Get number of entries in map
 * @param map map
 * @param count_ptr entry count
 * @return
 * * ::YADSL_MAP_RET_OK, and *count_ptr is updated
*/
yadsl_MapRet
yadsl_map_entry_count_get(
	yadsl_MapHandle* map,
	size_t* count_ptr);

/**
 * @brief Destroy map and its remaining entries
 * @param map map
*/
void
yadsl_map_destroy(
	yadsl_MapHandle* map);

/** @} */

#endif
