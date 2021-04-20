#include <yatester/builtins.h>
#include <yatester/runner.h>
#include <yatester/yatester.h>

#include <stdio.h>

/**
 * @brief Expect status code of command
 * @note Usage: expect <status-code> <command-name> [<command-arguments...>]
 */
static void yatester_builtin_expect(int argc, char** argv)
{
	int expected_status;
	yatester_status obtained_status;

	if (sscanf(argv[0], "%d", &expected_status) != 1)
	{
		yatester_raise(yatester_report(YATESTER_BADARG, "\"%s\" isn't integer", argv[0]));
	}

	obtained_status = yatester_call(argv[1], argc-2, argv+2);

	if ((yatester_status) expected_status != obtained_status)
	{
		if (obtained_status == YATESTER_OK)
		{
			obtained_status = YATESTER_ERROR;
		}

		yatester_raise(obtained_status);
	}
}

const yatester_command yatester_builtin_commands[] =
{
	{ "expect", AT_LEAST(2), yatester_builtin_expect },
	{ NULL, 0, NULL },
};
