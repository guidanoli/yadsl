#include <yatester/runner.h>
#include <yatester/yatester.h>
#include <yatester/cmdhdl.h>
#include <yatester/err.h>
#include <yatester/builtins.h>

#include <stdio.h>
#include <setjmp.h>

static jmp_buf* curr_env;
static const yatester_command* curr_command;

yatester_status yatester_call(const char* commandname, int argc, char** argv)
{
	const yatester_command *command, *prev_command;
	yatester_status status;
	jmp_buf env, *prev_env;

	command = yatester_getcommand(commandname);

	if (command == NULL)
	{
		return yatester_report(YATESTER_BADCALL, "command \"%s\" not found", commandname);
	}

	/* Assumes AT_MOST and AT_LEAST are involutions */
	if (command->argc < 0)
	{
		if (argc > AT_MOST(command->argc))
		{
			return yatester_report(YATESTER_BADCALL, "command \"%s\" expected at most %d argument(s), but got %d", commandname, AT_MOST(command->argc), argc);
		}
	}
	else
	{
		if (argc < AT_LEAST(command->argc))
		{
			return yatester_report(YATESTER_BADCALL, "command \"%s\" expected at least %d argument(s), but got %d", commandname, AT_LEAST(command->argc), argc);
		}
	}

	prev_env = curr_env;
	curr_env = &env;

	prev_command = curr_command;
	curr_command = command;

	status = setjmp(env);

	if (status == YATESTER_OK)
	{
		command->handler(argc, argv);
	}

	curr_command = prev_command;

	curr_env = prev_env;

	return status;
}

void yatester_raise(yatester_status status)
{
	if (status != YATESTER_OK)
	{
		longjmp(*curr_env, status);
	}
}

void yatester_assert_function(const char* code, yatester_status status, int condition)
{
	if (!condition && status != YATESTER_OK)
	{
		yatester_raise(yatester_report(status, "failed assertion \"%s\" in command \"%s\"", code, curr_command->name));
	}
}

yatester_status yatester_report(yatester_status status, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	status = yatester_vreport(status, fmt, va);
	va_end(va);
	return status;
}

yatester_status yatester_vreport(yatester_status status, const char* fmt, va_list va)
{
	static const char* prefixes[] =
	{
		[YATESTER_OK] = "report",
		[YATESTER_ERROR] = "error",
		[YATESTER_NOMEM] = "no memory",
		[YATESTER_FTLERR] = "fatal error",
		[YATESTER_MEMLK] = "memory leak",
		[YATESTER_IOERR] = "i/o error",
		[YATESTER_STXERR] = "syntax error",
		[YATESTER_BADCMD] = "bad command",
		[YATESTER_BADCALL] = "bad call",
		[YATESTER_BADARG] = "bad argument",
	};

	if (status >= 0 && status < sizeof(prefixes)/sizeof(*prefixes))
	{
		fprintf(stderr, "yatester: %s: ", prefixes[status]);
		vfprintf(stderr, fmt, va);
		fprintf(stderr, "\n");
		return status;
	}
	else
	{
		return yatester_report(YATESTER_ERROR, "bad status code %d", status);
	}
}

