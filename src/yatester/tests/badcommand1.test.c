#include <yatester/yatester.h>

static void emptystringname_cmd(size_t argc, const char** argv)
{
}

const yatester_command yatester_commands[] =
{
	{ "", 0, emptystringname_cmd },
	{ NULL, 0, NULL },
};
