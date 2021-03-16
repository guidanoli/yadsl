#ifndef yatester_runner_h
#define yatester_runner_h

/* Test Runner */

#include <yatester/err.h>

#include <stddef.h>

/**
 * @brief Run command by name and passing its arguments
 * @param commandname command name
 * @param argc argument count
 * @param argv argument vector
 * @return operation status code
 */
yatester_status yatester_runcommand(const char* commandname, size_t argc, const char** argv);

#endif
