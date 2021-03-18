#ifndef yatester_builtins_h
#define yatester_builtins_h

/* Built-in commands */

#include <yatester/cmd.h>

/**
 * @brief Expect next command status code
 * @note Usage: /expect <status-code>
 */
void yatester_builtin_expect(size_t argc, const char** argv);

/**
 * @brief NULL-terminated array of built-in commands
 */
extern const yatester_command yatester_builtin_commands[];

#endif
