#include <yatester/builtins.h>

#include <stddef.h>

const yatester_command yatester_builtin_commands[] =
{
	{ "expect", AT_LEAST(1), yatester_builtin_expect },
	{ NULL, 0, NULL },
};
