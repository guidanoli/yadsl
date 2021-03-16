#include <yatester/yatester.h>
#include <yatester/cmdhdl.h>

#include <string.h>

static const char* commandname_;
static int hascommand_;
static int longcmdcalled;

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

static void throw_cmd(const char** argv)
{
	yatester_throw();
}

static void long_cmd(const char** argv)
{
	longcmdcalled = 1;
}

static void assertlongcmdcalled_cmd(const char** argv)
{
	yatester_assert(longcmdcalled);
}

static void streq_cmd(const char** argv)
{
	yatester_assert(strcmp(argv[0], argv[1]) == 0);
}

static void strlen_cmd(const char** argv)
{
	size_t len;
	yatester_assert(sscanf(argv[0], "%zu", &len) == 1);
	yatester_assert(len == strlen(argv[1]));
}

static void sum16_cmd(const char** argv)
{
	int total, acc = 0;

	yatester_assert(sscanf(argv[0], "%d", &total) == 1);

	for (int i = 0; i < 16; ++i)
	{
		int num;
		yatester_assert(sscanf(argv[i+1], "%d", &num) == 1);
		acc += num;
	}

	yatester_assert(total == acc);
}

static void noerr_cmd(const char** argv)
{
}

const yatester_command yatester_commands[] =
{
	{ "throw", 0, throw_cmd },
	{ "hascommand", 1, hascommand_cmd },
	{ "assert", 1, assert_cmd },
	{ "reallylongcommandwithmorecharactersthanthecommandbufferofsixtyfourcharacters", 0, long_cmd },
	{ "assertlongcmdcalled", 0, assertlongcmdcalled_cmd },
	{ "streq", 2, streq_cmd },
	{ "strlen", 2, strlen_cmd },
	{ "sumsixteen", 17, sum16_cmd },
	{ "noerror", 0, noerr_cmd },
	{ NULL, 0, NULL },
};
