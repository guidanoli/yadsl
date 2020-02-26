#include "tester.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#pragma once
#if defined(_MSC_VER)
# pragma warning(disable : 4996)
#endif

////////////////////////////////////////////////////////////////////////////////
// STATIC VARIABLES DECLARATIONS
////////////////////////////////////////////////////////////////////////////////

static const char *float_f[] =  { "%f", "%g", "%e", NULL };
static const char *int_f[] =    { "%d", "%u", NULL };
static const char *long_f[] =   { "%l", "%lu", NULL };
static const char *size_t_f[] = { "%zu", NULL };

static size_t line; // line count
static const char *externalReturnValueInfo = NULL; // external return value
static const char *nativeReturnValueInfos[TESTER_RETURN_COUNT]; // return value
static char buffer[BUFSIZ] = "", // file line
command[BUFSIZ] = "", // command string
sep[BUFSIZ] = "", // separation characters
temp[BUFSIZ] = ""; // temp. variable
static char *cursor = buffer; // buffer cursor

////////////////////////////////////////////////////////////////////////////////
// STATIC FUNCTIONS DECLARATIONS
////////////////////////////////////////////////////////////////////////////////

static TesterReturnValue _TesterMain(int argc, char **argv);
static void _TesterLoadReturnValueInfos();
static TesterReturnValue _TesterParse(FILE *fp);
static void _TesterPrintCursorPosition(FILE *fp, size_t spacing);
static TesterReturnValue _TesterParseCatchCommand(TesterReturnValue ret);
static int _TesterParseArg(const char *format, void *arg, size_t *inc);
static int _TesterParseStr(char *arg, size_t *inc);
static void _TesterPrintReturnValueInfo(TesterReturnValue ret);
static int _TesterParseArgFormat(const char **format_t, void *arg, size_t *inc);

////////////////////////////////////////////////////////////////////////////////
// EXTERN FUNCTIONS DEFINITIONS
////////////////////////////////////////////////////////////////////////////////

int hasflag(int argc, char **argv, const char *flag)
{
	while (argc--)
		if (strcmp(argv[argc], flag) == 0)
			return 1;
	return 0;
}

/**
* Usage: <program> [script-path [/LOG]]
* If script-path is not provided,
* then help strings are displayed.
*/
int main(int argc, char **argv)
{
	TesterReturnValue exitReturn, ret;
	FILE *logger = NULL;
	if (hasflag(argc, argv, "/LOG")) {
		logger = fopen("memdb.log", "w");
		_memdb_set_logger(logger);
	}
	ret = _TesterMain(argc, argv);
	exitReturn = TesterExitCallback();
	if (ret == TESTER_RETURN_OK)
		ret = exitReturn;
	if (ret == TESTER_RETURN_OK &&
		(_memdb_list_size() || _memdb_error_occurred()))
		ret = TESTER_RETURN_MEMLEAK;
	_TesterPrintReturnValueInfo(ret);
	_memdb_clear_list();
	_memdb_set_logger(NULL);
	if (logger) fclose(logger);
	return ret;
}

int TesterParseArguments(const char *format, ...)
{
	va_list va;
	size_t inc = 0;
	void *arg;
	char *str;
	int argc = 0;
	if (format == NULL) return -1;
	va_start(va, format);
	for (; *format != '\0'; ++format, ++argc) {
		int parsingError = 0;
		switch (*format) {
		case 'f':
			if (!(arg = va_arg(va, float *))) {
				parsingError = 1;
				break;
			}
			parsingError = _TesterParseArgFormat(float_f, arg, &inc);
			break;
		case 'i':
			if (!(arg = va_arg(va, int *))) {
				parsingError = 1;
				break;
			}
			parsingError = _TesterParseArgFormat(int_f, arg, &inc);
			break;
		case 'l':
			if (!(arg = va_arg(va, long *))) {
				parsingError = 1;
				break;
			}
			parsingError = _TesterParseArgFormat(long_f, arg, &inc);
			break;
		case 's':
			if (!(str = va_arg(va, char *))) {
				parsingError = 1;
				break;
			}
			parsingError = _TesterParseStr(str, &inc);
			break;
		case 'z':
			if (!(arg = va_arg(va, size_t *))) {
				parsingError = 1;
				break;
			}
			parsingError = _TesterParseArgFormat(size_t_f, arg, &inc);
			break;
		default:
			fprintf(stderr, "Format character %c unknown.\n", *format);
			parsingError = 1;
			break;
		}
		if (parsingError) {
			va_end(va);
			return -1;
		}
	}
	va_end(va);
	cursor += inc;
	return argc;
}

TesterReturnValue TesterExternalReturnValue(const char *info)
{
	externalReturnValueInfo = info;
	return TESTER_RETURN_EXTERNAL;
}

void TesterPrintHelpStrings()
{
	const char **str = TesterHelpStrings;
	for (; str && *str; ++str) puts(*str);
}

const char *TesterGetReturnValueInfo(TesterReturnValue returnValue)
{
	if (returnValue >= TESTER_RETURN_COUNT)
		return "Invalid return value";
	if (returnValue == TESTER_RETURN_EXTERNAL)
		if (externalReturnValueInfo == NULL)
			return "Missing external return value information";
		else
			return externalReturnValueInfo;
	return nativeReturnValueInfos[returnValue];
}

void TesterLog(const char *message, ...)
{
	va_list va;
	size_t spacing = 0, col = cursor - buffer;
	va_start(va, message);
	spacing += fprintf(stdout, "LOG: \"");
	spacing += vfprintf(stdout, message, va);
	spacing += fprintf(stdout, "\" (line %zu, col %zu)\n", line, col);
	va_end(va);
}

////////////////////////////////////////////////////////////////////////////////
// STATIC FUNCTIONS DEFINITIONS
////////////////////////////////////////////////////////////////////////////////

static TesterReturnValue _TesterMain(int argc, char **argv)
{
	FILE *fp;
	TesterReturnValue ret;
	// First, load return value informations
	_TesterLoadReturnValueInfos();
	// If no arguments were passed, then print help strings
	if (argc == 1) {
		TesterPrintHelpStrings();
		return TESTER_RETURN_OK;
	}
	// Open file whose path was passed as argument
	fp = fopen(argv[1], "r");
	if (fp == NULL)
		return TESTER_RETURN_FILE;
	// Initialize tester
	if (ret = TesterInitCallback())
		return ret;
	// Parse script
	return _TesterParse(fp);
}

static TesterReturnValue _TesterParse(FILE *fp)
{
	TesterReturnValue ret = TESTER_RETURN_OK;
	line = 1;
	// Read a line from the file and store it in a buffer
	while (fgets(buffer, sizeof(buffer), fp)) {
		size_t bufflen = strlen(buffer);
		// Make sure it doesn't overflow
		if (bufflen == BUFSIZ - 1)
			return TESTER_RETURN_OVERFLOW;
		// Iterate through the buffer with a cursor
		for (cursor = buffer; cursor < buffer + bufflen; ++cursor) {
			if (*cursor == '\t' || *cursor == ' ' || *cursor == '\n')
				continue; /* Ignore spacings */
			if (*cursor == '#')
				break; /* Ignore comments */
			if (*cursor == '/') {
				// Detect and parse commands
				if (sscanf(cursor, "/%[^ \t\n]", command) == 1) {
					// Move the cursor to after the command
					cursor += strlen(command) + 1;
					if (strcmp(command, "catch") == 0) {
						// Check if an error already occurred
						if (ret = _TesterParseCatchCommand(ret))
							return ret;
					} else {
						if (ret) {
							fprintf(stderr,
								"ERROR: Expected /catch command\n");
							return ret;
						}
						// Call the command parser (can move cursor)
#ifdef _VERBOSE
						printf("Parsing '%s' at line %zu\n", command, line);
#endif
						externalReturnValueInfo = NULL;
						ret = TesterParseCallback(command);
					}
				} else {
					// Command does not match regex
					return TESTER_RETURN_COMMAND;
				}
			} else {
				// Unexpected character
				return TESTER_RETURN_ARGUMENT;
			}
		}
		// Increase the line counter
		++line;
	}
	return ret;
}
static void _TesterLoadReturnValueInfos()
{
	struct returnValue
	{
		TesterReturnValue value;
		const char *info;
	};
	size_t i;
	struct returnValue nativeValues[] = {
		{TESTER_RETURN_OK, "ok"},
		{TESTER_RETURN_FILE, "file"},
		{TESTER_RETURN_MALLOC, "malloc"},
		{TESTER_RETURN_MEMLEAK, "memleak"},
		{TESTER_RETURN_OVERFLOW, "overflow"},
		{TESTER_RETURN_COMMAND, "command"},
		{TESTER_RETURN_ARGUMENT, "argument"},
		{TESTER_RETURN_RETURN, "return"},
		{TESTER_RETURN_EXTERNAL, "external"},
	};
	for (i = 0; i < sizeof(nativeValues) / sizeof(nativeValues[0]); ++i) {
		struct returnValue retVal = nativeValues[i];
		nativeReturnValueInfos[retVal.value] = retVal.info;
	}
}

static int _TesterParseArg(const char *format, void *arg, size_t *inc)
{
	if (arg == NULL) return -1;
	if (sscanf(cursor + *inc, "%[ \t\n]%[^ \t\n]", sep, temp) != 2)
		return -1;
	if (sscanf(temp, format, arg) != 1)
		return -1;
	*inc += strlen(sep) + strlen(temp);
	return 0;
}

static int _TesterParseStr(char *arg, size_t *inc)
{
	if (arg == NULL) return -1;
	if (sscanf(cursor + *inc, "%[ \t\n]\"%[^\"]\"", sep, temp) == 2) {
		*inc += 2; /* First checks if there are quotation marks */
	} else if (sscanf(cursor + *inc, "%[ \t\n]%[^ \t\n]", sep, temp) != 2) {
		return -1; /* Then attempts to parse without them */
	}
	strcpy(arg, temp);
	if (arg == NULL)
		return -1;
	*inc += strlen(sep) + strlen(temp);
	return 0;
}

static TesterReturnValue _TesterParseCatchCommand(TesterReturnValue ret)
{
	char arg[BUFSIZ] = "";
	if (TesterParseArguments("s", arg) == 1) {
		if (ret == TESTER_RETURN_EXTERNAL) {
			if (externalReturnValueInfo != NULL &&
				strcmp(externalReturnValueInfo, arg) == 0)
				return TESTER_RETURN_OK;
		} else {
			if (strcmp(nativeReturnValueInfos[ret], arg) == 0)
				return TESTER_RETURN_OK;
		}
	}
	fprintf(stderr, "ERROR: Could not catch \"%s\"\n",
	TesterGetReturnValueInfo(ret));
	return TESTER_RETURN_ARGUMENT;
}

static void _TesterPrintReturnValueInfo(TesterReturnValue ret)
{
	if (ret) {
		size_t spacing = fprintf(stderr, "ERROR: \"%s\" ",
			TesterGetReturnValueInfo(ret));
		_TesterPrintCursorPosition(stderr, spacing);
	}
}

// Prints the buffer and the cursor current position with an arrow (^)
// spacing = how many characters have been printed out on the current line
// HINT: printf-like functions return the number of characters printed out
static void _TesterPrintCursorPosition(FILE *fp, size_t spacing)
{
	size_t col = cursor - buffer, bufflen = strlen(buffer);
	if (bufflen == 0) return;
	spacing += fprintf(fp, "(lin %zu, col %zu) ", line, col + 1);
	spacing %= 80; /* Loop command length */
	if (spacing + bufflen > 80) {
		// trim buffer to fit 80 characters
		fprintf(fp, "\n%.*s", 80, buffer);
		spacing = col;
		if (buffer[79] != '\n') fprintf(fp, "\n");
	} else {
		fprintf(fp, "%s", buffer);
		spacing += col;
		if (buffer[bufflen - 1] != '\n') fprintf(fp, "\n");
	}
	fprintf(fp, "\n");
	while (spacing--) fprintf(fp, " ");
	fprintf(fp, "^\n");
}

static int _TesterParseArgFormat(const char **format_t, void *arg, size_t *inc)
{
	const char *format;
	while (format = *(format_t++)) {
		if (!_TesterParseArg(format, arg, inc))
			return 0;
	}
	return 1;
}