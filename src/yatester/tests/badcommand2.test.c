#include <yatester/yatester.h>

static void expect_cmd(size_t argc, const char** argv)
{
}

const yatester_command yatester_commands[] =
{
	{ "expect", 0, expect_cmd },
	{ NULL, 0, NULL },
};
