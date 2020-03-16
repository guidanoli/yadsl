#include "map.h"

#include <stdlib.h>
#include <assert.h>

#include "set.h"
#include "memdb.h"

struct Entry
{
	void *key;
	void *value;
};

struct Map
{
	Set *entrySet;
	int (*compareKeys)(void *a, void *b);
	void (*freeEntry)(void *key, void *value, void *arg);
	void *arg;
};

/* Private functions prototypes */

struct _cmpEntryKeyParameter
{
	void *key;
	int (*cmp)(void *a, void *b);
};

struct _freeEntryParameter
{
	void (*freeEntry)(void *key, void *value, void *arg);
	void *arg;
};

static MapRet _allocateEntry(void *key, void *value,
	struct Entry **ppEntry);
static void _freeEntry(struct Entry *pEntry, struct _freeEntryParameter *par);
static int _cmpEntryKey(struct Entry *pEntry,
	struct _cmpEntryKeyParameter *par);
static MapRet _getEntry(Map *pMap, void *key, struct Entry **ppEntry);

/* Public functions */

MapRet mapCreate(Map **ppMap,
	int (*compareKeys)(void *a, void *b),
	void (*freeEntry)(void *key, void *value, void *arg),
	void *arg)
{
	SetRet setId;
	Map *pMap = NULL;
	pMap = malloc(sizeof(struct Map));
	if (pMap == NULL)
		return MAP_MEMORY;
	if (setId = setCreate(&pMap->entrySet)) {
		free(pMap);
		assert(setId == SET_MEMORY);
		return MAP_MEMORY;
	}
	pMap->compareKeys = compareKeys;
	pMap->freeEntry = freeEntry;
	pMap->arg = arg;
	*ppMap = pMap;
	return MAP_OK;
}

MapRet mapPutEntry(Map *pMap, void *key, void *value,
	int *pOverwritten,
	void **pPreviousValue)
{
	MapRet mapId;
	SetRet setId;
	struct Entry *pEntry;
	if (mapId = _getEntry(pMap, key, &pEntry)) {
		*pOverwritten = 0;
		if (mapId != MAP_ENTRY_NOT_FOUND)
			return mapId;
	} else {
		*pPreviousValue = pEntry->value;
		*pOverwritten = 1;
		pEntry->value = value;
		return MAP_OK;
	}
	if (mapId = _allocateEntry(key, value, &pEntry))
		return mapId;
	if (setId = setAddItem(pMap->entrySet, pEntry)) {
		free(pEntry);
		assert(setId == SET_MEMORY);
		return MAP_MEMORY;
	}
	return MAP_OK;
}

MapRet mapGetEntry(Map *pMap, void *key, void **pValue)
{
	MapRet mapId;
	struct Entry *pEntry;
	if (mapId = _getEntry(pMap, key, &pEntry))
		return mapId;
	*pValue = pEntry->value;
	return MAP_OK;
}

MapRet mapRemoveEntry(Map *pMap, void *key, void **pKey, void **pValue)
{
	MapRet mapId;
	struct Entry *pEntry;
	void *tempKey, *tempValue;
	if (mapId = _getEntry(pMap, key, &pEntry))
		return mapId;
	tempKey = pEntry->key;
	tempValue = pEntry->value;
	if (setRemoveItem(pMap->entrySet, pEntry))
		assert(0);
	free(pEntry);
	*pKey = tempKey;
	*pValue = tempValue;
	return MAP_OK;
}

MapRet mapGetNumberOfEntries(Map *pMap, size_t *pNum)
{
	size_t temp;
	if (setGetSize(pMap->entrySet, &temp))
		assert(0);
	*pNum = temp;
	return MAP_OK;
}

void mapDestroy(Map *pMap)
{
	struct _freeEntryParameter arg;
	if (pMap == NULL)
		return;
	arg.freeEntry = pMap->freeEntry;
	arg.arg = pMap->arg;
	setDestroyDeep(pMap->entrySet, _freeEntry, &arg);
	free(pMap);
}

/* Private functions definitions */

static void _freeEntry(struct Entry *pEntry, struct _freeEntryParameter *par)
{
	if (par->freeEntry)
		par->freeEntry(pEntry->key, pEntry->value, par->arg);
	free(pEntry);
}

static int _cmpEntryKey(struct Entry *pEntry,
	struct _cmpEntryKeyParameter *par)
{
	if (par->cmp)
		return par->cmp(pEntry->key, par->key);
	else
		return pEntry->key == par->key;
}

static MapRet _getEntry(Map *pMap, void *key, struct Entry **ppEntry)
{
	SetRet setId;
	struct _cmpEntryKeyParameter arg = { key, pMap->compareKeys };
	if (setId = setFilterItem(pMap->entrySet, _cmpEntryKey, &arg, ppEntry)) {
		assert(setId == SET_DOES_NOT_CONTAIN);
		return MAP_ENTRY_NOT_FOUND;
	}
	return MAP_OK;
}

static MapRet _allocateEntry(void *key, void *value,
	struct Entry **ppEntry)
{
	struct Entry *pEntry;
	pEntry = malloc(sizeof(struct Entry));
	if (pEntry == NULL)
		return MAP_MEMORY;
	pEntry->key = key;
	pEntry->value = value;
	*ppEntry = pEntry;
	return MAP_OK;
}
