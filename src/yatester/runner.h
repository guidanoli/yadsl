#ifndef yatester_runner_h
#define yatester_runner_h

/* Test Runner */

#include <yatester/err.h>

#include <stddef.h>

yatester_status yatester_runcommand(const char* commandname, size_t argc, const char** argv);

#endif
