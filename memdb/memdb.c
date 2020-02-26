#define __MEMDB_SUPRESS_MACROS__

#include "memdb.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#pragma once
#if defined(_MSC_VER)
# pragma warning(disable : 4996)
#endif

typedef enum
{
	MEM_RETURN_OK = 0,
	MEM_RETURN_COPY,
	MEM_RETURN_NOT_FOUND,
	MEM_RETURN_MEMORY,
}
_memdb_enum;

struct _memdb_node
{
	struct _memdb_node *next;
	const char *file;
	size_t size;
	void *mem;
	int line;
};

static struct _memdb_node *list = NULL;
static size_t listsize = 0;
static int supress = 0;
static int error_occurred = 0;

static struct _memdb_node *_memdb_get(void *_mem)
{
	struct _memdb_node *node = list;
	for (; node; node = node->next) {
		if (node->mem == _mem)
			return node;
	}
	return NULL;
}

static _memdb_enum _memdb_add(void *_mem, size_t _size, const char *file,
	const line, struct _memdb_node **pCopy)
{
	struct _memdb_node *node;
	if (node = _memdb_get(_mem)) {
		*pCopy = node;
		return MEM_RETURN_COPY;
	}
	node = malloc(sizeof(struct _memdb_node));
	if (node == NULL)
		return MEM_RETURN_MEMORY;
	node->mem = _mem;
	node->size = _size;
	node->file = file;
	node->line = line;
	node->next = list;
	list = node;
	++listsize;
	return MEM_RETURN_OK;
}

static _memdb_enum _memdb_remove(void *_mem)
{
	struct _memdb_node *node = list, *prev = NULL;
	for (; node; node = node->next) {
		if (node->mem == _mem) {
			if (prev == NULL)
				list = node->next;
			else
				prev->next = node->next;
			free(node);
			--listsize;
			return MEM_RETURN_OK;
		}
		prev = node;
	}
	return MEM_RETURN_NOT_FOUND;
}

void _memdb_supress_messages(int _supress)
{
	supress = _supress;
}

int _memdb_contains(void *_mem)
{
	return _memdb_get(_mem) != NULL;
}

void _memdb_clear_list()
{
	struct _memdb_node *node = list, *next = NULL;
	if (listsize && !supress)
		fprintf(stderr, "MEMDB: %zu leak(s) in total:\n", listsize);
	for (; node; node = next) {
		next = node->next;
		if (!supress)
			fprintf(stderr, "MEMDB: Leaked %p (%zuB), allocated in %s:%d\n",
				node->mem, node->size, node->file, node->line);
		free(node->mem);
		free(node);
	}
	list = NULL;
	listsize = 0;
}

size_t _memdb_list_size()
{
	return listsize;
}

int _memdb_error_occurred()
{
	return error_occurred;
}

void _memdb_free(void *_mem)
{
	if (_memdb_remove(_mem) == MEM_RETURN_NOT_FOUND) {
		if (!supress)
			fprintf(stderr, "MEMDB: Freeing block (%p) not in list.\n", _mem);
		error_occurred = 1;
	}
	free(_mem);
}

void *_memdb_malloc(size_t _size, const char *file, const line)
{
	void *_mem = malloc(_size);
	if (_mem) {
		struct _memdb_node *copy;
		switch (_memdb_add(_mem, _size, file, line, &copy)) {
		case MEM_RETURN_OK:
			break;
		case MEM_RETURN_COPY:
			assert(0);
			break;
		case MEM_RETURN_MEMORY:
			free(_mem);
			return NULL;
		default:
			assert(0);
		}
	}
	return _mem;
}

void *_memdb_realloc(void *_mem, size_t _size, const char *file, const line)
{
	void *_new_mem = realloc(_mem, _size);
	if (_new_mem) {
		struct _memdb_node *copy;
		_memdb_enum returnId;
		assert(!_memdb_remove(_mem));
		returnId = _memdb_add(_new_mem, _size, file, line, &copy);
		switch (returnId) {
		case MEM_RETURN_OK:
			break;
		case MEM_RETURN_COPY:
			assert(0);
			break;
		case MEM_RETURN_MEMORY:
			free(_new_mem);
			return NULL;
		default:
			assert(0);
		}
	}
	return _new_mem;
}

void *_memdb_calloc(size_t _cnt, size_t _size, const char *file, const line)
{
	void *_mem = calloc(_cnt, _size);
	if (_mem) {
		struct _memdb_node *copy;
		switch (_memdb_add(_mem, _cnt * _size, file, line, &copy)) {
		case MEM_RETURN_OK:
			break;
		case MEM_RETURN_COPY:
			assert(0);
			break;
		case MEM_RETURN_MEMORY:
			free(_mem);
			return NULL;
		default:
			assert(0);
		}
	}
	return _mem;
}

char *_memdb_strdup(const char *_str, const char *file, const line)
{
	char *_dup = strdup(_str);
	if (_dup) {
		struct _memdb_node *copy;
		switch (_memdb_add(_dup, sizeof(_dup), file, line, &copy)) {
		case MEM_RETURN_OK:
			break;
		case MEM_RETURN_COPY:
			assert(0);
			break;
		case MEM_RETURN_MEMORY:
			free(_dup);
			return NULL;
		default:
			assert(0);
		}
	}
	return _dup;
}