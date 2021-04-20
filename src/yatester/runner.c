#include <yatester/runner.h>
#include <yatester/yatester.h>
#include <yatester/cmdhdl.h>
#include <yatester/err.h>
#include <yatester/builtins.h>

#include <stdio.h>
#include <setjmp.h>

static jmp_buf* env_ptr;

yatester_status yatester_call(const char* commandname, int argc, char** argv)
{
	const yatester_command* command;
	yatester_status status;
	jmp_buf env, *old_env_ptr;

	command = yatester_getcommand(commandname);

	if (command == NULL)
	{
		return yatester_report(YATESTER_BADCALL, "Command \"%s\" not found", commandname);
	}

	/* Assumes AT_MOST and AT_LEAST are involutions */
	if (command->argc < 0)
	{
		if (argc > AT_MOST(command->argc))
		{
			return yatester_report(YATESTER_BADCALL, "Command \"%s\" expected at most %d argument(s), but got %d", commandname, AT_MOST(command->argc), argc);
		}
	}
	else
	{
		if (argc < AT_LEAST(command->argc))
		{
			return yatester_report(YATESTER_BADCALL, "Command \"%s\" expected at least %d argument(s), but got %d", commandname, AT_LEAST(command->argc), argc);
		}
	}

	old_env_ptr = env_ptr;
	env_ptr = &env;

	status = setjmp(env);

	if (status == YATESTER_OK)
	{
		command->handler(argc, argv);
	}

	env_ptr = old_env_ptr;

	return status;
}

void yatester_raise(yatester_status status)
{
	if (status != YATESTER_OK)
	{
		longjmp(*env_ptr, status);
	}
}

void yatester_assert_function(const char* code, const char* file, int line, yatester_status status, int condition)
{
	if (!condition && status != YATESTER_OK)
	{
		yatester_raise(yatester_report(status, "Failed assertion \"%s\" in \"%s\", line %d", code, file, line));
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
	static const char* preffixes[] =
	{
		[YATESTER_OK] = "Report",
		[YATESTER_ERROR] = "Error",
		[YATESTER_NOMEM] = "No memory",
		[YATESTER_FTLERR] = "Fatal error",
		[YATESTER_MEMLK] = "Memory leak",
		[YATESTER_IOERR] = "I/O error",
		[YATESTER_STXERR] = "Syntax error",
		[YATESTER_BADCMD] = "Bad command",
		[YATESTER_BADCALL] = "Bad call",
		[YATESTER_BADARG] = "Bad argument",
	};

	if (status >= 0 && status < sizeof(preffixes)/sizeof(*preffixes))
	{
		fprintf(stderr, "%s: ", preffixes[status]);
		vfprintf(stderr, fmt, va);
		fprintf(stderr, "\n");
		return status;
	}
	else
	{
		return yatester_report(YATESTER_ERROR, "Bad status code %d", status);
	}
}

