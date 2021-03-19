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

/**
 * @brief String
 */
typedef struct
{
	char* ptr; /**< Buffer */
	size_t size; /**< Buffer size */
	size_t length; /**< String length */
}
string_t;

#define IS_FINAL(st) ((st) == ST_EOF)
#define IS_SEPARATOR(c) ((c) == ' ' || (c) == '\t')
#define IS_NEWLINE(c) ((c) == '\n')
#define IS_DIGIT(c) ((c) >= '0' && (c) <= '9')
#define IS_ALPHA(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))
#define MAX_ARGC 64

static string_t cmdstr, argstr; /* Command and argument strings */
static size_t argidx[MAX_ARGC]; /* Argument indices */
static int argc; /* Argument count */
static size_t line, col, tkline, tkcol; /* Line and column counters */
static jmp_buf env; /* Long jump buffer */

/**
 * @brief Throws an error
 * @note Performs a long jump
 * @param status error status code
 * @param fmt error message format
 * @param ... format arguments
 * @return ST_EOF
 */
static state raise_internal(yatester_status status, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	yatester_vreport(status, fmt, va);
	va_end(va);
	longjmp(env, status);
	return ST_EOF; /* never reaches here */
}

/**
 * @brief Initialize string with size
 * @param string
 * @return failure
 */
static int string_initialize(string_t *string)
{
	string->size = 1;
	string->length = 0;
	string->ptr = malloc(1);
	return string->ptr == NULL;
}

/**
 * @brief Check if string will overflow, and allocate more space if necessary
 * @note Buffer might be moved
 * @note On error, performs a long jump
 * @param string
 */
static void string_check(string_t *string)
{
	char *newptr;
	size_t size = string->size;
	size_t newsize = size | (size << 1);
	
	/* Check if string will overflow */
	if (string->length < size - 1)
	{
		return;
	}

	/* Check if new size fits in a size_t */
	if (newsize <= size)
	{
		raise_internal(YATESTER_NOMEM, "Reached maximum string size of %zu\n", size);
	}

	/* Resize string to fit newsize items */
	newptr = realloc(string->ptr, newsize);

	/* Check if realloc failed */
	if (newptr == NULL)
	{
		raise_internal(YATESTER_NOMEM, "Could not resize string\n");
	}

	/* Update string */
	string->ptr = newptr;
	string->size = newsize;
}

/**
 * @brief Free string
 * @param string
 */
static void string_terminate(string_t *string)
{
	if (string->ptr != NULL)
	{
		free(string->ptr);
		string->length = 0;
		string->size = 0;
		string->ptr = NULL;
	}
}

/**
 * @brief Write character to command string
 * @note On error, performs a long jump
 * @param c character to be written
 */
static void writecommand_internal(char c)
{
	string_check(&cmdstr);
	cmdstr.ptr[cmdstr.length++] = c;
	tkline = line;
	tkcol = col;
}

/**
 * @brief Write character to argument string
 * @note On error, performs a long jump
 * @param c character to be written
 */
static void writeargument_internal(char c)
{
	string_check(&argstr);
	argstr.ptr[argstr.length++] = c;
	tkline = line;
	tkcol = col;
}

/**
 * @brief Push argument
 * @note On error, performs a long jump
 */
static void pushargument_internal()
{
	if (argc == MAX_ARGC)
	{
		raise_internal(YATESTER_BADCALL, "too many arguments\n");
	}

	argstr.ptr[argstr.length++] = '\0';

	if (++argc < MAX_ARGC)
	{
		argidx[argc] = argstr.length;
	}

#ifdef YATESTER_PARSER_DEBUG
	fprintf(stderr, "[PARSER] Pushed argument \"%s\"\n", &argstr.ptr[argstr.length]);
#endif
}

/**
 * @brief Push command
 */
static void pushcommand_internal()
{
	cmdstr.ptr[cmdstr.length] = '\0';

#ifdef YATESTER_PARSER_DEBUG
	fprintf(stderr, "[PARSER] Pushed command \"%s\"\n", cmdstr.ptr);
#endif
}

/**
 * @brief Call command with pushed arguments
 * @note If call fails, performs a long jump
 */
static void call_internal()
{
	yatester_status status;
	static char *argv[MAX_ARGC];

	for (int i = 0; i < argc; ++i)
	{
		argv[i] = argstr.ptr + argidx[i];
	}

#ifdef YATESTER_PARSER_DEBUG
	fprintf(stderr, "[PARSER] Calling /%s", cmdstr.ptr);

	for (int i = 0; i < argc; ++i)
	{
		fprintf(stderr, " \"%s\"", argv[i]);
	}

	fprintf(stderr, "\n");
#endif

	status = yatester_call(cmdstr.ptr, argc, argv);

	argc = 0;
	cmdstr.length = 0;
	argstr.length = 0;
	
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
			return raise_internal(YATESTER_STXERR, "Expected '#', '/' or EOF\n");
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
			return raise_internal(YATESTER_STXERR, "Expected an alphabetic character\n");
		}
		break;
	case ST_COMMAND:
		if (IS_DIGIT(c) || IS_ALPHA(c) || c == '-')
		{
			writecommand_internal(c);
			return ST_COMMAND;
		}
		else if (c == EOF)
		{
			pushcommand_internal();
			call_internal();
			return ST_EOF;
		}
		else if (IS_NEWLINE(c))
		{
			pushcommand_internal();
			call_internal();
			return ST_INITIAL;
		}
		else if (IS_SEPARATOR(c))
		{
			pushcommand_internal();
			return ST_SEPARATOR;
		}
		else
		{
			return raise_internal(YATESTER_STXERR, "Expected an alphanumeric character, '-' or EOF\n");
		}
		break;
	case ST_SEPARATOR:
		if (c == '#')
		{
			call_internal();
			return ST_COMMENT;
		}
		else if (c == '/')
		{
			call_internal();
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
			call_internal();
			return ST_INITIAL;
		}
		else if (c == EOF)
		{
			call_internal();
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
			call_internal();
			return ST_INITIAL;
		}
		else if (c == EOF)
		{
			pushargument_internal();
			call_internal();
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
			return raise_internal(YATESTER_STXERR, "Unexpected EOF\n");
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
			call_internal();
			return ST_INITIAL;
		}
		else if (c == EOF)
		{
			pushargument_internal();
			call_internal();
			return ST_EOF;
		}
		else
		{
			return raise_internal(YATESTER_STXERR, "Expected separator or EOF\n");
		}
		break;
	case ST_EOF:
		return ST_EOF;
	default:
		return raise_internal(YATESTER_FTLERR, "Invalid state %d\n", st);
	}
}

yatester_status yatester_initializeparser()
{
	if (string_initialize(&cmdstr))
	{
		return YATESTER_NOMEM;
	}

	if (string_initialize(&argstr))
	{
		return YATESTER_NOMEM;
	}

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

		status = yatester_report(status, "error raised in line %zu, column %zu\n", tkline, tkcol);
	}

	return status;
}

void yatester_terminateparser()
{
	string_terminate(&cmdstr);
	string_terminate(&argstr);
}
