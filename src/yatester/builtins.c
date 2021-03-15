#include <yatester/builtins.h>
#include <yatester/err.h>

#include <stdio.h>
#include <stddef.h>

const yatester_command yatester_builtin_commands[] =
{
	{ "throw", 1, yatester_builtin_throw },
	{ "expect", 1,yatester_builtin_expect },
	{ NULL, 0, NULL },
};

void yatester_builtin_throw(const char** argv)
{
	int status;
	yatester_assert(sscanf(argv[0], "%d", &status) == 1);
	yatester_throw(status);
}
