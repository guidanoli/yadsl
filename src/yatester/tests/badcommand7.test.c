#include <yatester/yatester.h>

static void my_cmd(int argc, char** argv)
{
}

const yatester_command yatester_commands[] =
{
	{ "-", AT_LEAST(0), my_cmd },
	{ NULL, 0, NULL },
};
