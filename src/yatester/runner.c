#include <yatester/runner.h>
#include <yatester/yatester.h>
#include <yatester/cmdhdl.h>
#include <yatester/err.h>
#include <yatester/builtins.h>

#include <stdio.h>
#include <setjmp.h>

static jmp_buf env;
static yatester_status statusq[2];

void yatester_builtin_expect(size_t argc, const char** argv)
{
	int status;
	yatester_assert(YATESTER_ERROR, sscanf(argv[0], "%d", &status) == 1);
	statusq[1] = status;
}

static yatester_status evalstatus_internal(yatester_status status)
{
	if (status == statusq[0])
	{
		status = YATESTER_OK;
	}
	else if (statusq[0] != YATESTER_OK)
	{
		status = yatester_report(YATESTER_ERROR, "expected and real status codes differ\n");
	}

	statusq[0] = statusq[1];
	statusq[1] = YATESTER_OK;

	return status;
}


yatester_status yatester_call(const char* commandname, size_t argc, const char** argv)
{
	const yatester_command* command;
	yatester_status status;

	command = yatester_getcommand(commandname);

	if (command == NULL)
	{
		return yatester_report(YATESTER_BADCALL, "command \"%s\" not found\n", commandname);
	}

	if (argc < command->minargc)
	{
		return yatester_report(YATESTER_BADCALL, "command \"%s\" expected %zu argument(s), but got %zu\n", commandname, command->minargc, argc);
	}

	status = setjmp(env);

	if (status == YATESTER_OK)
	{
		command->handler(argc, (const char**) argv);
	}

	return evalstatus_internal(status);
}

void yatester_raise(yatester_status status)
{
	longjmp(env, status);
}

void yatester_assert_function(const char* code, const char* file, int line, yatester_status status, int condition)
{
	if (!condition)
	{
		yatester_raise(yatester_report(status, "failed assertion \"%s\" in \"%s\", line %d\n", code, file, line));
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
		[YATESTER_OK] = "Ok",
		[YATESTER_ERROR] = "Error",
		[YATESTER_NOMEM] = "No memory",
		[YATESTER_FTLERR] = "Fatal error",
		[YATESTER_MEMLK] = "Memory leak",
		[YATESTER_IOERR] = "I/O error",
		[YATESTER_STXERR] = "Syntax error",
		[YATESTER_BADCMD] = "Bad command",
		[YATESTER_BADCALL] = "Bad call",
	};

	if (status >= 0 && status < sizeof(preffixes)/sizeof(*preffixes))
	{
		fprintf(stderr, "%s: ", preffixes[status]);
		vfprintf(stderr, fmt, va);
		return status;
	}
	else
	{
		return yatester_report(YATESTER_ERROR, "bad status code %d\n", status);
	}
}

