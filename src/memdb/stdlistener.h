#ifndef __YADSL_MEMDB_STDLISTENER_H__
#define __YADSL_MEMDB_STDLISTENER_H__

#include <stdio.h>
#include <stdbool.h>

bool yadsl_memdb_stdlistener_init();
bool yadsl_memdb_stdlistener_fail_occurred();
void yadsl_memdb_stdlistener_set_logger(FILE* fp);
void yadsl_memdb_stdlistener_set_fail_countdown(size_t cd);
bool yadsl_memdb_stdlistener_log_channel_set(const char* channel, bool value);
bool yadsl_memdb_stdlistener_finalize();

#endif
