#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>

#define _MEMDB_INTERNAL
#include <memdb/memdb.h>

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
	struct _memdb_node* next;
	const char* file;
	size_t size;
	void* mem;
	int line;
};

static struct _memdb_node* list = NULL;
static size_t listsize = 0;
static bool error_occurred = false;
static FILE* log_fp = NULL;
static float fail_rate = 0.0f;

static bool fail_bernoulli()
{
	return rand() < (int) (fail_rate * (float) RAND_MAX);
}

static void _memdb_log(const char* format, ...)
{
	FILE* fp = log_fp ? log_fp : stderr;
	va_list va;
	va_start(va, format);
	fprintf(fp, "MEMDB: ");
	vfprintf(fp, format, va);
	fprintf(fp, "\n");
	va_end(va);
}

static struct _memdb_node* _memdb_get(void* _mem)
{
	struct _memdb_node* node = list;
	for (; node; node = node->next) {
		if (node->mem == _mem)
			return node;
	}
	return NULL;
}

static _memdb_enum _memdb_add(void* _mem, size_t _size, const char* file,
	const int line, struct _memdb_node** pCopy)
{
	struct _memdb_node* node;
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

static _memdb_enum _memdb_remove(void* _mem)
{
	struct _memdb_node* node = list, * prev = NULL;
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

bool yadsl_memdb_contains(void* _mem)
{
	return _memdb_get(_mem) != NULL;
}

void yadsl_memdb_clear_list()
{
	struct _memdb_node* node = list, * next = NULL;
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

float yadsl_memdb_get_fail_rate()
{
	return fail_rate;
}

void yadsl_memdb_set_fail_rate(float rate)
{
	if (rate < 0.0f)
		fail_rate = 0.0f;
	else if (rate > 1.0f)
		fail_rate = 1.0f;
	else
		fail_rate = rate;
}

size_t yadsl_memdb_list_size()
{
	return listsize;
}

bool yadsl_memdb_error_occurred()
{
	return error_occurred;
}

void yadsl_memdb_free(void* _mem)
{
	if (_memdb_remove(_mem) == MEM_NOT_FOUND) {
		_memdb_log("Freeing block (%p) not in list.", _mem);
		error_occurred = true;
	}
	free(_mem);
}

void yadsl_memdb_set_logger(FILE* fp)
{
	log_fp = fp;
}

void* yadsl_memdb_malloc(size_t _size, const char* file, const int line)
{
	void* _mem;
	if (fail_bernoulli()) {
		fprintf(stderr, "MEMDB: Failing malloc (%zuB) in %s:%d.\n",
			_size, file, line);
		return NULL;
	}
	_mem = malloc(_size);
	if (_mem) {
		struct _memdb_node* copy;
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

void* yadsl_memdb_realloc(void* _mem, size_t _size, const char* file, const int line)
{
	void* _new_mem;
	if (fail_bernoulli()) {
		fprintf(stderr, "MEMDB: Failing realloc (%zuB) in %s:%d.\n",
			_size, file, line);
		return NULL;
	}
	_new_mem = realloc(_mem, _size);
	if (_new_mem) {
		struct _memdb_node* copy;
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

void* yadsl_memdb_calloc(size_t _cnt, size_t _size, const char* file, const int line)
{
	void* _mem;
	if (fail_bernoulli()) {
		fprintf(stderr, "MEMDB: Failing calloc (%zuB) in %s:%d.\n",
			_size, file, line);
		return NULL;
	}
	_mem = calloc(_cnt, _size);
	if (_mem) {
		struct _memdb_node* copy;
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