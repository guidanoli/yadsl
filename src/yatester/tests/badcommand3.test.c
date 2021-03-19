#include <yatester/yatester.h>

const yatester_command yatester_commands[] =
{
	{ "mycmd", AT_LEAST(0), NULL },
	{ NULL, 0, NULL },
};
