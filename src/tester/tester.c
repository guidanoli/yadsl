#include <tester/tester.h>

#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <inttypes.h>
#include <setjmp.h>

#include <yadsl/utl.h>
#include <argvp/argvp.h>

#if defined(_MSC_VER)
# pragma warning(disable : 4996)
#endif

/*****************************************************************************/
/*                      STATIC VARIABLES DECLARATIONS                        */
/*****************************************************************************/

typedef struct
{
	char dtype;
	void *data;
}
yadsl_TesterObject;

static const char
*char_f[] = { "%c", NULL },
*float_f[] = { "%f", "%g", "%e", NULL },
*int_f[] = { "%d", "%u", NULL },
*long_f[] = { "%l", "%lu", NULL },
*size_t_f[] = { "%zu", NULL },
*intmax_t_f[] = { "%" SCNdMAX, NULL };

static size_t line; /* line count */
static jmp_buf env; /* environment */
static const char* errmsg; /* custom error message */
static const char* errfile; /* file in which error was thrown */
static int errline; /* line in which error was thrown */
static const char* errnames[YADSL_TESTER_RET_COUNT] =
{
	[YADSL_TESTER_RET_OK] = "ok",
	[YADSL_TESTER_RET_FILE] = "file",
	[YADSL_TESTER_RET_MALLOC] = "malloc",
	[YADSL_TESTER_RET_MEMLEAK] = "memleak",
	[YADSL_TESTER_RET_OVERFLOW] = "overflow",
	[YADSL_TESTER_RET_COMMAND] = "command",
	[YADSL_TESTER_RET_ARGUMENT] = "argument",
	[YADSL_TESTER_RET_TOKEN] = "token",
	[YADSL_TESTER_RET_RETURN] = "return",
	[YADSL_TESTER_RET_CATCH] = "catch",
}; /* error descriptions */

static char
buffer[BUFSIZ], /* current script line */
sepbuf[BUFSIZ], /* separation characters */
tmpbuf[BUFSIZ], /* multi-purpose */
errbuf[BUFSIZ], /* error message */
fullerrmsg[BUFSIZ]; /* full error message */

static char* cursor = buffer; /* buffer cursor */

static yadsl_ArgvParserHandle* argvp; /* argument vector parser */

static int help; /* Print help message */
static FILE* input_fp; /* Input file pointer */
const char* input_file; /* Input file name */
static FILE* log_fp; /* Logger file pointer */

static int argc; /* Argument count */
static char** argv; /* Argument vector */

#define UPDATE_STATUS(old_status, new_status) \
	do { \
		yadsl_TesterRet temp = new_status; \
		if (old_status == 0) old_status = temp; \
	} while (0)

/*****************************************************************************/
/*                      STATIC FUNCTIONS DECLARATIONS                        */
/*****************************************************************************/

static yadsl_TesterRet yadsl_tester_argvp_init_internal();
static yadsl_TesterRet yadsl_tester_argvp_release_internal();
static yadsl_TesterRet yadsl_tester_check_memleak_internal();
static yadsl_TesterRet yadsl_tester_parse_file_internal();
static void yadsl_tester_print_cursor_position_internal(FILE* fp);
static yadsl_TesterRet yadsl_tester_parse_catch_command_internal(yadsl_TesterRet ret);
static int yadsl_tester_parse_argument_internal(const char* format, void* arg, size_t* inc);
static int yadsl_tester_parse_string_internal(char* arg, size_t* inc);
static void yadsl_tester_print_return_value_info_internal(yadsl_TesterRet ret);
static int yadsl_tester_parse_dtype_internal(const char** format_t, void* arg, size_t* inc);
static const char* yadsl_tester_errinfo_internal(yadsl_TesterRet ret);
static void yadsl_tester_print_help_strings_internal();
static void yadsl_tester_seterr_internal(const char* _errfile, int _errline);

/*****************************************************************************/
/*                      EXTERN FUNCTIONS DEFINITIONS                         */
/*****************************************************************************/

int main(int argc_, char** argv_)
{
	yadsl_TesterRet status = YADSL_TESTER_RET_OK, tempstatus;

	/* Store arguments globally */
	argc = argc_;
	argv = argv_;

	/* Initialize argument vector parser (critical) */
	if (status = yadsl_tester_argvp_init_internal())
		return status;

	/* Print help strings */
	if (help) {
		yadsl_tester_print_help_strings_internal();
		return YADSL_TESTER_RET_OK;
	}

	/* Set environment */
	status = setjmp(env);

	/* If not returning from long jump */
	if (!status) {
		/* Initialize tester (may fail) */
		status = yadsl_tester_init();
	}

	/* If initialization failed, skip parsing */
	if (!status) {
		/* Parse script (may fail) */
		status = yadsl_tester_parse_file_internal();
	}

	/* Set environment */
	tempstatus = setjmp(env);

	if (!tempstatus) {
		/* Release tester (may fail) */
		tempstatus = yadsl_tester_release();
	}

	/* If parsing succeeds, take status code from release callback */
	UPDATE_STATUS(status, tempstatus);

	/* Release argument vector parser */
	UPDATE_STATUS(status, yadsl_tester_argvp_release_internal());

	/* Check for memory leak (may fail) */
	UPDATE_STATUS(status, yadsl_tester_check_memleak_internal());

#ifdef YADSL_DEBUG
	/* Clear allocated memory block list */
	yadsl_memdb_clear_amb_list();

	/* Ignore MALLOC error if a fail has occurred */
	if (status == YADSL_TESTER_RET_MALLOC && yadsl_memdb_fail_occurred())
		status = YADSL_TESTER_RET_OK;
#endif

	/* Print return value information */
	yadsl_tester_print_return_value_info_internal(status);

	return status;
}

size_t yadsl_tester_get_dtype_size(char dtype)
{
	switch (dtype) {
	case 'c':
		return sizeof(char);
	case 'i':
		return sizeof(int);
	case 'f':
		return sizeof(float);
	case 'l':
		return sizeof(long);
	case 's':
		return sizeof(char*);
	case 'z':
		return sizeof(size_t);
	case 'I':
		return sizeof(intmax_t);
	default:
		return 0;
	}
}

const char*
yadsl_tester_get_dtype_fmt(
	char dtype)
{
	switch (dtype) {
	case 'c':
		return "%c";
	case 'i':
		return "%d";
	case 'f':
		return "%f";
	case 'l':
		return "%l";
	case 's':
		return "%s";
	case 'z':
		return "%zu";
	case 'I':
		return "%" SCNdMAX;
	default:
		return 0;
	}
}

int yadsl_tester_compare_arguments(char dtype, const void* expected, const void* obtained)
{
	if (dtype == 's')
		return strcmp(expected, obtained);
	else
		return memcmp(expected, obtained, yadsl_tester_get_dtype_size(dtype));
}

void yadsl_tester_copy_argument(char dtype, const void* source, void* destination)
{
	if (dtype == 's')
		strcpy(destination, source);
	else
		memcpy(destination, source, yadsl_tester_get_dtype_size(dtype));
}

int yadsl_tester_parse_arguments(const char* format, ...)
{
	va_list va;
	size_t inc = 0;
	void* arg;
	char* str;
	int argc = 0;
	if (format == NULL) return -1;
	va_start(va, format);
	for (; *format != '\0'; ++format, ++argc) {
		int parsingError = 0;
		switch (*format) {
		case 'c':
			if (!(arg = va_arg(va, char*))) {
				parsingError = 1;
				break;
			}
			parsingError = yadsl_tester_parse_dtype_internal(char_f, arg, &inc);
			break;
		case 'f':
			if (!(arg = va_arg(va, float*))) {
				parsingError = 1;
				break;
			}
			parsingError = yadsl_tester_parse_dtype_internal(float_f, arg, &inc);
			break;
		case 'i':
			if (!(arg = va_arg(va, int*))) {
				parsingError = 1;
				break;
			}
			parsingError = yadsl_tester_parse_dtype_internal(int_f, arg, &inc);
			break;
		case 'l':
			if (!(arg = va_arg(va, long*))) {
				parsingError = 1;
				break;
			}
			parsingError = yadsl_tester_parse_dtype_internal(long_f, arg, &inc);
			break;
		case 's':
			if (!(str = va_arg(va, char*))) {
				parsingError = 1;
				break;
			}
			parsingError = yadsl_tester_parse_string_internal(str, &inc);
			break;
		case 'z':
			if (!(arg = va_arg(va, size_t*))) {
				parsingError = 1;
				break;
			}
			parsingError = yadsl_tester_parse_dtype_internal(size_t_f, arg, &inc);
			break;
		case 'I':
			if (!(arg = va_arg(va, intmax_t*))) {
				parsingError = 1;
				break;
			}
			parsingError = yadsl_tester_parse_dtype_internal(intmax_t_f, arg, &inc);
			break;
		default:
			fprintf(stderr, "Unknown format character \"%c\".\n", *format);
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

void* yadsl_tester_object_create(char dtype, char *data)
{
	yadsl_TesterObject* obj;
	size_t size;
	if (dtype == 's') {
		size = strlen(data) + 1;
	} else {
		size = yadsl_tester_get_dtype_size(dtype);
		if (size == 0)
			return NULL;
	}
	obj = malloc(sizeof * obj);
	if (obj) {
		obj->dtype = dtype;
		obj->data = malloc(size);
		if (!(obj->data))
			goto fail;
		memcpy(obj->data, (void*) data, size);
	}
	return obj;
fail:
	if (obj)
		free(obj);
	return NULL;
}

void* yadsl_tester_object_parse()
{
	char* cursor_temp = cursor;
	char dtypebuf[2] = { 0, 0 };
	char data[BUFSIZ];
	
	/* Parse data type */
	if (yadsl_tester_parse_arguments("c", &dtypebuf[0]) != 1) {
		/* Stopping in the middle of an object causes
		   YADSL_TESTER_RET_TOKEN to be thrown */
		cursor = cursor_temp;
	}

	/* Parse data */
	if (yadsl_tester_parse_arguments(dtypebuf, data) != 1)
		return NULL;

	/* Return object */
	return yadsl_tester_object_create(dtypebuf[0], data);
}

void yadsl_tester_object_free(void* object)
{
	yadsl_TesterObject* object_ = (yadsl_TesterObject*) object;
	free(object_->data);
	free(object_);
}

bool yadsl_tester_object_equal(void* object1, void* object2)
{
	yadsl_TesterObject* obj1 = (yadsl_TesterObject*) object1;
	yadsl_TesterObject* obj2 = (yadsl_TesterObject*) object2;
	if (obj1->dtype != obj2->dtype)
		return false;
	return yadsl_tester_compare_arguments(obj1->dtype, obj1->data, obj2->data) == 0;
}

void* yadsl_tester_object_copy(void* object)
{
	yadsl_TesterObject* new_object = malloc(sizeof *new_object);
	if (new_object) {
		yadsl_TesterObject *base = (yadsl_TesterObject *) object;
		size_t size;

		new_object->dtype = base->dtype;
		
		if (base->dtype == 's')
			size = strlen((const char*) base->data) + 1;
		else
			size = yadsl_tester_get_dtype_size(base->dtype);
		
		void* data = malloc(size);
		if (!data) {
			free(new_object);
			return NULL;
		}

		yadsl_tester_copy_argument(base->dtype, base->data, data);
		new_object->data = data;
	}
	return new_object;
}

char yadsl_tester_object_dtype(void* object)
{
	return ((yadsl_TesterObject*) object)->dtype;
}

const void* yadsl_tester_object_data(void* object)
{
	return ((yadsl_TesterObject*) object)->data;
}

yadsl_TesterRet yadsl_tester_error_func(const char* _errfile, int _errline, const char* _errmsg)
{
	errmsg = _errmsg;
	yadsl_tester_seterr_internal(_errfile, _errline);
	return YADSL_TESTER_RET_CUSTOM;
}

void yadsl_tester_log(const char* message, ...)
{
	va_list va;
	size_t col = cursor - buffer + 1;
	va_start(va, message);
	fprintf(stdout, "LOG: \"");
	vfprintf(stdout, message, va);
	fprintf(stdout, "\" (line %zu, col %zu)\n", line, col);
	va_end(va);
}

void
yadsl_tester_throw_func(
	const char* errfile,
	int errline,
	yadsl_TesterRet errnum)
{
	if (errnum == 0)
		errnum = yadsl_tester_error_func(errfile, errline, "error");
	else
		yadsl_tester_seterr_internal(errfile, errline);

	longjmp(env, errnum);
}

void
yadsl_tester_vthrowf_func(
	const char* errfile,
	int errline,
	const char* fmt,
	va_list va)
{
	vsnprintf(errbuf, sizeof(errbuf), fmt, va);
	va_end(va);
	yadsl_tester_throw_func(errfile, errline, yadsl_tester_error_func(errfile, errline, errbuf));
}

void
yadsl_tester_throwf_func(
	const char* errfile,
	int errline,
	const char* fmt,
	...)
{
	va_list va;
	va_start(va, fmt);
	yadsl_tester_vthrowf_func(errfile, errline, fmt, va);
}

void
yadsl_tester_assert_func(
	const char* errfile,
	int errline,
	int condition,
	yadsl_TesterRet errnum)
{
	if (!condition)
		yadsl_tester_throw_func(errfile, errline, errnum);
}

void
yadsl_tester_vassertf_func(
	const char* errfile,
	int errline,
	int condition,
	const char* fmt,
	va_list va)
{
	if (!condition) {

		vsnprintf(errbuf, sizeof(errbuf), fmt, va);
		va_end(va);
		yadsl_tester_assert_func(errfile, errline, condition, yadsl_tester_error_func(errfile, errline, errbuf));
	}
}

void
yadsl_tester_assertf_func(
	const char* errfile,
	int errline,
	int condition,
	const char* fmt,
	...)
{
	if (!condition) {
		va_list va;
		va_start(va, fmt);
		yadsl_tester_vassertf_func(errfile, errline, condition, fmt, va);
	}
}

void
yadsl_tester_vxassertf_func(
	const char* errfile,
	int errline,
	int condition,
	const char* fmt,
	void (*falsecb)(),
	va_list va)
{
	static int reclvl = 0;
	if (!condition) {
		if (reclvl == 0) {
			++reclvl;
			falsecb();
			--reclvl;
		}
		yadsl_tester_vthrowf_func(errfile, errline, fmt, va);
	}
}

void
yadsl_tester_xassertf_func(
	const char* errfile,
	int errline,
	int condition,
	const char* fmt,
	void (*falsecb)(),
	...)
{
	if (!condition) {
		va_list va;
		va_start(va, falsecb);
		yadsl_tester_vxassertf_func(errfile, errline, condition, fmt, falsecb, va);
	}
}

#define NORMALEQ(a, b) (a == b)
#define string const char*
#define STREQ(a, b) (strcmp(a, b) == 0)
#define MAKE_ASSERTEQ(type, fmt, suffix, eqf) \
void yadsl_tester_asserteq ## suffix ## _func ( \
		const char* errfile, int errline, type a, type b, const char* errmsg) { \
	if (!eqf(a, b)) { \
		if (errmsg == NULL) errmsg = #type " inequality"; \
		yadsl_tester_throwf_func(errfile, errline, "%s (" fmt " != " fmt ")", errmsg, a, b); \
	} \
}

MAKE_ASSERTEQ(float, "%f", f, NORMALEQ)
MAKE_ASSERTEQ(int, "%d", i, NORMALEQ)
MAKE_ASSERTEQ(long, "%ld", l, NORMALEQ)
MAKE_ASSERTEQ(char, "%c", c, NORMALEQ)
MAKE_ASSERTEQ(string, "%s", s, STREQ)
MAKE_ASSERTEQ(size_t, "%zu", z, NORMALEQ)
MAKE_ASSERTEQ(intmax_t, "%" SCNdMAX, I, NORMALEQ)

#undef string

/*****************************************************************************/
/*                      STATIC FUNCTIONS DEFINITIONS                         */
/*****************************************************************************/

yadsl_TesterRet yadsl_tester_parse_file_internal()
{
	char prevbuf[BUFSIZ]; /* previous file line */
	char cmdbuf[BUFSIZ]; /* command string */
	yadsl_TesterRet ret = YADSL_TESTER_RET_OK;
	size_t prev_line = line = 0;
	// Read a line from the file and store it in a buffer
	while (fgets(buffer, sizeof(buffer), input_fp)) {
		size_t bufflen = strlen(buffer);
		if (bufflen == sizeof(buffer) - 1)
			// If the buffer overflows, then quit
			return YADSL_TESTER_RET_OVERFLOW;
		// Increment line count
		++line;
		// Iterate through the buffer with a cursor
		for (cursor = buffer; cursor < buffer + bufflen; ++cursor) {
			if (*cursor == '\t' || *cursor == ' ' || *cursor == '\n')
				continue; /* Ignore spacings */
			if (*cursor == '#')
				break; /* Ignore comments */
			if (*cursor == '/') {
				// Detect and parse commands
				if (sscanf(cursor, "/%[^ \t\n]", cmdbuf) == 1) {
					// Move the cursor to after the command
					cursor += strlen(cmdbuf) + 1;
					if (strcmp(cmdbuf, "catch") == 0) {
						// Check if an error already occurred
						if (ret = yadsl_tester_parse_catch_command_internal(ret))
							return ret;
					} else if (strcmp(cmdbuf, "exit") == 0) {
						// Return current status value
						return ret;
					} else {
						if (ret) {
							strcpy(buffer, prevbuf);
							cursor = buffer + strlen(buffer);
							line = prev_line;
							return ret;
						}
						// Set environment
						ret = setjmp(env);
						if (ret == 0) {
							// Call the command parser (can move cursor)
							ret = yadsl_tester_parse(cmdbuf);
						}
						// Copy command to prevbuf
						strncpy(prevbuf, buffer, cursor - buffer);
						prevbuf[cursor - buffer] = '\0';
						// Save previous line
						prev_line = line;
					}
				} else {
					// Command does not match regex
					return ret ? ret : YADSL_TESTER_RET_COMMAND;
				}
			} else {
				// Unexpected token
				return ret ? ret : YADSL_TESTER_RET_TOKEN;
			}
		}
	}
	return ret;
}

int yadsl_tester_parse_argument_internal(const char* format, void* arg, size_t* inc)
{
	if (arg == NULL) return -1;
	if (sscanf(cursor + *inc, "%[ \t\n]%[^ \t\n]", sepbuf, tmpbuf) != 2)
		return -1;
	if (sscanf(tmpbuf, format, arg) != 1)
		return -1;
	*inc += strlen(sepbuf) + strlen(tmpbuf);
	return 0;
}

int yadsl_tester_parse_string_internal(char* arg, size_t* inc)
{
	if (arg == NULL)
		return -1;
	if (sscanf(cursor + *inc, "%[ \t\n]\"%[^\"]\"", sepbuf, tmpbuf) == 2) {
		*inc += strlen(sepbuf) + strlen(tmpbuf) + 2; /* Includes quotation marks */
	} else if (sscanf(cursor + *inc, "%[ \t\n]%[^ \t\n]", sepbuf, tmpbuf) == 2) {
		*inc += strlen(sepbuf) + strlen(tmpbuf);
		if (strcmp(tmpbuf, "\"\"") == 0)
			tmpbuf[0] = '\0'; /* Make "" an empty string in practice */
	} else {
		return -1; /* Then attempts to parse without them */
	}
	strcpy(arg, tmpbuf);
	return 0;
}

yadsl_TesterRet yadsl_tester_parse_catch_command_internal(yadsl_TesterRet ret)
{
	char argbuf[BUFSIZ];
	if (yadsl_tester_parse_arguments("s", argbuf) == 1) {
		if (ret == YADSL_TESTER_RET_CUSTOM) {
			if (errmsg != NULL && strcmp(errmsg, argbuf) == 0) {
				yadsl_tester_error_func(NULL, 0, NULL);
				return YADSL_TESTER_RET_OK;
			}
		} else if (ret >= YADSL_TESTER_RET_OK && ret < YADSL_TESTER_RET_COUNT) {
			if (strcmp(errnames[ret], argbuf) == 0)
				return YADSL_TESTER_RET_OK;
		}
	} else {
		return YADSL_TESTER_RET_ARGUMENT;
	}

	if (ret == YADSL_TESTER_RET_OK)
		ret = YADSL_TESTER_RET_CATCH;

	return ret;
}

void yadsl_tester_print_return_value_info_internal(yadsl_TesterRet ret)
{
	if (ret != YADSL_TESTER_RET_OK) {
		const char* infile = input_file ? input_file : "stdin";
		size_t col = cursor - buffer + 1;

		fprintf(stderr, "%s:%zu:%zu: %s\n", infile, line, col, yadsl_tester_errinfo_internal(ret));
		fprintf(stderr, "stack traceback:\n");
		if (errfile != NULL) fprintf(stderr, "\t%s:%d\n", errfile, errline);
		fprintf(stderr, "\t%s:%zu:%zu\n", infile, line, col);
	}
}

int yadsl_tester_parse_dtype_internal(const char** format_t, void* arg, size_t* inc)
{
	const char* format;
	while (format = *(format_t++)) {
		if (!yadsl_tester_parse_argument_internal(format, arg, inc))
			return 0;
	}
	return 1;
}

yadsl_TesterRet yadsl_tester_argvp_init_internal()
{
	static yadsl_ArgvKeywordArgumentDef kwargdefs[] = {
		{ "--input-file", 1 },
		{ "--log-file", 1 },
		{ "--malloc-failing-countdown", 1 },
		{ "--enable-log-channel", 1 },
		{ NULL, 0 }, /* End of definitions array */
	};

	argvp = yadsl_argvp_create(argc, argv);

	/* If could not allocate argvp, exit */
	if (argvp == NULL) {
		fprintf(stderr, "ERROR: Could not allocate argvp.\n");
		return YADSL_TESTER_RET_MALLOC;
	}

	/* Add keyword arguments */
	yadsl_argvp_add_keyword_arguments(argvp, kwargdefs);

	/* Process positional arguments */
	int pargc = yadsl_argvp_get_positional_argument_count(argvp);
	for (int pargi = 0; pargi < pargc; ++pargi) {
		const char* parg = yadsl_argvp_get_positional_argument(argvp, pargi);
		if (strcmp(parg, "--help") == 0)
			help = 1;
	}

	input_file = yadsl_argvp_get_keyword_argument_value(argvp, "--input-file", 0);
	if (input_file != NULL) {
		input_fp = fopen(input_file, "r");
		if (input_fp == NULL)
			return YADSL_TESTER_RET_FILE;
	} else {
		input_fp = stdin;
	}

#ifdef YADSL_DEBUG
	const char* log_file;
	log_file = yadsl_argvp_get_keyword_argument_value(argvp, "--log-file", 0);
	if (log_file != NULL) {
		log_fp = fopen(log_file, "w");
		if (log_fp == NULL)
			return YADSL_TESTER_RET_FILE;
		yadsl_memdb_set_logger(log_fp);
	} else {
		yadsl_memdb_set_logger(NULL);
	}
#endif

#ifdef YADSL_DEBUG
	size_t malloc_failing_countdown;
	if (yadsl_argvp_parse_keyword_argument_value(argvp,
		"--malloc-failing-countdown", 0, "%zu", &malloc_failing_countdown) == 1) {
		yadsl_memdb_set_fail_countdown(malloc_failing_countdown);
	}
#endif

#ifdef YADSL_DEBUG
	const char* log_channel_name, *kw = "--enable-log-channel";
	while (1) {
		log_channel_name = yadsl_argvp_get_keyword_argument_value(argvp, kw, 0);
		kw = NULL; /* Iterate over keyword argument values */
		if (log_channel_name) {
			yadsl_MemDebugLogChannel log_channel_value;
			if (strcmp(log_channel_name, "ALLOCATION") == 0) {
				log_channel_value = YADSL_MEMDB_LOG_CHANNEL_ALLOCATION;
			} else if (strcmp(log_channel_name, "DEALLOCATION") == 0) {
				log_channel_value = YADSL_MEMDB_LOG_CHANNEL_DEALLOCATION;
			} else if (strcmp(log_channel_name, "LEAKAGE") == 0) {
				log_channel_value = YADSL_MEMDB_LOG_CHANNEL_LEAKAGE;
			} else {
				fprintf(stderr, "WARNING: Invalid log channel name.\n");
				continue;
			}
			yadsl_memdb_log_channel_set(log_channel_value, true);
		} else {
			break;
		}
	}
#endif
	
	return YADSL_TESTER_RET_OK;
}

yadsl_TesterRet yadsl_tester_argvp_release_internal()
{
	yadsl_argvp_destroy(argvp);
	argvp = NULL;

#ifdef YADSL_DEBUG
	if (log_fp != NULL) {
		fclose(log_fp);
		yadsl_memdb_set_logger(NULL);
		log_fp = NULL;
	}
#endif

	if (input_fp != stdin) {
		fclose(input_fp);
		input_fp = NULL;
	}

	return YADSL_TESTER_RET_OK;
}

yadsl_TesterRet yadsl_tester_check_memleak_internal()
{
#ifdef YADSL_DEBUG
	if (yadsl_memdb_amb_list_size() > 0 || yadsl_memdb_error_occurred())
		return YADSL_TESTER_RET_MEMLEAK;	
#endif
	return YADSL_TESTER_RET_OK;
}

const char* yadsl_tester_errinfo_internal(yadsl_TesterRet ret)
{
	if (ret == YADSL_TESTER_RET_CUSTOM) {
		return errmsg; /* By contract, should not be NULL */
	} else if (ret >= YADSL_TESTER_RET_OK && ret < YADSL_TESTER_RET_COUNT) {
		return errnames[ret];
	} else {
		return "invalid error code";
	}
}

void yadsl_tester_print_help_strings_internal()
{
	const char** str = yadsl_tester_help_strings;
	for (; str && *str; ++str) puts(*str);
}

void yadsl_tester_seterr_internal(const char* _errfile, int _errline)
{
	errfile = _errfile;
	errline = _errline;
}

