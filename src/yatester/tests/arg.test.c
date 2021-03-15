#include <yatester/yatester.h>

#include <stdio.h>
#include <string.h>

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

const yatester_command yatester_commands[] =
{
	{ "streq", 2, streq_cmd },
	{ "strlen", 2, strlen_cmd },
	{ NULL, 0, NULL },
};
