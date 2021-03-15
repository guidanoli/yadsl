#include <yatester/yatester.h>
#include <yatester/cmdhdl.h>

#include <string.h>

static const char* commandname_;
static int hascommand_;

static void hascommandaux(const yatester_command* command)
{
	if (strcmp(command->name, commandname_) == 0)
	{
		hascommand_ = 1;
	}
}

static void hascommand_cmd(const char** argv)
{
	commandname_ = argv[0];
	hascommand_ = 0;
	yatester_itercommands(hascommandaux);
	yatester_assert(hascommand_);
}

static void assert_cmd(const char** argv)
{
	int condition;
	yatester_assert(sscanf(argv[0], "%d", &condition) == 1);
	yatester_assert(condition);
}

const yatester_command yatester_commands[] =
{
	{ "hascommand", 1, hascommand_cmd },
	{ "assert", 1, assert_cmd },
	{ NULL, 0, NULL },
};
