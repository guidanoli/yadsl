#ifndef yatester_runner_h
#define yatester_runner_h

/* Test Runner */

#include <yatester/status.h>

#include <stddef.h>

/**
 * @brief Call command
 * @param commandname command name
 * @param argc argument count
 * @param argv argument vector
 * @return operation status code
 */
yatester_status yatester_call(const char* commandname, size_t argc, char** argv);

#endif
