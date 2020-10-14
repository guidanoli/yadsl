#include <map/map.h>

#include <assert.h>

#include <set/set.h>

#ifdef YADSL_DEBUG
#include <memdb/memdb.h>
#else
#include <stdlib.h>
#endif

typedef struct
{
	yadsl_MapEntryKey *key;
	yadsl_MapEntryValue *value;
}
yadsl_MapEntry;

typedef struct
{
	yadsl_SetHandle *entry_set;
	yadsl_MapEntryKeyCmpFunc cmp_keys_func;
	yadsl_MapEntryFreeFunc free_entry_func;
	yadsl_MapEntryFreeArg *cmp_keys_func_arg;
	yadsl_MapEntryFreeArg* free_entry_arg;
}
yadsl_Map;

/* Private functions prototypes */

typedef struct
{
	yadsl_MapEntryKey *key;
	yadsl_MapEntryKeyCmpFunc func;
	yadsl_MapEntryFreeArg *arg;
}
yadsl_MapCmpEntryKeyParameter;

typedef struct
{
	yadsl_MapEntryFreeFunc func;
	yadsl_MapEntryFreeArg* arg;
}
yadsl_MapEntryFreeParameter;

static yadsl_MapEntry*
yadsl_map_entry_create_internal(
	yadsl_MapEntryKey *key,
	yadsl_MapEntryValue *value);

static void
yadsl_map_entry_destroy_internal(
	yadsl_MapEntry *entry,
	yadsl_MapEntryFreeParameter *par);

static bool
yadsl_map_entry_key_compare_internal(
	yadsl_MapEntry *entry,
	yadsl_MapCmpEntryKeyParameter *par);

static yadsl_MapRet
yadsl_map_entry_get_internal(
	yadsl_MapHandle *map,
	yadsl_MapEntryKey *key,
	yadsl_MapEntry **entry_ptr);

/* Public functions */

yadsl_MapHandle*
yadsl_map_create(
	yadsl_MapEntryKeyCmpFunc cmp_keys_func,
	yadsl_MapEntryFreeFunc free_entry_func,
	yadsl_MapEntryFreeArg* cmp_keys_func_arg,
	yadsl_MapEntryFreeArg* free_entry_arg)
{
	yadsl_Map *map = malloc(sizeof(*map));
	if (!map)
		goto fail1;

	if (!(map->entry_set = yadsl_set_create()))
		goto fail2;

	map->cmp_keys_func = cmp_keys_func;
	map->free_entry_func = free_entry_func;
	map->cmp_keys_func_arg = cmp_keys_func_arg;
	map->free_entry_arg = free_entry_arg;

	return map;
fail2:
	free(map);
fail1:
	return NULL;
}

yadsl_MapRet
yadsl_map_entry_add(
	yadsl_MapHandle* map,
	yadsl_MapEntryKey* key,
	yadsl_MapEntryValue* value,
	bool* overwritten_ptr,
	yadsl_MapEntryValue** overwritten_value_ptr)
{
	yadsl_MapRet map_ret;
	yadsl_SetRet set_ret;
	yadsl_MapEntry *entry;

	if (map_ret = yadsl_map_entry_get_internal(map, key, &entry)) {
		*overwritten_ptr = false;
		if (map_ret != YADSL_MAP_RET_ENTRY_NOT_FOUND)
			return map_ret;
	} else {
		*overwritten_value_ptr = entry->value;
		*overwritten_ptr = true;
		entry->value = value;
		return YADSL_MAP_RET_OK;
	}

	if (!(entry = yadsl_map_entry_create_internal(key, value)))
		return YADSL_MAP_RET_MEMORY;

	if (set_ret = yadsl_set_item_add(((yadsl_Map*) map)->entry_set, entry)) {
		free(entry);
		assert(set_ret == YADSL_SET_RET_MEMORY);
		return YADSL_MAP_RET_MEMORY;
	}

	return YADSL_MAP_RET_OK;
}

yadsl_MapRet
yadsl_map_entry_get(
	yadsl_MapHandle* map,
	yadsl_MapEntryKey* key,
	yadsl_MapEntryValue** value_ptr)
{
	yadsl_MapRet map_ret;
	yadsl_MapEntry *entry;

	if (map_ret = yadsl_map_entry_get_internal(map, key, &entry))
		return map_ret;

	*value_ptr = entry->value;

	return YADSL_MAP_RET_OK;
}

yadsl_MapRet
yadsl_map_entry_remove(
	yadsl_MapHandle* map,
	yadsl_MapEntryKey* key,
	yadsl_MapEntryKey** original_key_ptr,
	yadsl_MapEntryValue** value_ptr)
{
	yadsl_MapRet map_ret;
	yadsl_MapEntry *entry;
	yadsl_MapEntryKey *temp_key;
	yadsl_MapEntryValue *temp_value;

	if (map_ret = yadsl_map_entry_get_internal(map, key, &entry))
		return map_ret;

	temp_key = entry->key;
	temp_value = entry->value;

	if (yadsl_set_item_remove(((yadsl_Map*) map)->entry_set, entry))
		assert(0);

	free(entry);

	*original_key_ptr = temp_key;
	*value_ptr = temp_value;

	return YADSL_MAP_RET_OK;
}

yadsl_MapRet
yadsl_map_entry_count_get(
	yadsl_MapHandle* map,
	size_t* count_ptr)
{
	size_t count;
	if (yadsl_set_size_get(((yadsl_Map*) map)->entry_set, &count))
		assert(0);
	*count_ptr = count;
	return YADSL_MAP_RET_OK;
}

void
yadsl_map_destroy(
	yadsl_MapHandle* map)
{
	yadsl_MapEntryFreeParameter cmp_keys_func_arg;
	yadsl_Map* map_ = (yadsl_Map *) map;

	if (map_ == NULL)
		return;

	cmp_keys_func_arg.func = map_->free_entry_func;
	cmp_keys_func_arg.arg = map_->free_entry_arg;
	yadsl_set_destroy(map_->entry_set, yadsl_map_entry_destroy_internal, &cmp_keys_func_arg);

	free(map_);
}

/* Private functions definitions */

void
yadsl_map_entry_destroy_internal(
	yadsl_MapEntry* entry,
	yadsl_MapEntryFreeParameter* par)
{
	if (par->func)
		par->func(entry->key, entry->value, par->arg);
	free(entry);
}

bool
yadsl_map_entry_key_compare_internal(
	yadsl_MapEntry* entry,
	yadsl_MapCmpEntryKeyParameter* par)
{
	if (par->func)
		return par->func(entry->key, par->key, par->arg);
	else
		return entry->key == par->key;
}

yadsl_MapRet
yadsl_map_entry_get_internal(
	yadsl_MapHandle* map,
	yadsl_MapEntryKey* key,
	yadsl_MapEntry** entry_ptr)
{
	yadsl_SetRet set_ret;
	yadsl_Map* map_ = (yadsl_Map*) map;
	yadsl_MapCmpEntryKeyParameter cmp_keys_func_arg;

	cmp_keys_func_arg.key = key;
	cmp_keys_func_arg.func = map_->cmp_keys_func;
	cmp_keys_func_arg.arg = map_->cmp_keys_func_arg;

	if (set_ret = yadsl_set_item_filter(map_->entry_set, yadsl_map_entry_key_compare_internal, &cmp_keys_func_arg, entry_ptr)) {
		assert(set_ret == YADSL_SET_RET_DOES_NOT_CONTAIN);
		return YADSL_MAP_RET_ENTRY_NOT_FOUND;
	}

	return YADSL_MAP_RET_OK;
}

yadsl_MapEntry* yadsl_map_entry_create_internal(
	yadsl_MapEntryKey* key,
	yadsl_MapEntryValue* value)
{
	yadsl_MapEntry *entry = malloc(sizeof(*entry));
	
	if (entry) {
		entry->key = key;
		entry->value = value;
	}

	return entry;
}
