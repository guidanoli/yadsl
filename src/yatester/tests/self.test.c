#include <yatester/yatester.h>
#include <yatester/cmdhdl.h>

#include <string.h>
#include <stdio.h>

static char* commandname_;
static int hascommand_;
static int longcmdcalled;

static void hascommandaux(const yatester_command* command)
{
	if (strcmp(command->name, commandname_) == 0)
	{
		hascommand_ = 1;
	}
}

static void hascommand_cmd(int argc, char** argv)
{
	commandname_ = argv[0];
	hascommand_ = 0;
	yatester_itercommands(hascommandaux);
	commandname_ = NULL;
	yatester_assert(YATESTER_ERROR, hascommand_);
}

static void assert_cmd(int argc, char** argv)
{
	int condition;
	int status;
	yatester_assert(YATESTER_BADARG, sscanf(argv[0], "%d", &condition) == 1);
	yatester_assert(YATESTER_BADARG, sscanf(argv[1], "%d", &status) == 1);
	yatester_assert(status, condition);
}

static void raise_cmd(int argc, char** argv)
{
	int status;
	yatester_assert(YATESTER_BADARG, sscanf(argv[0], "%d", &status) == 1);
	yatester_raise(status);
}

static void long_cmd(int argc, char** argv)
{
	longcmdcalled = 1;
}

static void assertlongcmdcalled_cmd(int argc, char** argv)
{
	yatester_assert(YATESTER_ERROR, longcmdcalled);
}

static void streq_cmd(int argc, char** argv)
{
	yatester_assert(YATESTER_ERROR, strcmp(argv[0], argv[1]) == 0);
}

static void strlen_cmd(int argc, char** argv)
{
	size_t len;
	yatester_assert(YATESTER_BADARG, sscanf(argv[0], "%zu", &len) == 1);
	yatester_assert(YATESTER_ERROR, len == strlen(argv[1]));
}

static void sum_cmd(int argc, char** argv)
{
	int total;

	yatester_assert(YATESTER_BADARG, sscanf(argv[0], "%d", &total) == 1);

	for (size_t argi = 1; argi < argc; ++argi)
	{
		int num;
		yatester_assert(YATESTER_BADARG, sscanf(argv[argi], "%d", &num) == 1);
		total -= num;
	}

	yatester_assert(YATESTER_ERROR, total == 0);
}

static void noerr_cmd(int argc, char** argv)
{
}

static void malloc_cmd(int argc, char** argv)
{
	size_t size;
	yatester_assert(YATESTER_BADARG, sscanf(argv[0], "%zu", &size) == 1);
	yatester_assert(YATESTER_NOMEM, malloc(size) != NULL);
}

const yatester_command yatester_commands[] =
{
	{ "raise", AT_LEAST(1), raise_cmd },
	{ "r-a-i-s-e", AT_LEAST(1), raise_cmd },
	{ "r4153", AT_LEAST(1), raise_cmd },
	{ "hascommand", AT_LEAST(1), hascommand_cmd },
	{ "assert", AT_LEAST(2), assert_cmd },
	{ "reallylongcommandwithmorecharactersthanthecommandbufferofsixtyfourcharacters", AT_LEAST(0), long_cmd },
	{ "assertlongcmdcalled", AT_LEAST(0), assertlongcmdcalled_cmd },
	{ "streq", AT_LEAST(2), streq_cmd },
	{ "strlen", AT_LEAST(2), strlen_cmd },
	{ "sum", AT_LEAST(1), sum_cmd },
	{ "noerror", AT_LEAST(0), noerr_cmd },
	{ "malloc", AT_LEAST(1), malloc_cmd },
	{ "atleast0", AT_LEAST(0), noerr_cmd },
	{ "atmost0", AT_MOST(0), noerr_cmd },
	{ "atleast1", AT_LEAST(1), noerr_cmd },
	{ "atmost1", AT_MOST(1), noerr_cmd },
	{ NULL, AT_LEAST(0), NULL },
};
