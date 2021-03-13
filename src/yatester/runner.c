#include <yatester/runner.h>
#include <yatester/parser.h>
#include <yatester/err.h>
#include <yatester/cmdhdl.h>
#include <yatester/yatester.h>

#include <stdio.h>
#include <string.h>
#include <setjmp.h>

static jmp_buf env;

#define PLURAL(n) ((n) == 1 ? "" : "s")

yatester_status yatester_runcommand(const char* commandname, int argc, const char** argv)
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
		const char* plural_ending = command->argc == 1 ? "" : "s";
		fprintf(stderr, "Command \"%s\" expected %d argument%s but got %d\n", commandname, command->argc, PLURAL(command->argc), argc);
		return YATESTER_ERR;
	}

	status = setjmp(env);

	if (status == YATESTER_OK)
	{
		command->handler((const char**) argv);
	}

	return status;
}

void yatester_throw(yatester_status status)
{
	longjmp(env, status);
}
