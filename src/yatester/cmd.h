#ifndef yatester_cmd_h
#define yatester_cmd_h

/* Commands */

#include <stddef.h>

typedef struct
{
	const char *name;
	size_t argc;
	void (*handler)(const char** argv);
}
yatester_command;

/* Terminates on command with name = NULL */
extern const yatester_command yatester_commands[];

#endif
