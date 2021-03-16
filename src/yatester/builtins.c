#include <yatester/builtins.h>

const yatester_command yatester_builtin_commands[] =
{
	{ "expect", 1, yatester_builtin_expect },
	{ NULL, 0, NULL },
};
