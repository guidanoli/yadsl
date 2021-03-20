#include <yatester/yatester.h>

static void evil_cmd(int argc, char** argv)
{
	exit(0);
}

const yatester_command yatester_commands[] =
{
	{ "evil", AT_LEAST(0), evil_cmd },
	{ "evil\n", AT_LEAST(0), evil_cmd },
	{ NULL, 0, NULL },
};
