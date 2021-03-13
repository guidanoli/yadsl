#include <yatester/parser.h>
#include <yatester/cmdhdl.h>
#include <yatester/runner.h>

#include <setjmp.h>
#include <stdarg.h>

typedef enum
{
	ST_INITIAL,
	ST_COMMENT,
	ST_SLASH,
	ST_COMMAND,
	ST_SEPARATOR,
	ST_ARGUMENT,
	ST_QUOTED_ARGUMENT,
	ST_QUOTED_ARGUMENT_END,
	ST_EOF,
}
state;

#define IS_FINAL(st) ((st) == ST_EOF)
#define IS_SEPARATOR(c) ((c) == ' ' || (c) == '\t' || (c) == '\n')
#define IS_ALPHA(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))

static char cmdbuf[MAXCMDLEN], argbuf[MAXARGLEN];
static char* argvbuf[MAXARGCNT] = { argbuf };
static size_t cmdlen, arglen;
static int argc, xargc;
static jmp_buf env;

static state error_internal(const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	va_end(va);
	longjmp(env, YATESTER_ERR);
	return ST_EOF; /* never reaches here */
}

static void writecommand_internal(char c)
{
	if (cmdlen == MAXCMDLEN - 1)
	{
		error_internal("Command buffer overflow\n");
	}

	cmdbuf[cmdlen++] = c;
}

static void writeargument_internal(char c)
{
	if (arglen == MAXARGLEN - 1)
	{
		error_internal("Argument buffer overflow\n");
	}

	argbuf[arglen++] = c;
}

static void pushargument_internal()
{
	if (argc == MAXARGCNT)
	{
		error_internal("Argument vector buffer overflow\n");
	}

	writeargument_internal('\0');
	argvbuf[++argc] = argbuf;
}

static void runcommand_internal()
{
	yatester_status status;

	status = yatester_runcommand(cmdbuf, argc, (const char**) argvbuf);

	if (status == YATESTER_OK)
	{
		argc = 0;
		xargc = 0;
		cmdlen = 0;
		arglen = 0;
	}
	else
	{
		longjmp(env, status);
	}
}

static void pushcommand_internal()
{
	const yatester_command* command;

	cmdbuf[cmdlen] = '\0';

	command = yatester_getcommand(cmdbuf);

	if (command == NULL)
	{
		error_internal("Could not find command named \"%s\"\n", cmdbuf);
	}
	else
	{
		xargc = command->argc;
	}
}

static state transition_internal(state st, int c)
{
	switch (st)
	{
	case ST_INITIAL:
		if (c == '#')
		{
			return ST_COMMENT;
		}
		else if (c == '/')
		{
			return ST_SLASH;
		}
		else if (IS_SEPARATOR(c))
		{
			return ST_INITIAL;
		}
		else if (c == EOF)
		{
			return ST_EOF;
		}
		else
		{
			return error_internal("Expected '#', '/' or EOF\n");
		}
		break;
	case ST_COMMENT:
		if (c == '\n')
		{
			return ST_INITIAL;
		}
		else if (c == EOF)
		{
			return ST_EOF;
		}
		else
		{
			return ST_COMMENT;
		}
		break;
	case ST_SLASH:
		if (IS_ALPHA(c))
		{
			writecommand_internal(c);
			return ST_COMMAND;
		}
		else
		{
			return error_internal("Expected an alphabetic character\n");
		}
		break;
	case ST_COMMAND:
		if (IS_ALPHA(c) || c == '-')
		{
			writecommand_internal(c);
			return ST_COMMAND;
		}
		else if (c == EOF)
		{
			pushcommand_internal();
			runcommand_internal();
			return ST_EOF;
		}
		else if (IS_SEPARATOR(c))
		{
			pushcommand_internal();
			return ST_SEPARATOR;
		}
		else
		{
			return error_internal("Expected an alphabetic character, EOF or '-'\n");
		}
		break;
	case ST_SEPARATOR:
		if (c == '#')
		{
			runcommand_internal();
			return ST_COMMENT;
		}
		else if (c == '/')
		{
			runcommand_internal();
			return ST_SLASH;
		}
		else if (IS_SEPARATOR(c))
		{
			return ST_SEPARATOR;
		}
		else if (c == '"')
		{
			return ST_QUOTED_ARGUMENT;
		}
		else if (c == EOF)
		{
			runcommand_internal();
			return ST_EOF;
		}
		else
		{
			writeargument_internal(c);
			return ST_ARGUMENT;
		}
		break;
	case ST_ARGUMENT:
		if (IS_SEPARATOR(c))
		{
			pushargument_internal();
			if (argc < xargc)
			{
				return ST_SEPARATOR;
			}
			else
			{
				runcommand_internal();
				return ST_INITIAL;
			}
		}
		else if (c == EOF)
		{
			pushargument_internal();
			runcommand_internal();
			return ST_EOF;
		}
		else
		{
			writeargument_internal(c);
			return ST_ARGUMENT;
		}
		break;
	case ST_QUOTED_ARGUMENT:
		if (c == '"')
		{
			return ST_QUOTED_ARGUMENT_END;
		}
		else if (c == EOF)
		{
			return error_internal("Unexpected EOF\n");
		}
		else
		{
			writeargument_internal(c);
			return ST_QUOTED_ARGUMENT;
		}
		break;
	case ST_QUOTED_ARGUMENT_END:
		if (IS_SEPARATOR(c))
		{
			pushargument_internal();
			if (argc < xargc)
			{
				return ST_SEPARATOR;
			}
			else
			{
				runcommand_internal();
				return ST_INITIAL;
			}
		}
		else if (c == EOF)
		{
			pushargument_internal();
			runcommand_internal();
			return ST_EOF;
		}
		else
		{
			return error_internal("Expected separator or EOF\n");
		}
		break;
	case ST_EOF:
		return ST_EOF;
	default:
		return error_internal("Invalid state %d\n", st);
	}
}

yatester_status yatester_parsescript(FILE *fp)
{
	int c;
	size_t line = 0, col = 1;
	state st = ST_INITIAL;
	yatester_status status;

	status = setjmp(env);

	if (status == YATESTER_OK)
	{
		while (1)
		{
			c = getc(fp);
			st = transition_internal(st, c);

			if (IS_FINAL(st))
			{
				break;
			}
			else
			{
				if (c == '\n')
				{
					++line;
					col = 1;
				}
				else
				{
					++col;
				}
			}
		}
	}

	if (status != YATESTER_OK)
	{
		fprintf(stderr, "Error in line %zu, column %zu\n", line, col);
	}

	return status;
}
