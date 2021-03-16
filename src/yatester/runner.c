#include <yatester/runner.h>
#include <yatester/parser.h>
#include <yatester/err.h>
#include <yatester/cmdhdl.h>
#include <yatester/builtins.h>
#include <yatester/errhdl.h>
#include <yatester/yatester.h>

#include <stdio.h>
#include <string.h>
#include <setjmp.h>

static jmp_buf env;

void yatester_builtin_expect(const char** argv)
{
	int status;
	yatester_assert(sscanf(argv[0], "%d", &status) == 1);
	yatester_pushexpectedstatus(status);
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

	return yatester_evaluatestatus(status);
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
