#include <memdb/memdb.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <stdint.h>

#if defined(_MSC_VER)
# pragma warning(disable : 4996)
#endif

/**
 * @brief Return code used internally
*/
typedef enum
{
	YADSL_MEMDB_RET_OK = 0, /**< All went ok */
	YADSL_MEMDB_RET_COPY, /**< Found duplicate node */
	YADSL_MEMDB_RET_NOT_FOUND, /**< Node not found */
	YADSL_MEMDB_RET_MEMORY, /**< Could not allocate memory */
}
yadsl_MemDebugRet;

/* Globals */

static yadsl_MemDebugAMB* amb_list_head; /**< AMB list head (nullable) */
static size_t amb_list_size; /**< AMB list size */

static uint8_t log_channels; /**< Log channels bitmap */
static FILE* log_fp; /**< Log file pointer (nullable) */

static bool error_occurred; /**< Error occurred flag */
static bool fail_occurred; /**< Fail occurred flag */

static size_t fail_countdown; /**< Allocation fail countdown */

/* Functions */

yadsl_MemDebugAMB*
yadsl_memdb_get_amb_list()
{
	return amb_list_head;
}

bool
yadsl_memdb_log_channel_get(
	yadsl_MemDebugLogChannel log_channel)
{
	return log_channels & (1 << log_channel);
}

void
yadsl_memdb_log_channel_set(
	yadsl_MemDebugLogChannel log_channel,
	bool enable)
{
	if (enable)
		log_channels |= (1 << log_channel);
	else
		log_channels &= ~(1 << log_channel);
}

/**
 * @brief Get label for log channel
 * @param log_channel log channel
 * @return label or NULL if nonexistent
*/
static const char*
yadsl_memdb_log_channel_label_get_internal(
		yadsl_MemDebugLogChannel log_channel)
{
	switch (log_channel) {
	case YADSL_MEMDB_LOG_CHANNEL_ALLOCATION:
		return "ALLOC";
	case YADSL_MEMDB_LOG_CHANNEL_DEALLOCATION:
		return "DEALLOC";
	case YADSL_MEMDB_LOG_CHANNEL_LEAKAGE:
		return "LEAK";
	default:
		return NULL;
	}
}

/**
 * @brief Log message to a specific channel
 * @param log_channel log channel
 * @param format see fprintf function family
 * @param ... see fprintf function family
*/
static void
yadsl_memdb_log_internal(
		yadsl_MemDebugLogChannel log_channel,
		const char* format,
		...)
{
	va_list va;
	const char* label;
	FILE* fp;

	/* Check if log channel is temp */
	if (!yadsl_memdb_log_channel_get(log_channel))
		return;

	/* Use stderr by default if no file pointer is given */
	fp = log_fp ? log_fp : stderr;

	/* Start varadic arguments */
	va_start(va, format);

	/* Get log channel label */
	label = yadsl_memdb_log_channel_label_get_internal(log_channel);

	if (label)
		fprintf(fp, "[MEMDB<<%s] ", label);
	else
		fprintf(fp, "[MEMDB] ");
	
	vfprintf(fp, format, va);

	/* End varadic arguments */
	va_end(va);

	fprintf(fp, "\n");
}

bool
yadsl_memdb_error_occurred()
{
	return error_occurred;
}

bool
yadsl_memdb_fail_occurred()
{
	return fail_occurred;
}

/**
 * @brief Checks if AMB total count coincides with failing countdown and
 *        whether failing by countdown is enabled
 * @return whether allocation should fail (true) or not (false)
*/
static bool
yadsl_memdb_fail_by_countdown_internal()
{
	return fail_countdown == 1;
}

/**
 * @brief Checks if memory allocation should fail
 * @return whether allocation should fail (true) or not (false)
*/
static bool
yadsl_memdb_fail_internal(
		const char* func,
		size_t size,
		const char* file,
		const int line)
{
	bool fail = yadsl_memdb_fail_by_countdown_internal();
	/* Register that failure occurred */
	fail_occurred = fail_occurred || fail;
	if (fail) {
		/* Log failure */
		yadsl_memdb_log_internal(YADSL_MEMDB_LOG_CHANNEL_ALLOCATION,
			"%s(%zu) @ %s:%d -> %p (FAILED ARTIFICIALLY)",
			func, size, file, line, NULL);
	}
	return fail;
}

/**
 * @brief Find node holding AMB
 * @param camb pointer to an AMB
 * @param prev_ptr previous node (NULL = first node)
 * @return
 * * node, and *prev_ptr is updated if not NULL
 * * NULL if node was not found
*/
static yadsl_MemDebugAMB*
yadsl_memdb_find_amb_internal(
		void* amb,
		yadsl_MemDebugAMB** prev_ptr)
{
	yadsl_MemDebugAMB* node = amb_list_head, *prev = NULL;
	for (; node; node = node->next) {
		if (node->amb == amb) {
			if (prev_ptr)
				*prev_ptr = prev;
			return node;
		}
		prev = node;
	}
	return NULL;
}

/**
 * @brief Add node to AMB list
 * @params see yadsl_MemDebugAMB
 * @return
 * * ::YADSL_MEMDB_RET_OK
 * * ::YADSL_MEMDB_RET_COPY
 * * ::YADSL_MEMDB_RET_MEMORY
*/
static yadsl_MemDebugRet
yadsl_memdb_add_amb_internal(
		const char* funcname,
		void* amb,
		size_t size,
		const char* file,
		const int line)
{
	yadsl_MemDebugAMB* node;

	/* Try to find node holding AMB */
	if (node = yadsl_memdb_find_amb_internal(amb, NULL)) {

		/* Log copy error */
		yadsl_memdb_log_internal(YADSL_MEMDB_LOG_CHANNEL_ALLOCATION,
			"Tried to add %p (%zuB @ %s:%d by %s) to the list but found copy "
			" (%zuB @ %s:%d by %s)",
			amb, size, file, line, funcname,
			node->size, node->file, node->line, node->funcname);

		error_occurred = true;

		return YADSL_MEMDB_RET_COPY;
	}

	/* Allocate node */
	node = malloc(sizeof(yadsl_MemDebugAMB));
	if (node) {
		node->amb = amb;
		node->size = size;
		node->file = file;
		node->line = line;
		node->next = amb_list_head;
		node->funcname = funcname;
		
		/* Append node to AMB list */
		amb_list_head = node;
		++amb_list_size;

		/* Decrement countdown */
		if (fail_countdown > 0)
			--fail_countdown;

		/* Log allocation */
		yadsl_memdb_log_internal(YADSL_MEMDB_LOG_CHANNEL_ALLOCATION,
			"%s(%zu) @ %s:%d -> %p",
			funcname, size, file, line, amb);

		return YADSL_MEMDB_RET_OK;
	} else {
		return YADSL_MEMDB_RET_MEMORY;
	}
}

/**
 * @brief Remove node from AMB list
 * @param camb pointer to AMB
 * @return
 * * ::YADSL_MEMDB_RET_OK
 * * ::YADSL_MEMDB_RET_NOT_FOUND
*/
static yadsl_MemDebugRet
yadsl_memdb_remove_amb_internal(
		void* amb)
{
	yadsl_MemDebugAMB* node, *prev;

	/* Try to find node holding AMB */
	node = yadsl_memdb_find_amb_internal(amb, &prev);

	if (node) {
		/* Remove node */
		if (prev == NULL)
			amb_list_head = node->next;
		else
			prev->next = node->next;

		--amb_list_size;

		/* Log deallocation */
		yadsl_memdb_log_internal(YADSL_MEMDB_LOG_CHANNEL_DEALLOCATION,
			"free(%p) <- %s(%zu) @ %s:%d",
			node->amb, node->funcname, node->size, node->file, node->line);

		/* Deallocate node */
		free(node);

		return YADSL_MEMDB_RET_OK;
	} else {
		/* Log failed deallocation */
		yadsl_memdb_log_internal(YADSL_MEMDB_LOG_CHANNEL_DEALLOCATION,
			"free(%p) <- ?(?) @ ?:?", amb);

		error_occurred = true;

		return YADSL_MEMDB_RET_NOT_FOUND;
	}
}

bool
yadsl_memdb_contains_amb(
		void* amb)
{
	return yadsl_memdb_find_amb_internal(amb, NULL) != NULL;
}

void
yadsl_memdb_clear_amb_list()
{
	bool temp;

	/* Check if list is not empty */
	if (amb_list_size) {

		/* Log leakage */
		yadsl_memdb_log_internal(YADSL_MEMDB_LOG_CHANNEL_LEAKAGE,
			"%zu leak(s) detected:", amb_list_size);
	}

	/* Temporarily turn on the deallocation log channel */
	temp = yadsl_memdb_log_channel_get(YADSL_MEMDB_LOG_CHANNEL_DEALLOCATION);
	yadsl_memdb_log_channel_set(YADSL_MEMDB_LOG_CHANNEL_DEALLOCATION, true);

	while (amb_list_size) {
		void* amb = amb_list_head->amb;
		yadsl_memdb_remove_amb_internal(amb);
		free(amb);
	}

	yadsl_memdb_log_channel_set(YADSL_MEMDB_LOG_CHANNEL_DEALLOCATION, temp);
}

void
yadsl_memdb_clear_amb_list_from_file(const char* file)
{
	bool temp;
	yadsl_MemDebugAMB* node, *next;
	size_t mem_leaks = 0;

	temp = yadsl_memdb_log_channel_get(YADSL_MEMDB_LOG_CHANNEL_DEALLOCATION);
	yadsl_memdb_log_channel_set(YADSL_MEMDB_LOG_CHANNEL_DEALLOCATION, true);

	for (node = amb_list_head; node; node = node->next)
		if (strcmp(node->file, file) == 0)
			++mem_leaks;

	if (mem_leaks) {

		/* Log leakage */
		yadsl_memdb_log_internal(YADSL_MEMDB_LOG_CHANNEL_LEAKAGE,
			"%zu leak(s) detected:", mem_leaks);
	}

	for (node = amb_list_head; node; node = next) {
		next = node->next;
		if (strcmp(node->file, file) == 0) {
			void* amb = node->amb;
			yadsl_memdb_remove_amb_internal(amb);
			free(amb);
		}
	}

	yadsl_memdb_log_channel_set(YADSL_MEMDB_LOG_CHANNEL_DEALLOCATION, temp);
}

size_t
yadsl_memdb_get_fail_countdown()
{
	return fail_countdown;
}

void
yadsl_memdb_set_fail_countdown(
		size_t countdown)
{
	fail_countdown = countdown;
}

size_t
yadsl_memdb_amb_list_size()
{
	return amb_list_size;
}

void
yadsl_memdb_set_logger(
		FILE* fp)
{
	log_fp = fp;
}

void
yadsl_memdb_dump(
		FILE* fp,
		void* mem)
{
	yadsl_MemDebugAMB* amb = yadsl_memdb_find_amb_internal(mem, NULL);
	if (amb == NULL) {
		fputc('?', fp);
	} else {
		unsigned char* c = mem;
		size_t size = amb->size;
		while (size--) fprintf(fp, "%02X", (unsigned int) *(c++));
	}
}

void
yadsl_memdb_free(
	void* amb)
{
	/* Remove AMB node from list */
	yadsl_memdb_remove_amb_internal(amb);

	/* Deallocate AMB */
	free(amb);
}

void*
yadsl_memdb_malloc(
		size_t size,
		const char* file,
		const int line)
{
	void* amb = NULL; /* Allocated memory block */

	if (!yadsl_memdb_fail_internal("malloc", size, file, line)) {
		amb = malloc(size);
		if (amb) {
			/* If allocation succeeded, add AMB node */
			if (yadsl_memdb_add_amb_internal("malloc", amb, size, file, line)) {
				/* If node could not be added, deallocate AMB */
				free(amb);
				amb = NULL;
			}
		}
	}

	return amb;
}

void*
yadsl_memdb_realloc(
		void* amb,
		size_t size,
		const char* file,
		const int line)
{
	void* ramb = NULL; /* Reallocated memory block */

	if (!yadsl_memdb_fail_internal("realloc", size, file, line)) {
		yadsl_MemDebugAMB* node;

		/* Try finding AMB node in list */
		node = yadsl_memdb_find_amb_internal(amb, NULL);
		if (node) {
			/* If found, reallocate memory block */
			ramb = realloc(amb, size);
			if (ramb) {
				/* If reallocation succedded, update AMB node */
				node->file = file;
				node->funcname = "realloc";
				node->line = line;
				node->amb = ramb;
				node->size = size;
			}
			yadsl_memdb_log_internal(YADSL_MEMDB_LOG_CHANNEL_ALLOCATION,
				"realloc(%p, %zu) @ %s:%d -> %p",
				amb, size, file, line, ramb);
		} else {
			/* Log reallocation error */
			yadsl_memdb_log_internal(YADSL_MEMDB_LOG_CHANNEL_ALLOCATION,
				"realloc(%p, %zu) @ %s:%d -> %p (NODE NOT FOUND)",
				amb, size, file, line, NULL);

			error_occurred = true;
		}
	}
	
	return ramb;
}

void*
yadsl_memdb_calloc(
		size_t cnt,
		size_t size,
		const char* file,
		const int line)
{
	void* camb = NULL; /* Cleanly allocated memory block */

	if (!yadsl_memdb_fail_internal("calloc", size*cnt, file, line)) {
		camb = calloc(cnt, size);
		if (camb) {
			/* If cleanly allocation succeeded, add AMB node */
			if (yadsl_memdb_add_amb_internal("calloc", camb, size*cnt, file, line)) {
				/* If node could not be added, deallocate AMB */
				free(camb);
				camb = NULL;
			}
		}
	}

	return camb;
}
