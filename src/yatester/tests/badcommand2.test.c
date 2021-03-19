#include <yatester/yatester.h>

static void expect_cmd(int argc, char** argv)
{
}

const yatester_command yatester_commands[] =
{
	{ "expect", AT_LEAST(1), expect_cmd },
	{ NULL, 0, NULL },
};
