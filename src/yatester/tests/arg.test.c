#include <yatester/yatester.h>

#include <stdio.h>
#include <string.h>

static void streq_cmd(const char** argv)
{
	if (strcmp(argv[0], argv[1]) != 0)
	{
		fprintf(stderr, "\"%s\" != \"%s\"\n", argv[0], argv[1]);
		yatester_throw(YATESTER_ERR);
	}
}

static void strlen_cmd(const char** argv)
{
	size_t expected, obtained;

	if (sscanf(argv[0], "%zu", &expected) != 1)
	{
		fprintf(stderr, "\"%s\" is not an integer\n", argv[0]);
		yatester_throw(YATESTER_ERR);
	}

	obtained = strlen(argv[1]);

	if (expected != obtained)
	{
		fprintf(stderr, "%zu != %zu\n", expected, obtained);
		yatester_throw(YATESTER_ERR);
	}
}

const yatester_command yatester_commands[] =
{
	{ "streq", 2, streq_cmd },
	{ "strlen", 2, strlen_cmd },
	{ NULL, 0, NULL },
};
