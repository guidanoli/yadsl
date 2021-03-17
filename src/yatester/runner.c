#include <yatester/runner.h>
#include <yatester/yatester.h>
#include <yatester/cmdhdl.h>
#include <yatester/err.h>
#include <yatester/builtins.h>

#include <stdio.h>
#include <setjmp.h>

static jmp_buf env;
static yatester_status statusq[2];

void yatester_builtin_expect(const char** argv)
{
	statusq[1] = YATESTER_ERROR;
}

static yatester_status evalstatus_internal(yatester_status status)
{
	if (status == statusq[0])
	{
		status = YATESTER_OK;
	}
	else if (status == YATESTER_OK)
	{
		status = YATESTER_NOERROR;
	}

	statusq[0] = statusq[1];
	statusq[1] = YATESTER_OK;

	return status;
}


yatester_status yatester_runcommand(const char* commandname, size_t argc, const char** argv)
{
	const yatester_command* command;
	yatester_status status;

	command = yatester_getcommand(commandname);

	if (command == NULL)
	{
		fprintf(stderr, "Could not find command named \"%s\"\n", commandname);
		return YATESTER_NOCMD;
	}

	if (command->argc != argc)
	{
		fprintf(stderr, "Command \"%s\" expected %zu argument(s) but got %zu\n", commandname, command->argc, argc);
		return YATESTER_CMDARGCMM;
	}

	status = setjmp(env);

	if (status == YATESTER_OK)
	{
		command->handler((const char**) argv);
	}

	return evalstatus_internal(status);
}

void yatester_throw()
{
	longjmp(env, YATESTER_ERROR);
}

void yatester_assert_function(const char* code, const char* file, int line, int condition)
{
	if (!condition)
	{
		fprintf(stderr, "Assertion \"%s\" failed in \"%s\", line %d\n", code, file, line);
		longjmp(env, YATESTER_ERROR);
	}
}

void yatester_notnull_function(const char* code, const char* file, int line, void* p)
{
	if (p == NULL)
	{
		fprintf(stderr, "Null pointer \"%s\" caught in \"%s\", line %d\n", code, file, line);
		longjmp(env, YATESTER_NOMEM);
	}
}
