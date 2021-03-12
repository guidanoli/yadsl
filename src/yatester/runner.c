#include <yatester/runner.h>
#include <yatester/parser.h>
#include <yatester/errhdl.h>
#include <yatester/cmdhdl.h>
#include <yatester/yatester.h>

#include <stdio.h>
#include <string.h>

yatester_status yatester_runcommand(const char* commandname, int argc, const char** argv)
{
	const yatester_command* command;
	yatester_status status;

	command = yatester_getcommand(commandname);

	if (command == NULL)
	{
		fprintf(stderr, "Could not find command named \"%s\"", commandname);
		return YATESTER_ERR;
	}

	if (command->argc != argc)
	{
		fprintf(stderr, "Command \"%s\" expected %d arguments but got %d\n", commandname, command->argc, argc);
		return YATESTER_ERR;
	}

	status = yatester_catch();

	if (status == YATESTER_OK)
	{
		command->handler((const char**) argv);
	}

	return status;
}
