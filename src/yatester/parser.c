#include <yatester/parser.h>
#include <yatester/cmdhdl.h>
#include <yatester/runner.h>
#include <yatester/yatester.h>

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

static char *cmdbuf, *argbuf;
static char **argvbuf;
static size_t cmdbufsize, argbufsize, argvbufsize, argc, xargc;
static yatester_status expected_status, next_expected_status;
static size_t cmdlen, arglen;
static size_t line, col;
static size_t tkline, tkcol;
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

static void resizebuffer_internal(void** buffer_ptr, size_t item_size, size_t* size_ptr)
{
	void *newbuffer;
	size_t size = *size_ptr;
	size_t newsize = size * 2;
	
	if (newsize * item_size <= size * item_size)
	{
		fprintf(stderr, "Buffer overflow\n");
		longjmp(env, YATESTER_MEM);
	}

	newbuffer = realloc(*buffer_ptr, newsize * item_size);

	if (newbuffer == NULL)
	{
		fprintf(stderr, "Could not resize buffer\n");
		longjmp(env, YATESTER_MEM);
	}

	*buffer_ptr = newbuffer;
	*size_ptr = newsize;
}

static void writecommand_internal(char c)
{
	if (cmdlen == cmdbufsize - 1)
	{
		resizebuffer_internal((void**) &cmdbuf, sizeof *cmdbuf, &cmdbufsize);
	}

	cmdbuf[cmdlen++] = c;
	tkline = line;
	tkcol = col;
}

static void writeargument_internal(char c)
{
	if (arglen == argbufsize - 1)
	{
		char* prevargbuf = argbuf;
		
		resizebuffer_internal((void**) &argbuf, sizeof *argbuf, &argbufsize);
		
		for (size_t i = 0; i < argvbufsize; ++i)
		{
			argvbuf[i] = (argvbuf[i] - prevargbuf) + argbuf;
		}
	}

	argbuf[arglen++] = c;
	tkline = line;
	tkcol = col;
}

static void pushargument_internal()
{
	if (argc == argvbufsize)
	{
		resizebuffer_internal((void**) &argvbuf, sizeof *argvbuf, &argvbufsize);
	}

	argbuf[arglen++] = '\0';
	argvbuf[++argc] = argbuf + arglen;

#ifdef YATESTER_PARSER_DEBUG
	fprintf(stderr, "[PARSER] Pushed argument \"%s\"\n", argvbuf[argc-1]);
#endif
}

static void runcommand_internal()
{
	yatester_status status;

#ifdef YATESTER_PARSER_DEBUG
	fprintf(stderr, "[PARSER] Calling /%s", cmdbuf);

	for (size_t i = 0; i < argc; ++i)
	{
		fprintf(stderr, " \"%s\"", argvbuf[i]);
	}

	fprintf(stderr, "\n");
#endif

	status = yatester_runcommand(cmdbuf, argc, (const char**) argvbuf);

	argc = 0;
	xargc = 0;
	cmdlen = 0;
	arglen = 0;

	if (status == YATESTER_OK)
	{
		tkline = 0;
		tkcol = 0;
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

#ifdef YATESTER_PARSER_DEBUG
	fprintf(stderr, "[PARSER] Pushed command \"%s\"\n", cmdbuf);
#endif
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
			if (xargc == 0)
			{
				runcommand_internal();
				return ST_INITIAL;
			}
			else
			{
				return ST_SEPARATOR;
			}
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

yatester_status yatester_initializeparser()
{
	cmdbufsize = 64;
	cmdbuf = malloc(sizeof(*cmdbuf) * cmdbufsize);
	if (cmdbuf == NULL)
	{
		return YATESTER_MEM;
	}

	argbufsize = 64;
	argbuf = malloc(sizeof(*argbuf) * argbufsize);
	if (argbuf == NULL)
	{
		return YATESTER_MEM;
	}

	argvbufsize = 4;
	argvbuf = malloc(sizeof(*argvbuf) * argvbufsize);
	if (argvbuf == NULL)
	{
		return YATESTER_MEM;
	}

	argvbuf[0] = argbuf;

	return YATESTER_OK;
}

yatester_status yatester_parsescript(FILE *fp)
{
	int c;
	state st = ST_INITIAL;
	yatester_status status;

	col = 0;
	line = 1;
	status = setjmp(env);

	if (status == YATESTER_OK)
	{
		do
		{
			c = getc(fp);

			if (c == '\n')
			{
				++line;
				col = 1;
			}
			else
			{
				++col;
			}

			st = transition_internal(st, c);

#ifdef YATESTER_PARSER_DEBUG
			if (c != EOF)
				fprintf(stderr, "[PARSER] Char: '%c'\tState: %d\n", (char) c, st);
			else
				fprintf(stderr, "[PARSER] Char: EOF\tState: %d\n", st);
#endif
		}
		while (!IS_FINAL(st));
	}
	else
	{
		/* If no token was read yet */
		if (tkline == 0 && tkcol == 0)
		{
			tkline = line;
			tkcol = col;
		}

		fprintf(stderr, "Error %d raised in line %zu, column %zu\n", status, tkline, tkcol);
	}

	return status;
}

void yatester_terminateparser()
{
	if (cmdbuf != NULL)
	{
		free(cmdbuf);
		cmdbuf = NULL;
	}

	if (argbuf != NULL)
	{
		free(argbuf);
		argbuf = NULL;
	}

	if (argvbuf != NULL)
	{
		free(argvbuf);
		argvbuf = NULL;
	}
}
