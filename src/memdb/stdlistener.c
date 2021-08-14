#include <memdb/stdlistener.h>

#include <memdb/memdb.h>

#include <assert.h>
#include <string.h>
#include <stdarg.h>

typedef enum
{
	ALLOCATION,
	DEALLOCATION,
}
channel_t;

static const char* channel_names[] =
{
	[ALLOCATION] = "ALLOCATION",
	[DEALLOCATION] = "DEALLOCATION",
};

static yadsl_MemDebugListenerHandle* g_handle; /* Listener handle */
static unsigned int g_channels; /* Channels */
static bool g_fail_occurred; /* Fail occurred */
static size_t g_countdown; /* Fail countdown */
static FILE* g_log_fp; /* Logger file pointer */

static bool get_channel(channel_t channel)
{
	return (g_channels & (1 << channel)) != 0;
}

static void set_channel(channel_t channel, bool value)
{
	if (value)
		g_channels |= (1 << channel);
	else
		g_channels &= ~(1 << channel);
}

static FILE* get_log_fp()
{
	return g_log_fp == NULL ? stderr : g_log_fp;
}

static int chwrite(channel_t channel, const char* fmt, ...)
{
	FILE* fp;
	va_list va;
	int ret;
	if (!get_channel(channel))
		return 0;
	fp = get_log_fp();
	ret = fprintf(fp, "[MEMDB<<%s] ", channel_names[channel]);
	va_start(va, fmt);
	ret += vfprintf(fp, fmt, va);
	va_end(va);
	fputc('\n', fp);
	return ret + 1;
}

static bool stdlistener_accept_cb(yadsl_MemDebugEvent const* event, void* arg)
{
	if (event->function != YADSL_MEMDB_FREE && g_countdown > 0) {
		if (--g_countdown == 0) {
			g_fail_occurred = true;
			return false;
		}
	}
	return true;
}

static void stdlistener_ack_cb(yadsl_MemDebugEvent const* event, const void* ptr, void* arg)
{
	switch (event->function) {
	case YADSL_MEMDB_FREE:
		chwrite(DEALLOCATION, "free(%p) @ %s:%d", event->free.ptr, event->file, event->line);
		break;
	case YADSL_MEMDB_MALLOC:
		chwrite(ALLOCATION, "%p <- malloc(%zu) @ %s:%d", ptr, event->malloc.size, event->file, event->line);
		break;
	case YADSL_MEMDB_REALLOC:
		chwrite(ALLOCATION, "%p <- realloc(%p, %zu) @ %s:%d", ptr, event->realloc.ptr, event->realloc.size, event->file, event->line);
		break;
	case YADSL_MEMDB_CALLOC:
		chwrite(ALLOCATION, "%p <- calloc(%zu, %zu) @ %s:%d", ptr, event->calloc.nmemb, event->calloc.size, event->file, event->line);
		break;
	}
}

bool yadsl_memdb_stdlistener_init()
{
	if (g_handle == NULL) {
		g_handle = yadsl_memdb_add_listener(
			stdlistener_accept_cb, stdlistener_ack_cb, NULL);
		if (g_handle != NULL) {
			g_channels = 0;
			g_fail_occurred = false;
			g_countdown = 0;
			g_log_fp = NULL;
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}

bool yadsl_memdb_stdlistener_fail_occurred()
{
	return g_fail_occurred;
}

void yadsl_memdb_stdlistener_set_logger(FILE* fp)
{
	g_log_fp = fp;
}

void yadsl_memdb_stdlistener_set_fail_countdown(size_t cd)
{
	g_countdown = cd;
}

bool yadsl_memdb_stdlistener_log_channel_set(const char* channel, bool value)
{
	for (size_t i = 0; i < sizeof(channel_names)/sizeof(*channel_names); ++i) {
		if (strcmp(channel_names[i], channel) == 0) {
			set_channel((channel_t)i, value);
			return true;
		}
	}
	return false;
}

bool yadsl_memdb_stdlistener_finalize()
{
	if (g_handle == NULL) {
		return false;
	}
	if (yadsl_memdb_remove_listener(g_handle)) {
		g_handle = NULL;
		return true;
	} else {
		return false;
	}
}
