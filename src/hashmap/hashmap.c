#include <hashmap/hashmap.h>

#include <yadsl/posixstring.h>
#include <stdlib.h>

#include <memdb/memdb.h>

struct yadsl_HashMapEntry_s
{
	struct yadsl_HashMapEntry_s* next;
	char* key;
	yadsl_HashMapValue* value;
};

struct yadsl_HashMapEntryHead_s
{
	struct yadsl_HashMapEntry_s* first;
};

typedef struct
{
	struct yadsl_HashMapEntryHead_s** entries;
	yadsl_HashMapFreeEntryFunc free_entry_func;
	yadsl_HashMapSizeExponent size_exponent;
}
yadsl_HashMap;

typedef struct yadsl_HashMapEntry_s yadsl_HashMapEntry;
typedef struct yadsl_HashMapEntryHead_s yadsl_HashMapEntryHead;

static unsigned long
yadsl_hashmap_get_size_internal(
	yadsl_HashMap* hashmap)
{
	return 1UL << (hashmap->size_exponent);
}

yadsl_HashMapHandle*
yadsl_hashmap_create(
	yadsl_HashMapSizeExponent size_exponent,
	yadsl_HashMapFreeEntryFunc free_entry_func)
{
	yadsl_HashMap* hashmap = malloc(sizeof(*hashmap));
	if (!hashmap)
		goto fail;

	hashmap->size_exponent = size_exponent;
	hashmap->free_entry_func = free_entry_func;

	unsigned long const size = yadsl_hashmap_get_size_internal(hashmap);
	hashmap->entries = malloc(sizeof(*hashmap->entries) * size);
	if (!hashmap->entries)
		goto fail2;

	for (unsigned long i = 0; i < size; ++i) {
		if (!(hashmap->entries[i] = malloc(sizeof(*hashmap->entries[i])))) {
			for (unsigned long j = 0; j < i; ++j)
				free(hashmap->entries[j]);
			goto fail2;
		} else {
			hashmap->entries[i]->first = NULL;
		}
	}

	return hashmap;
fail2:
	free(hashmap);
fail:
	return NULL;
}

static yadsl_HashMapEntry*
yadsl_hashmap_entry_create_internal(
	yadsl_HashMapKey key,
	yadsl_HashMapValue* value)
{
	yadsl_HashMapEntry* entry = malloc(sizeof(*entry));
	if (!entry)
		goto fail;

	if (!(entry->key = strdup(key)))
		goto fail2;

	entry->value = value;
	entry->next = NULL;

	return entry;
fail2:
	free(entry);
fail:
	return NULL;
}

static void
yadsl_hashmap_entry_destroy_internal(
	yadsl_HashMapEntry* entry,
	yadsl_HashMapFreeEntryFunc free_entry_func)
{
	free_entry_func(entry->key, entry->value);
	free(entry->key);
	free(entry);
}

static int
yadsl_hashmap_entry_exists_in_list_internal(
	yadsl_HashMapEntryHead* head,
	yadsl_HashMapKey key,
	yadsl_HashMapEntry** pprev)
{
	yadsl_HashMapEntry* prev = NULL;
	for (yadsl_HashMapEntry* p = head->first; p; p = p->next) {
		if (!strcmp(p->key, key)) {
			if (pprev)
				*pprev = prev;
			return 1;
		}
		prev = p;
	}
	return 0;
}

static yadsl_HashMapRet
yadsl_hashmap_entry_add_in_list_internal(
	yadsl_HashMapEntryHead* head,
	yadsl_HashMapKey key,
	yadsl_HashMapValue* value)
{
	if (yadsl_hashmap_entry_exists_in_list_internal(head, key, NULL))
		return YADSL_HASHMAP_RET_EXISTS;

	yadsl_HashMapEntry* new_entry = yadsl_hashmap_entry_create_internal(key, value);
	if (!new_entry)
		return YADSL_HASHMAP_RET_MEMORY;

	new_entry->next = head->first;
	head->first = new_entry;

	return YADSL_HASHMAP_RET_OK;
}

static yadsl_HashMapRet
yadsl_hashmap_entry_remove_from_list_internal(
	yadsl_HashMapEntryHead* head,
	yadsl_HashMapKey key,
	yadsl_HashMapFreeEntryFunc free_entry_func)
{
	yadsl_HashMapEntry* prev;
	if (!yadsl_hashmap_entry_exists_in_list_internal(head, key, &prev))
		return YADSL_HASHMAP_RET_DOESNT_EXIST;

	if (prev == NULL) {
		yadsl_HashMapEntry* temp = head->first->next;
		yadsl_hashmap_entry_destroy_internal(head->first, free_entry_func);
		head->first = temp;
	} else {
		yadsl_HashMapEntry* temp = prev->next->next;
		yadsl_hashmap_entry_destroy_internal(prev->next, free_entry_func);
		prev->next = temp;
	}

	return YADSL_HASHMAP_RET_OK;
}

static yadsl_HashMapRet
yadsl_hashmap_entry_get_value_in_list_internal(
	yadsl_HashMapEntryHead* head,
	yadsl_HashMapKey key,
	yadsl_HashMapValue** value_ptr)
{
	yadsl_HashMapEntry* prev;

	if (!yadsl_hashmap_entry_exists_in_list_internal(head, key, &prev))
		return YADSL_HASHMAP_RET_DOESNT_EXIST;

	yadsl_HashMapEntry* entry = prev ? prev->next : head->first;
	*value_ptr = entry->value;

	return YADSL_HASHMAP_RET_OK;
}

static unsigned long
yadsl_hashmap_hash_internal(
	unsigned const char* str)
{
	unsigned long hash = 5381;
	int c;

	while (c = *str++)
		hash = ((hash << 5) + hash) + c;

	return hash;
}

static unsigned long
yadsl_hashmap_index_get_internal(
	yadsl_HashMapHandle* hashmap,
	yadsl_HashMapKey key)
{
	unsigned long hashed_key = yadsl_hashmap_hash_internal(key);
	return hashed_key & ((1 << ((yadsl_HashMap*) hashmap)->size_exponent) - 1);
}

static yadsl_HashMapEntryHead*
yadsl_hashmap_entry_head_get_internal(
	yadsl_HashMapHandle* hashmap,
	yadsl_HashMapKey key)
{
	unsigned long index = yadsl_hashmap_index_get_internal(hashmap, key);
	return ((yadsl_HashMap*) hashmap)->entries[index];
}

yadsl_HashMapRet
yadsl_hashmap_entry_add(
	yadsl_HashMapHandle* hashmap,
	yadsl_HashMapKey key,
	yadsl_HashMapValue* value)
{
	yadsl_HashMapEntryHead* head = yadsl_hashmap_entry_head_get_internal(hashmap, key);
	return yadsl_hashmap_entry_add_in_list_internal(head, key, value);
}

yadsl_HashMapRet
yadsl_hashmap_entry_value_get(
	yadsl_HashMapHandle* hashmap,
	yadsl_HashMapKey key,
	yadsl_HashMapValue** value_ptr)
{
	yadsl_HashMapEntryHead* head = yadsl_hashmap_entry_head_get_internal(hashmap, key);
	return yadsl_hashmap_entry_get_value_in_list_internal(head, key, value_ptr);
}

yadsl_HashMapRet
yadsl_hashmap_entry_remove(
	yadsl_HashMapHandle* hashmap,
	yadsl_HashMapKey key)
{
	yadsl_HashMapEntryHead* head = yadsl_hashmap_entry_head_get_internal(hashmap, key);
	return yadsl_hashmap_entry_remove_from_list_internal(head, key, ((yadsl_HashMap*) hashmap)->free_entry_func);
}

static void
yadsl_hashmap_print_entry_head_internal(
	yadsl_HashMapEntryHead* head)
{
	for (yadsl_HashMapEntry* p = head->first; p; p = p->next)
		printf("%s: %p, ", p->key, p->value);
}

void
yadsl_hashmap_print(
	yadsl_HashMapHandle* hashmap)
{
	printf("{ ");
	const unsigned long size = yadsl_hashmap_get_size_internal(hashmap);
	for (unsigned long i = 0; i < size; ++i)
		yadsl_hashmap_print_entry_head_internal(((yadsl_HashMap*) hashmap)->entries[i]);
	printf("}\n");
}

static void
yadsl_hashmap_entry_head_destroy_internal(
	yadsl_HashMapEntryHead* head,
	yadsl_HashMapFreeEntryFunc free_entry_func)
{
	for (yadsl_HashMapEntry* p = head->first; p;) {
		yadsl_HashMapEntry* next = p->next;
		yadsl_hashmap_entry_destroy_internal(p, free_entry_func);
		p = next;
	}
	free(head);
}

void
yadsl_hashmap_destroy(
	yadsl_HashMapHandle* hashmap)
{
	yadsl_HashMap* hashmap_ = (yadsl_HashMap*) hashmap;
	const unsigned long size = yadsl_hashmap_get_size_internal(hashmap);
	for (unsigned long i = 0; i < size; ++i)
		yadsl_hashmap_entry_head_destroy_internal(hashmap_->entries[i], hashmap_->free_entry_func);
	free(hashmap_->entries);
	free(hashmap);
}
