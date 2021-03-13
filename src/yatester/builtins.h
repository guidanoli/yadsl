#ifndef yatester_builtins_h
#define yatester_builtins_h

/* Built-in commands */

#include <yatester/cmd.h>

void yatester_builtin_throw(const char** argv);
void yatester_builtin_expect(const char** argv);

extern const yatester_command yatester_builtin_commands[];

#endif
