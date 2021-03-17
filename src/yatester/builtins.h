#ifndef yatester_builtins_h
#define yatester_builtins_h

/* Built-in commands */

#include <yatester/cmd.h>

/**
 * @brief Expect next command fails
 * @note Only handles YATESTER_ERROR
 * @param argv argument vector
 */
void yatester_builtin_expect(const char** argv);

/**
 * @brief NULL-terminated array of built-in commands
 */
extern const yatester_command yatester_builtin_commands[];

#endif
