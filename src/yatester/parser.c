#include <yatester/parser.h>
#include <yatester/runner.h>
#include <yatester/yatester.h>

#include <setjmp.h>
#include <stdarg.h>

/**
 * @brief Parser state
 */
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
#define IS_SEPARATOR(c) ((c) == ' ' || (c) == '\t')
#define IS_NEWLINE(c) ((c) == '\n')
#define IS_ALPHA(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))

static char *cmdbuf, *argbuf;
static char **argvbuf;
static size_t cmdbufsize, argbufsize, argvbufsize, argc, cmdlen, arglen;
static size_t line, col, tkline, tkcol;
static jmp_buf env;

/**
 * @brief Throws an error
 * @note Performs a long jump
 * @param status error status code
 * @param fmt error message format
 * @param ... format arguments
 * @return ST_EOF
 */
static state error_internal(yatester_status status, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	va_end(va);
	longjmp(env, status);
	return ST_EOF; /* never reaches here */
}

/**
 * @brief Resize buffer, updating pointer to buffer and pointer to size
 * @note New buffer might be moved to a new location
 * @note On error, performs a long jump
 * @param buffer_ptr buffer address
 * @param item_size item size
 * @param size_ptr buffer size address
 */
static void resizebuffer_internal(void** buffer_ptr, size_t item_size, size_t* size_ptr)
{
	void *newbuffer;
	size_t size = *size_ptr;
	size_t newsize = size * 2;
	
	/* Check if new size fits in a size_t */
	if (newsize * item_size <= size * item_size)
	{
		error_internal(YATESTER_NOMEM, "Reached maximum buffer size of %zu\n", size);
	}

	/* Resize buffer to fit newsize items */
	newbuffer = realloc(*buffer_ptr, newsize * item_size);

	if (newbuffer == NULL)
	{
		error_internal(YATESTER_NOMEM, "Could not reallocate buffer\n");
	}

	/* Update buffer and size pointers */
	*buffer_ptr = newbuffer;
	*size_ptr = newsize;
}

/**
 * @brief Write character to command buffer
 * @note On error, performs a long jump
 * @param c character to be written
 */
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

/**
 * @brief Write character to argument buffer
 * @note If argument buffer is resized, updates argvbuf too
 * since it might have been moved when reallocated
 * @note On error, performs a long jump
 * @param c character to be written
 */
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

/**
 * @brief Push argument to argument vector buffer
 * @note On error, performs a long jump
 */
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

/**
 * @brief Push command
 */
static void pushcommand_internal()
{
	cmdbuf[cmdlen] = '\0';

#ifdef YATESTER_PARSER_DEBUG
	fprintf(stderr, "[PARSER] Pushed command \"%s\"\n", cmdbuf);
#endif
}

/**
 * @brief Run command with pushed command and arguments
 * @note If command fails, performs a long jump
 */
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

/**
 * @brief State machine transition function
 * @note On error, performs a long jump
 * @param st current state
 * @param c last character read
 * @return new state
 */
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
		else if (IS_SEPARATOR(c) || IS_NEWLINE(c))
		{
			return ST_INITIAL;
		}
		else if (c == EOF)
		{
			return ST_EOF;
		}
		else
		{
			return error_internal(YATESTER_STXERR, "Expected '#', '/' or EOF\n");
		}
		break;
	case ST_COMMENT:
		if (IS_NEWLINE(c))
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
			return error_internal(YATESTER_STXERR, "Expected an alphabetic character\n");
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
		else if (IS_NEWLINE(c))
		{
			pushcommand_internal();
			runcommand_internal();
			return ST_INITIAL;
		}
		else if (IS_SEPARATOR(c))
		{
			pushcommand_internal();
			return ST_SEPARATOR;
		}
		else
		{
			return error_internal(YATESTER_STXERR, "Expected an alphabetic character, '-' or EOF\n");
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
		else if (IS_NEWLINE(c))
		{
			runcommand_internal();
			return ST_INITIAL;
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
			return ST_SEPARATOR;
		}
		else if (IS_NEWLINE(c))
		{
			pushargument_internal();
			runcommand_internal();
			return ST_INITIAL;
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
			return error_internal(YATESTER_STXERR, "Unexpected EOF\n");
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
			return ST_SEPARATOR;
		}
		else if (IS_NEWLINE(c))
		{
			pushargument_internal();
			runcommand_internal();
			return ST_INITIAL;
		}
		else if (c == EOF)
		{
			pushargument_internal();
			runcommand_internal();
			return ST_EOF;
		}
		else
		{
			return error_internal(YATESTER_STXERR, "Expected separator or EOF\n");
		}
		break;
	case ST_EOF:
		return ST_EOF;
	default:
		return error_internal(YATESTER_FTLERR, "Invalid state %d\n", st);
	}
}

yatester_status yatester_initializeparser()
{
	cmdbufsize = 64;
	cmdbuf = malloc(sizeof(*cmdbuf) * cmdbufsize);
	if (cmdbuf == NULL)
	{
		return YATESTER_NOMEM;
	}

	argbufsize = 64;
	argbuf = malloc(sizeof(*argbuf) * argbufsize);
	if (argbuf == NULL)
	{
		return YATESTER_NOMEM;
	}

	argvbufsize = 4;
	argvbuf = malloc(sizeof(*argvbuf) * argvbufsize);
	if (argvbuf == NULL)
	{
		return YATESTER_NOMEM;
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

			if (IS_NEWLINE(c))
			{
				++line;
				col = 0;
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
