#include <yatester/runner.h>
#include <yatester/parser.h>
#include <yatester/err.h>
#include <yatester/cmdhdl.h>
#include <yatester/builtins.h>
#include <yatester/yatester.h>

#include <stdio.h>
#include <string.h>
#include <setjmp.h>

static jmp_buf env;
yatester_status expected_status[2]; /* Queue */

void yatester_builtin_expect(const char** argv)
{
	int status;
	yatester_assert(sscanf(argv[0], "%d", &status) == 1);
	expected_status[1] = status; /* Push to queue */
}

yatester_status yatester_runcommand(const char* commandname, size_t argc, const char** argv)
{
	const yatester_command* command;
	yatester_status status;

	command = yatester_getcommand(commandname);

	if (command == NULL)
	{
		fprintf(stderr, "Could not find command named \"%s\"\n", commandname);
		return YATESTER_ERR;
	}

	if (command->argc != argc)
	{
		fprintf(stderr, "Command \"%s\" expected %zu argument(s) but got %zu\n", commandname, command->argc, argc);
		return YATESTER_ERR;
	}

	status = setjmp(env);

	if (status == YATESTER_OK)
	{
		command->handler((const char**) argv);
	}

	if (status == expected_status[0])
	{
		if (status != YATESTER_OK)
		{
			fprintf(stderr, "Previous error was expected\n");
		}

		/* Pop from queue */
		expected_status[0] = expected_status[1];
		expected_status[1] = YATESTER_OK;

		return YATESTER_OK;
	}
	else
	{
		if (status == YATESTER_OK)
		{
			return YATESTER_ERR;
		}
		else
		{
			return status;
		}
	}
}

void yatester_throw(yatester_status status)
{
	longjmp(env, status);
}

void yatester_assert_function(const char* code, const char* file, int line, int condition)
{
	if (!condition)
	{
		fprintf(stderr, "Assertion \"%s\" failed in \"%s\", line %d\n", code, file, line);
		yatester_throw(YATESTER_ERR);
	}
}
