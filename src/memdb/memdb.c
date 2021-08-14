#include <memdb/memdb.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <assert.h>

#include <memdb/list.h>

/**
 * Listener list node
 * ==================
 *
 * Each node contains an accept callback (accept_cb) and an
 * acknowledge callback (ack_cb) that are called with the
 * auxiliar argument (arg). These nodes are linked through
 * the next field in a single-linked list fashion.
 *
 * Invariants
 * ----------
 * LLN1. The 'next' field points to valid node or to NULL
 * LLN2. accept_cb and ack_cb point to valid functions or to NULL *
*/
struct yadsl_MemDebugListener
{
	struct yadsl_MemDebugListener* next; /* nullable */
	yadsl_MemDebugAcceptCallback accept_cb; /* nullable */
	yadsl_MemDebugAcknowledgeCallback ack_cb; /* nullable */
	yadsl_MemDebugListenerArgument* arg; /* nullable */
};

typedef struct yadsl_MemDebugListener Listener;

/**
 * Listener list
 * =============
 *
 * A single-linked list with nodes containing information about the
 * registered listeners. The first node in the list is pointed by
 * g_listener_head and is here referred to as the 'head'. For iterating
 * through listeners in a safe way, we also maintain a pointer to the
 * current listener, g_current_listener, that we here refer to as 'cursor'.
 * 
 * Invariants
 * ----------
 *  LL0. The list does not have cycles
 *  LL1. If the head is NULL, then the cursor is NULL
 *  LL2. If the head is not NULL, then the cursor is either NULL or a valid node
 *       that is accessible via the head and the 'next' field of its nodes
 *
 *  Rules
 *  -----
 *  LL3. If the cursor is NULL, then no iteration is taking place
 *  LL4. If the cursor is not NULL, then iteration is taking place and cannot
 *       be overriden except for the function responsible for the iteration
*/
static Listener* g_listener_head; /* First listener */
static Listener* g_current_listener; /* Current listener */

/* Helper macro for checking if iteration is taking place */
#define IS_LISTENING (g_current_listener != NULL)

#ifdef YADSL_DEBUG
/* Helper function for checking invariants (for debugging) */
static void check_invariants()
{
	bool found_current = false;
	for (Listener* p = g_listener_head; p != NULL; p = p->next) {
		for (Listener* q = p->next; q != NULL; q = q->next) {
			assert(p != q && "LL0");
		}
	}
	for (Listener* p = g_listener_head; p != NULL; p = p->next) {
		if (p == g_current_listener) {
			found_current = true;
		}
	}
	if (g_listener_head == NULL) {
		assert(g_current_listener == NULL && "LL1");
	} else {
		assert((g_current_listener == NULL || found_current) && "LL2");
	}
}
#endif

yadsl_MemDebugListenerHandle*
_yadsl_memdb_add_listener(
	yadsl_MemDebugAcceptCallback accept_cb,
	yadsl_MemDebugAcknowledgeCallback ack_cb,
	yadsl_MemDebugListenerArgument* arg)
{
	Listener* listener;
	listener = malloc(sizeof(*listener));
	if (listener != NULL) {
		listener->accept_cb = accept_cb;
		listener->ack_cb = ack_cb;
		listener->arg = arg;
		list_append((ListNode**)&g_listener_head, (ListNode*)listener);
	}
	return listener;
}

yadsl_MemDebugListenerHandle*
yadsl_memdb_add_listener(
	yadsl_MemDebugAcceptCallback accept_cb,
	yadsl_MemDebugAcknowledgeCallback ack_cb,
	yadsl_MemDebugListenerArgument* arg)
{
	yadsl_MemDebugListenerHandle* handle;
#ifdef YADSL_DEBUG
	check_invariants();
#endif
	handle = _yadsl_memdb_add_listener(accept_cb, ack_cb, arg);
#ifdef YADSL_DEBUG
	check_invariants();
	if (handle != NULL) {
		assert(g_listener_head == (Listener*)handle && "listener was added first");
	}
#endif
	return handle;
}

bool
_yadsl_memdb_remove_listener(
	yadsl_MemDebugListenerHandle* handle)
{
	Listener** listener_ptr;
	listener_ptr = (Listener**)list_find((ListNode**)&g_listener_head, (ListNode*)handle);
	if (listener_ptr == NULL) {
		return false;
	}
	free(list_remove((ListNode**)listener_ptr));
	return true;
}

bool
yadsl_memdb_remove_listener(
	yadsl_MemDebugListenerHandle* handle)
{
	bool ok;
#ifdef YADSL_DEBUG
	check_invariants();
#endif
	ok = _yadsl_memdb_remove_listener(handle);
#ifdef YADSL_DEBUG
	check_invariants();
	for (Listener* p = g_listener_head; p != NULL; p = p->next) {
		assert(p != (Listener*)handle && "listener was removed");
	}
#endif
	return ok;
}

static void
_acknowledge_listeners(yadsl_MemDebugEvent const* event, const void* ptr)
{
#ifdef YADSL_DEBUG
	yadsl_MemDebugEvent eventcopy;
	memcpy(&eventcopy, event, sizeof(eventcopy));
#endif
	
	for (g_current_listener = g_listener_head;
			g_current_listener != NULL;
			g_current_listener = g_current_listener->next)
	{
		if (g_current_listener->ack_cb != NULL)
		{
			g_current_listener->ack_cb(event, ptr, g_current_listener->arg);
#ifdef YADSL_DEBUG
			assert(memcmp(&eventcopy, event, sizeof(eventcopy)) == 0 && "event was not modified");
#endif
		}
	}
}

static void
acknowledge_listeners(yadsl_MemDebugEvent const* event, const void* ptr)
{
#ifdef YADSL_DEBUG
	assert(!IS_LISTENING && "not listening to any events");
	assert(event != NULL);
	assert(event->function >= 0 && event->function < YADSL_MEMDB_FUNCTION_MAX);
	assert(event->file != NULL);
	check_invariants();
#endif
	_acknowledge_listeners(event, ptr);
#ifdef YADSL_DEBUG
	assert(!IS_LISTENING && "not listening to any events");
	check_invariants();
#endif
}

void
yadsl_memdb_free(
	void* ptr,
	const char* file,
	int line)
{
	yadsl_MemDebugEvent event;
#ifdef YADSL_DEBUG
	/* For later memcpy and memcmp */
	memset(&event, 0, sizeof(event));
#endif
	event = (yadsl_MemDebugEvent){
		.function = YADSL_MEMDB_FREE,
		.file = file,
		.line = line,
		.free = {
			.ptr = ptr,
		},
	};

	free(ptr);

	if (!IS_LISTENING) {
		acknowledge_listeners(&event, NULL);
	}
}

static bool
_ask_listeners(yadsl_MemDebugEvent const* event)
{
#ifdef YADSL_DEBUG
	yadsl_MemDebugEvent eventcopy;
	memcpy(&eventcopy, event, sizeof(eventcopy));
#endif
	g_current_listener = g_listener_head;
	while (g_current_listener != NULL) {
		if (g_current_listener->accept_cb != NULL) {
			if (!g_current_listener->accept_cb(event, g_current_listener->arg)) {
				g_current_listener = NULL;
				return false;
			}
#ifdef YADSL_DEBUG
			assert(memcmp(&eventcopy, event, sizeof(eventcopy)) == 0 && "event was not modified");
#endif
		}
		g_current_listener = g_current_listener->next;
	}
	return true;
}

static bool
ask_listeners(yadsl_MemDebugEvent const* event)
{
	bool ok;
#ifdef YADSL_DEBUG
	assert(!IS_LISTENING && "not listening to any events");
	assert(event != NULL);
	assert(event->function >= 0 && event->function < YADSL_MEMDB_FUNCTION_MAX);
	assert(event->file != NULL);
	check_invariants();
#endif
	ok = _ask_listeners(event);
#ifdef YADSL_DEBUG
	assert(!IS_LISTENING && "not listening to any events");
	check_invariants();
#endif
	return ok;
}

void*
yadsl_memdb_malloc(
	size_t size,
	const char* file,
	int line)
{
	void* ptr;

	yadsl_MemDebugEvent event;
#ifdef YADSL_DEBUG
	/* For later memcpy and memcmp */
	memset(&event, 0, sizeof(event));
#endif
	event = (yadsl_MemDebugEvent){
		.function = YADSL_MEMDB_MALLOC,
		.file = file,
		.line = line,
		.malloc = {
			.size = size,
		},
	};

	if (!IS_LISTENING && ask_listeners(&event)) {
		ptr = malloc(size);
	} else {
		ptr = NULL;
	}

	if (!IS_LISTENING) {
		acknowledge_listeners(&event, ptr);
	}

	return ptr;
}

void*
yadsl_memdb_realloc(
	void* ptr,
	size_t size,
	const char* file,
	int line)
{
	void* newptr;

	yadsl_MemDebugEvent event;
#ifdef YADSL_DEBUG
	/* For later memcpy and memcmp */
	memset(&event, 0, sizeof(event));
#endif
	event = (yadsl_MemDebugEvent){
		.function = YADSL_MEMDB_REALLOC,
		.file = file,
		.line = line,
		.realloc = {
			.ptr = ptr,
			.size = size,
		},
	};
	
	if (!IS_LISTENING && ask_listeners(&event)) {
		newptr = realloc(ptr, size);
	} else {
		newptr = NULL;
	}

	if (!IS_LISTENING) {
		acknowledge_listeners(&event, newptr);
	}
	
	return newptr;
}

void*
yadsl_memdb_calloc(
	size_t nmemb,
	size_t size,
	const char* file,
	int line)
{
	void* ptr;

	yadsl_MemDebugEvent event;
#ifdef YADSL_DEBUG
	/* For later memcpy and memcmp */
	memset(&event, 0, sizeof(event));
#endif
	event = (yadsl_MemDebugEvent){
		.function = YADSL_MEMDB_CALLOC,
		.file = file,
		.line = line,
		.calloc = {
			.nmemb = nmemb,
			.size = size,
		},
	};

	if (!IS_LISTENING && ask_listeners(&event)) {
		ptr = calloc(nmemb, size);
	} else {
		ptr = NULL;
	}

	if (!IS_LISTENING) {
		acknowledge_listeners(&event, ptr);
	}

	return ptr;
}
