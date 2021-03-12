#include <yatester/parser.h>
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
	ST_OK,
	ST_ERROR,
}
state;

#define IS_FINAL(st) ((st) == ST_OK || (st) == ST_ERROR)
#define IS_SEPARATOR(c) ((c) == ' ' || (c) == '\t' || (c) == '\n')
#define IS_ALPHA(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))

static char cmdbuf[MAXCMDLEN], argbuf[MAXARGCNT][MAXARGLEN];
static size_t cmdlen, arglen;
static int argc;
static jmp_buf env;

static void throw_internal(yatester_status status)
{
	longjmp(env, status);
}

static yatester_status catch_internal()
{
	return setjmp(env);
}

static state error_internal(const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	va_end(va);
	throw_internal(YATESTER_OK);
	return ST_ERROR; /* never reaches here */
}

static void writecommand_internal(char c)
{
	if (cmdlen == MAXCMDLEN - 1)
	{
		error_internal("Command buffer overflow (too long)\n");
	}

	cmdbuf[cmdlen++] = c;
}

static void writeargument_internal(char c)
{
	if (arglen == MAXARGLEN - 1)
	{
		error_internal("Argument buffer overflow (too long)\n");
	}

	argbuf[argc][arglen++] = c;
}

static void pushargument_internal()
{
	if (argc == MAXARGCNT)
	{
		error_internal("Argument buffer overflow (too many)\n");
	}

	argbuf[argc][arglen] = '\0';

	argc++;
	arglen = 0;
}

static void pushcommand_internal()
{
	yatester_status status;

	cmdbuf[cmdlen] = '\0';

	status = yatester_runcommand((const char*) cmdbuf, argc, (const char**) argbuf);

	if (status == YATESTER_OK)
	{
		argc = 0;
		cmdlen = 0;
		arglen = 0;
	}
	else
	{
		throw_internal(status);
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
			return ST_OK;
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
			return ST_OK;
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
			return ST_SLASH;
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
			return ST_OK;
		}
		else if (IS_SEPARATOR(c))
		{
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
			pushcommand_internal();
			return ST_COMMENT;
		}
		else if (c == '/')
		{
			pushcommand_internal();
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
			pushcommand_internal();
			return ST_OK;
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
			return ST_SEPARATOR;
		}
		else if (c == EOF)
		{
			pushargument_internal();
			pushcommand_internal();
			return ST_OK;
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
			pushargument_internal();
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
			return ST_SEPARATOR;
		}
		else if (c == EOF)
		{
			pushcommand_internal();
			return ST_OK;
		}
		else
		{
			return error_internal("Expected separator or EOF\n");
		}
		break;
	case ST_OK:
		return ST_OK;
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

	status = catch_internal();

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

		if (st == ST_ERROR)
		{
			fprintf(stderr, "Parsing error in line %zu, column %zu\n", line, col);
		}
	}
	else
	{
		fprintf(stderr, "Command error in line %zu, column %zu\n", line, col);
	}

	return status;
}
