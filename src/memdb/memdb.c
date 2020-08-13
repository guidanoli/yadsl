// Boilerplate preprocessors to temporarily
// disable macros
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>

#define _MEMDB_INTERNAL
#include "memdb.h"

#if defined(_MSC_VER)
# pragma warning(disable : 4996)
#endif

typedef enum
{
	MEM_OK = 0,
	MEM_COPY,
	MEM_NOT_FOUND,
	MEM_MEMORY,
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
static int error_occurred = 0;
static FILE *log_fp = NULL;

static void _memdb_log(const char *format, ...)
{
	FILE *fp = log_fp ? log_fp : stderr;
	va_list va;
	va_start(va, format);
	fprintf(fp, "MEMDB: ");
	vfprintf(fp, format, va);
	fprintf(fp, "\n");
	va_end(va);
}

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
	const int line, struct _memdb_node **pCopy)
{
	struct _memdb_node *node;
	if (node = _memdb_get(_mem)) {
		*pCopy = node;
		return MEM_COPY;
	}
	node = malloc(sizeof(struct _memdb_node));
	if (node == NULL)
		return MEM_MEMORY;
#ifdef _VERBOSE
	_memdb_log("Allocated %p (%zuB) in %s:%d.", _mem, _size, file, line);
#endif
	node->mem = _mem;
	node->size = _size;
	node->file = file;
	node->line = line;
	node->next = list;
	list = node;
	++listsize;
	return MEM_OK;
}

static _memdb_enum _memdb_remove(void *_mem)
{
	struct _memdb_node *node = list, *prev = NULL;
	for (; node; node = node->next) {
		if (node->mem == _mem) {
#ifdef _VERBOSE
			_memdb_log("Deallocated %p (%zuB) in %s:%d.", node->mem,
				node->size, node->file, node->line);
#endif
			if (prev == NULL)
				list = node->next;
			else
				prev->next = node->next;
			free(node);
			--listsize;
			return MEM_OK;
		}
		prev = node;
	}
	return MEM_NOT_FOUND;
}

int _memdb_contains(void *_mem)
{
	return _memdb_get(_mem) != NULL;
}

void _memdb_clear_list()
{
	struct _memdb_node *node = list, *next = NULL;
	if (listsize)
		_memdb_log("%zu leak(s) in total:", listsize);
	for (; node; node = next) {
		next = node->next;
		_memdb_log("Leaked %p (%zuB), allocated in %s:%d.",
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
	if (_memdb_remove(_mem) == MEM_NOT_FOUND) {
		_memdb_log("Freeing block (%p) not in list.", _mem);
		error_occurred = 1;
	}
	free(_mem);
}

void _memdb_set_logger(FILE *fp)
{
	log_fp = fp;
}

void *_memdb_malloc(size_t _size, const char *file, const int line)
{
	void *_mem = malloc(_size);
	if (_mem) {
		struct _memdb_node *copy;
		switch (_memdb_add(_mem, _size, file, line, &copy)) {
		case MEM_OK:
			break;
		case MEM_COPY:
			assert(0);
			break;
		case MEM_MEMORY:
			free(_mem);
			return NULL;
		default:
			assert(0);
		}
	}
	return _mem;
}

void *_memdb_realloc(void *_mem, size_t _size, const char *file, const int line)
{
	void *_new_mem = realloc(_mem, _size);
	if (_new_mem) {
		struct _memdb_node *copy;
		_memdb_enum returnId;
		if (_memdb_remove(_mem)) assert(0);
		returnId = _memdb_add(_new_mem, _size, file, line, &copy);
		switch (returnId) {
		case MEM_OK:
			break;
		case MEM_COPY:
			assert(0);
			break;
		case MEM_MEMORY:
			free(_new_mem);
			return NULL;
		default:
			assert(0);
		}
	}
	return _new_mem;
}

void *_memdb_calloc(size_t _cnt, size_t _size, const char *file, const int line)
{
	void *_mem = calloc(_cnt, _size);
	if (_mem) {
		struct _memdb_node *copy;
		switch (_memdb_add(_mem, _cnt * _size, file, line, &copy)) {
		case MEM_OK:
			break;
		case MEM_COPY:
			assert(0);
			break;
		case MEM_MEMORY:
			free(_mem);
			return NULL;
		default:
			assert(0);
		}
	}
	return _mem;
}

char *_memdb_strdup(const char *_str, const char *file, const int line)
{
	char *_dup = strdup(_str);
	if (_dup) {
		struct _memdb_node *copy;
		switch (_memdb_add(_dup, sizeof(_dup), file, line, &copy)) {
		case MEM_OK:
			break;
		case MEM_COPY:
			assert(0);
			break;
		case MEM_MEMORY:
			free(_dup);
			return NULL;
		default:
			assert(0);
		}
	}
	return _dup;
}
