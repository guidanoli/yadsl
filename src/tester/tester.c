#include <tester/tester.h>

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

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
*size_t_f[] = { "%zu", NULL };

static size_t line; /* line count */
static const char* external_ret_info; /* external return value */
static const char* tester_ret_infos[YADSL_TESTER_RET_COUNT]; /* return value */

static char
buffer[BUFSIZ], /* file line */
buffer2[BUFSIZ], /* previous file line */
command[BUFSIZ], /* command string */
sep[BUFSIZ], /* separation characters */
temp[BUFSIZ]; /* temporary variable */

static char* cursor = buffer; /* buffer cursor */

static yadsl_ArgvParserHandle* argvp; /* argument vector parser */

static FILE* input_fp; /* Input file pointer */
static FILE* log_fp; /* Logger file pointer */

static int argc; /* Argument count */
static char** argv; /* Argument vector */

#define YADSL_TESTER_PROPAGATE(status, temp) \
do { \
	if (!status) \
		status = temp; \
} while (0)

/*****************************************************************************/
/*                      STATIC FUNCTIONS DECLARATIONS                        */
/*****************************************************************************/

static yadsl_TesterRet yadsl_tester_argvp_init_internal();
static yadsl_TesterRet yadsl_tester_argvp_release_internal();
static yadsl_TesterRet yadsl_tester_check_memleak_internal();
static void yadsl_tester_load_return_values_internal();
static yadsl_TesterRet yadsl_tester_parse_file_internal();
static void yadsl_tester_print_cursor_position_internal(FILE* fp, size_t spacing);
static yadsl_TesterRet yadsl_tester_parse_catch_command_internal(yadsl_TesterRet ret);
static int yadsl_tester_parse_argument_internal(const char* format, void* arg, size_t* inc);
static int yadsl_tester_parse_string_internal(char* arg, size_t* inc);
static void yadsl_tester_print_return_value_info_internal(yadsl_TesterRet ret);
static int yadsl_tester_parse_dtype_internal(const char** format_t, void* arg, size_t* inc);

/*****************************************************************************/
/*                      EXTERN FUNCTIONS DEFINITIONS                         */
/*****************************************************************************/

int main(int argc_, char** argv_)
{
	yadsl_TesterRet status = YADSL_TESTER_RET_OK, temp;

	/* Store arguments globally */
	argc = argc_;
	argv = argv_;

	/* Load return values */
	yadsl_tester_load_return_values_internal();

	/* Initialize argument vector parser (critical) */
	if (status = yadsl_tester_argvp_init_internal())
		return status;

	/* If no arguments were passed, then print help strings */
	if (argc == 1) {
		yadsl_tester_print_help_strings();
		return YADSL_TESTER_RET_OK;
	}

	/* Initialize tester (may fail) */
	status = yadsl_tester_init();

	/* If initialization failed, skip parsing */
	if (!status) {

		/* Parse script (may fail) */
		status = yadsl_tester_parse_file_internal();

	}

	/* Release tester (may fail) */
	temp = yadsl_tester_release();

	/* If parsing succeeds, take status code from release callback */
	YADSL_TESTER_PROPAGATE(status, temp);

	/* Release argument vector parser */
	temp = yadsl_tester_argvp_release_internal();

	/* Propagate error code, if status is OK */
	YADSL_TESTER_PROPAGATE(status, temp);

#ifdef YADSL_DEBUG
	/* Clear allocated memory block list */
	yadsl_memdb_clear_amb_list();
#endif

	/* Check for memory leak (may fail) */
	temp = yadsl_tester_check_memleak_internal();

	/* Propagate error code, if status is OK */
	YADSL_TESTER_PROPAGATE(status, temp);

#ifdef YADSL_DEBUG
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

yadsl_TesterRet yadsl_tester_return_external_value(const char* info)
{
	external_ret_info = info;
	return YADSL_TESTER_RET_EXTERNAL;
}

void yadsl_tester_print_help_strings()
{
	const char** str = yadsl_tester_help_strings;
	for (; str && *str; ++str) puts(*str);
}

const char* yadsl_tester_get_return_value_info(yadsl_TesterRet returnValue)
{
	if (returnValue < YADSL_TESTER_RET_OK || returnValue >= YADSL_TESTER_RET_COUNT)
		return "Invalid return value";
	if (returnValue == YADSL_TESTER_RET_EXTERNAL)
		if (external_ret_info == NULL)
			return "Missing external return value information";
		else
			return external_ret_info;
	return tester_ret_infos[returnValue];
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

/*****************************************************************************/
/*                      STATIC FUNCTIONS DEFINITIONS                         */
/*****************************************************************************/

yadsl_TesterRet yadsl_tester_parse_file_internal()
{
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
				if (sscanf(cursor, "/%[^ \t\n]", command) == 1) {
					// Move the cursor to after the command
					cursor += strlen(command) + 1;
					if (strcmp(command, "catch") == 0) {
						// Check if an error already occurred
						if (ret = yadsl_tester_parse_catch_command_internal(ret))
							return ret;
					} else if (strcmp(command, "exit") == 0) {
						// Return current status value
						return ret;
					} else {
						if (ret) {
							strcpy(buffer, buffer2);
							cursor = buffer + strlen(buffer);
							line = prev_line;
							return ret;
						}
						external_ret_info = NULL;
						// Call the command parser (can move cursor)
						ret = yadsl_tester_parse(command);
						// Copy command to buffer2
						strncpy(buffer2, buffer, cursor - buffer);
						buffer2[cursor - buffer] = '\0';
						// Save previous line
						prev_line = line;
					}
				} else {
					// Command does not match regex
					return YADSL_TESTER_RET_COMMAND;
				}
			} else {
				// Unexpected token
				return YADSL_TESTER_RET_TOKEN;
			}
		}
	}
	return ret;
}

void yadsl_tester_load_return_values_internal()
{
	struct returnValue
	{
		yadsl_TesterRet value;
		const char* info;
	};
	size_t i;
	struct returnValue nativeValues[] = {
		{YADSL_TESTER_RET_OK, "ok"},
		{YADSL_TESTER_RET_FILE, "file"},
		{YADSL_TESTER_RET_MALLOC, "malloc"},
		{YADSL_TESTER_RET_MEMLEAK, "memleak"},
		{YADSL_TESTER_RET_OVERFLOW, "overflow"},
		{YADSL_TESTER_RET_COMMAND, "command"},
		{YADSL_TESTER_RET_ARGUMENT, "argument"},
		{YADSL_TESTER_RET_RETURN, "return"},
		{YADSL_TESTER_RET_TOKEN, "token"},
		{YADSL_TESTER_RET_CATCH, "catch"},
		{YADSL_TESTER_RET_EXTERNAL, "external"},
	};
	for (i = 0; i < sizeof(nativeValues) / sizeof(nativeValues[0]); ++i) {
		struct returnValue retVal = nativeValues[i];
		tester_ret_infos[retVal.value] = retVal.info;
	}
}

int yadsl_tester_parse_argument_internal(const char* format, void* arg, size_t* inc)
{
	if (arg == NULL) return -1;
	if (sscanf(cursor + *inc, "%[ \t\n]%[^ \t\n]", sep, temp) != 2)
		return -1;
	if (sscanf(temp, format, arg) != 1)
		return -1;
	*inc += strlen(sep) + strlen(temp);
	return 0;
}

int yadsl_tester_parse_string_internal(char* arg, size_t* inc)
{
	if (arg == NULL)
		return -1;
	if (sscanf(cursor + *inc, "%[ \t\n]\"%[^\"]\"", sep, temp) == 2) {
		*inc += strlen(sep) + strlen(temp) + 2; /* Includes quotation marks */
	} else if (sscanf(cursor + *inc, "%[ \t\n]%[^ \t\n]", sep, temp) == 2) {
		*inc += strlen(sep) + strlen(temp);
		if (strcmp(temp, "\"\"") == 0)
			temp[0] = '\0'; /* Make "" an empty string in practice */
	} else {
		return -1; /* Then attempts to parse without them */
	}
	strcpy(arg, temp);
	return 0;
}

yadsl_TesterRet yadsl_tester_parse_catch_command_internal(yadsl_TesterRet ret)
{
	char arg[BUFSIZ] = "";
	if (yadsl_tester_parse_arguments("s", arg) == 1) {
		if (ret == YADSL_TESTER_RET_EXTERNAL) {
			if (external_ret_info != NULL &&
				strcmp(external_ret_info, arg) == 0)
				return YADSL_TESTER_RET_OK;
		} else if (ret >= YADSL_TESTER_RET_OK && ret < YADSL_TESTER_RET_COUNT) {
			if (strcmp(tester_ret_infos[ret], arg) == 0)
				return YADSL_TESTER_RET_OK;
		}
	} else {
		ret = YADSL_TESTER_RET_ARGUMENT;
		goto jump_print;
	}
	if (ret == YADSL_TESTER_RET_OK) {
		fprintf(stderr, "ERROR: Tried to catch \"%s\" but there was no error\n", arg);
		ret = YADSL_TESTER_RET_CATCH;
	} else {
		fprintf(stderr, "ERROR: Tried to catch \"%s\" but error was \"%s\"\n",
			arg, yadsl_tester_get_return_value_info(ret));
	}
jump_print:
	return ret;
}

void yadsl_tester_print_return_value_info_internal(yadsl_TesterRet ret)
{
	if (ret) {
		size_t spacing = fprintf(stderr, "ERROR: \"%s\" ",
			yadsl_tester_get_return_value_info(ret));
		yadsl_tester_print_cursor_position_internal(stderr, spacing);
	}
}

// Prints the buffer and the cursor current position with an arrow (^)
// spacing = how many characters have been printed out on the current line
// HINT: printf-like functions return the number of characters printed out
void yadsl_tester_print_cursor_position_internal(FILE* fp, size_t spacing)
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
	while (spacing--) fprintf(fp, " ");
	fprintf(fp, "^\n");
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
		{ "--malloc-failing-rate", 1 },
		{ "--prng-seed", 1 },
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

	const char* input_file;
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
	float malloc_failing_rate;
	if (yadsl_argvp_parse_keyword_argument_value(argvp,
		"--malloc-failing-rate", 0, "%f", &malloc_failing_rate) == 1) {
		float effective_malloc_failing_rate;
		yadsl_memdb_set_fail_rate(malloc_failing_rate);
		effective_malloc_failing_rate = yadsl_memdb_get_fail_rate();
		if (effective_malloc_failing_rate != malloc_failing_rate) {
			fprintf(stderr, "WARNING: Failing rate clamped to %f.\n",
				effective_malloc_failing_rate);
		}
	} else {
		yadsl_memdb_set_fail_rate(0.f);
	}
#endif

#ifdef YADSL_DEBUG
	unsigned int prng_seed;
	if (yadsl_argvp_parse_keyword_argument_value(argvp,
		"--prng-seed", 0, "%u", &prng_seed) == 1) {
		yadsl_memdb_set_prng_seed(prng_seed);
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

#ifdef YADSL_DEBUG
	if (log_fp != NULL) {
		fclose(log_fp);
		log_fp = NULL;
		yadsl_memdb_set_logger(NULL);
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