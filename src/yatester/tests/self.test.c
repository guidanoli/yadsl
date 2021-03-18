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

static void hascommand_cmd(size_t argc, char** argv)
{
	commandname_ = argv[0];
	hascommand_ = 0;
	yatester_itercommands(hascommandaux);
	commandname_ = NULL;
	yatester_assert(YATESTER_ERROR, hascommand_);
}

static void assert_cmd(size_t argc, char** argv)
{
	int condition;
	yatester_assert(YATESTER_ERROR, sscanf(argv[0], "%d", &condition) == 1);
	yatester_assert(YATESTER_ERROR, condition);
}

static void raise_cmd(size_t argc, char** argv)
{
	int status;
	yatester_assert(YATESTER_ERROR, sscanf(argv[0], "%d", &status) == 1);
	yatester_raise(status);
}

static void long_cmd(size_t argc, char** argv)
{
	longcmdcalled = 1;
}

static void assertlongcmdcalled_cmd(size_t argc, char** argv)
{
	yatester_assert(YATESTER_ERROR, longcmdcalled);
}

static void streq_cmd(size_t argc, char** argv)
{
	yatester_assert(YATESTER_ERROR, strcmp(argv[0], argv[1]) == 0);
}

static void strlen_cmd(size_t argc, char** argv)
{
	size_t len;
	yatester_assert(YATESTER_ERROR, sscanf(argv[0], "%zu", &len) == 1);
	yatester_assert(YATESTER_ERROR, len == strlen(argv[1]));
}

static void sum_cmd(size_t argc, char** argv)
{
	int total;

	yatester_assert(YATESTER_ERROR, sscanf(argv[0], "%d", &total) == 1);

	for (size_t argi = 1; argi < argc; ++argi)
	{
		int num;
		yatester_assert(YATESTER_ERROR, sscanf(argv[argi], "%d", &num) == 1);
		total -= num;
	}

	yatester_assert(YATESTER_ERROR, total == 0);
}

static void noerr_cmd(size_t argc, char** argv)
{
}

static void malloc_cmd(size_t argc, char** argv)
{
	size_t size;
	yatester_assert(YATESTER_ERROR, sscanf(argv[0], "%zu", &size) == 1);
	yatester_assert(YATESTER_NOMEM, malloc(size) != NULL);
}

const yatester_command yatester_commands[] =
{
	{ "raise", 1, raise_cmd },
	{ "r-a-i-s-e", 1, raise_cmd },
	{ "r4153", 1, raise_cmd },
	{ "hascommand", 1, hascommand_cmd },
	{ "assert", 1, assert_cmd },
	{ "reallylongcommandwithmorecharactersthanthecommandbufferofsixtyfourcharacters", 0, long_cmd },
	{ "assertlongcmdcalled", 0, assertlongcmdcalled_cmd },
	{ "streq", 2, streq_cmd },
	{ "strlen", 2, strlen_cmd },
	{ "sum", 1, sum_cmd },
	{ "noerror", 0, noerr_cmd },
	{ "malloc", 1, malloc_cmd },
	{ NULL, 0, NULL },
};
