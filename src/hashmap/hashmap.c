#include "hashmap.h"

#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdlib.h>
#include <string.h>

#include "memdb.h"

struct Entry
{
	struct Entry* next;
	char* key; // created by strdup
	void* value;
};

struct EntryHead
{
	struct Entry* first;
};

struct HashMap
{
	struct EntryHead** entries;
	delete_entry_func delete_func;
	int size_exponent;
};

typedef struct Entry Entry;
typedef struct EntryHead EntryHead;

static unsigned long getSize(HashMap* pHashMap)
{
	return 1UL << (pHashMap->size_exponent);
}

HashMapRet hashMapCreate(HashMap** ppHashMap,
                         int size_exponent,
                         delete_entry_func delete_func)
{
	HashMap* pHashMap;
	
	pHashMap = malloc(sizeof(HashMap));
	if (!pHashMap)
		goto fail;
	
	pHashMap->size_exponent = size_exponent;
	pHashMap->delete_func = delete_func;

	unsigned long const size = getSize(pHashMap);
	pHashMap->entries = malloc(sizeof(EntryHead*) * size);
	if (!pHashMap->entries)
		goto fail2;

	for (unsigned long i = 0; i < size; ++i) {
		if (!(pHashMap->entries[i] = malloc(sizeof(Entry)))) {
			for (unsigned long j = 0; j < i; ++j)
				free(pHashMap->entries[i]);
			goto fail2;
		} else {
			pHashMap->entries[i]->first = NULL;
		}
	}

	*ppHashMap = pHashMap;

	return HASHMAP_OK;

fail2:
	free(pHashMap);
fail:
	return HASHMAP_MEMORY;
}

static Entry* newEntry(const char* key, void* value)
{
	Entry* pEntry = malloc(sizeof(Entry));
	if (!pEntry)
		goto fail;
	if (!(pEntry->key = strdup(key)))
		goto fail2;
	pEntry->value = value;
	pEntry->next = NULL;
	return pEntry;
fail2:
	free(pEntry);
fail:
	return NULL;
}

static void deleteEntry(Entry* entry, delete_entry_func delfunc)
{
	delfunc(entry->key, entry->value);
	free(entry->key);
	free(entry);
}

static int existsEntryInList(EntryHead* head, const char* key, Entry** pprev)
{
	Entry* prev = NULL;
	for (Entry* p = head->first; p; p = p->next) {
		if (!strcmp(p->key, key)) {
			if (pprev)
				*pprev = prev;
			return 1;
		}
		prev = p;
	}
	return 0;
}

static HashMapRet addEntryInList(EntryHead* head, const char* key, void* value)
{
	if (existsEntryInList(head, key, NULL))
		return HASHMAP_EXISTS;
	Entry* new_entry = newEntry(key, value);
	if (!new_entry)
		return HASHMAP_MEMORY;
	new_entry->next = head->first;
	head->first = new_entry;
	return HASHMAP_OK;
}

static HashMapRet removeEntryInList(EntryHead* head, const char* key, delete_entry_func delfunc)
{
	Entry* prev;
	if (!existsEntryInList(head, key, &prev))
		return HASHMAP_DOESNT_EXIST;
	if (prev == NULL) {
		Entry* temp = head->first->next;
		deleteEntry(head->first, delfunc);
		head->first = temp;
	} else {
		Entry* temp = prev->next->next;
		deleteEntry(prev->next, delfunc);
		prev->next = temp;
	}
	return HASHMAP_OK;
}

static HashMapRet getEntryValueInList(EntryHead* head, const char* key, void** pValue)
{
	Entry* prev;
	if (!existsEntryInList(head, key, &prev))
		return HASHMAP_DOESNT_EXIST;
	Entry* entry = prev ? prev->next : head->first;
	*pValue = entry->value;
	return HASHMAP_OK;
}

static unsigned long hash(unsigned const char* str)
{
	unsigned long hash = 5381;
	int c;

	while (c = *str++)
		hash = ((hash << 5) + hash) + c;

	return hash;
}

static unsigned long getIndex(HashMap* pHashMap, const char* key)
{
	unsigned long hashed_key = hash(key);
	return hashed_key & ((1 << pHashMap->size_exponent) - 1);
}

static EntryHead* getEntryHead(HashMap* pHashMap, const char* key)
{
	unsigned long index = getIndex(pHashMap, key);
	return pHashMap->entries[index];
}

HashMapRet hashMapAddEntry(HashMap* pHashMap, const char* key, void* value)
{
	EntryHead* head = getEntryHead(pHashMap, key);
	return addEntryInList(head, key, value);
}

HashMapRet hashMapGetEntry(HashMap* pHashMap, const char* key, void** pValue)
{
	EntryHead* head = getEntryHead(pHashMap, key);
	return getEntryValueInList(head, key, pValue);
}

HashMapRet hashMapRemoveEntry(HashMap* pHashMap, const char* key)
{
	EntryHead* head = getEntryHead(pHashMap, key);
	return removeEntryInList(head, key, pHashMap->delete_func);
}

static void printEntryHead(EntryHead* head)
{
	for (Entry* p = head->first; p; p = p->next)
		printf("%s: %p, ", p->key, p->value);
}

void hashMapPrint(HashMap* pHashMap)
{
	printf("{ ");
	const unsigned long size = getSize(pHashMap);
	for (unsigned long i = 0; i < size; ++i)
		printEntryHead(pHashMap->entries[i]);
	printf("}\n");
}

static void destroyEntryHead(EntryHead* head, delete_entry_func delfunc)
{
	for (Entry* p = head->first; p;) {
		Entry* next = p->next;
		deleteEntry(p, delfunc);
		p = next;
	}
	free(head);
}

void hashMapDestroy(HashMap* pHashMap)
{
	const unsigned long size = getSize(pHashMap);
	for (unsigned long i = 0; i < size; ++i)
		destroyEntryHead(pHashMap->entries[i], pHashMap->delete_func);
	free(pHashMap->entries);
	free(pHashMap);
}
