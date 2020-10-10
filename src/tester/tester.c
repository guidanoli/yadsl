#include <tester/tester.h>

#include <yadsl/posixstring.h>
#include <stdio.h>
#include <stdarg.h>

#if defined(_MSC_VER)
# pragma warning(disable : 4996)
#endif

////////////////////////////////////////////////////////////////////////////////
// STATIC VARIABLES DECLARATIONS
////////////////////////////////////////////////////////////////////////////////

struct tester_object
{
	char dtype;
	void *data;
};

static const char* char_f[] = { "%c", NULL };
static const char* float_f[] = { "%f", "%g", "%e", NULL };
static const char* int_f[] = { "%d", "%u", NULL };
static const char* long_f[] = { "%l", "%lu", NULL };
static const char* size_t_f[] = { "%zu", NULL };

static size_t line; // line count
static const char* external_ret_info = NULL; // external return value
static const char* tester_ret_infos[YADSL_TESTER_RET_COUNT]; // return value

static char buffer[BUFSIZ] = "", // file line
buffer2[BUFSIZ] = "", // previous file line
command[BUFSIZ] = "", // command string
sep[BUFSIZ] = "", // separation characters
temp[BUFSIZ] = ""; // temporary variable

static char* cursor = buffer; // buffer cursor

////////////////////////////////////////////////////////////////////////////////
// STATIC FUNCTIONS DECLARATIONS
////////////////////////////////////////////////////////////////////////////////

static yadsl_TesterRet yadsl_tester_main_internal(int argc, char** argv);
static void yadsl_tester_load_return_values_internal();
static yadsl_TesterRet yadsl_tester_parse_file_internal(FILE* fp);
static void yadsl_tester_print_cursor_position_internal(FILE* fp, size_t spacing);
static yadsl_TesterRet yadsl_tester_parse_catch_command_internal(yadsl_TesterRet ret);
static int yadsl_tester_parse_argument_internal(const char* format, void* arg, size_t* inc);
static int yadsl_tester_parse_string_internal(char* arg, size_t* inc);
static void yadsl_tester_print_return_value_info_internal(yadsl_TesterRet ret);
static int yadsl_tester_parse_dtype_internal(const char** format_t, void* arg, size_t* inc);
static int yadsl_tester_has_flag_internal(int argc, char** argv, const char* flag);

////////////////////////////////////////////////////////////////////////////////
// EXTERN FUNCTIONS DEFINITIONS
////////////////////////////////////////////////////////////////////////////////

/**
* Usage: <program> [script-path [/LOG] [/I]]
* If script-path is not provided,
* then help strings are displayed.
*/
int main(int argc, char** argv)
{
	yadsl_TesterRet exitReturn, ret;
	FILE* logger = NULL;
	if (yadsl_tester_has_flag_internal(argc, argv, "/LOG")) {
		logger = fopen("memdb.log", "w");
		yadsl_memdb_set_logger(logger);
	}
	ret = yadsl_tester_main_internal(argc, argv);
	exitReturn = yadsl_tester_release();
	if (ret == YADSL_TESTER_RET_OK)
		ret = exitReturn;
	if (ret == YADSL_TESTER_RET_OK &&
		(yadsl_memdb_list_size() || yadsl_memdb_error_occurred()))
		ret = YADSL_TESTER_RET_MEMLEAK;
	yadsl_tester_print_return_value_info_internal(ret);
	yadsl_memdb_clear_list();
	yadsl_memdb_set_logger(NULL);
	if (logger) fclose(logger);
	return ret;
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

void* yadsl_tester_object_parse()
{
	char* cursor_temp = cursor;
	char dtype[2] = {0, 0};
	size_t size;
	char data[BUFSIZ];
	struct tester_object* obj = NULL;
	if (yadsl_tester_parse_arguments("c", dtype) != 1)
		goto fail;
	if (yadsl_tester_parse_arguments(dtype, data) != 1)
		goto fail;
	obj = malloc(sizeof *obj);
	if (!obj)
		goto fail;
	obj->dtype = *dtype;
	if (*dtype == 's')
		size = (strlen(data) + 1) * sizeof(char);
	else
		size = yadsl_tester_get_dtype_size(*dtype);
	obj->data = malloc(size);
	if (!(obj->data))
		goto fail;
	memcpy(obj->data, (void*) data, size);
	return (void*) obj;
fail:
	if (obj)
		free(obj);
	cursor = cursor_temp;
	return NULL;
}

void yadsl_tester_object_free(void* object)
{
	free(((struct tester_object*) object)->data);
	free(object);
}

bool yadsl_tester_object_equal(void* object1, void* object2)
{
	struct tester_object* obj1 = (struct tester_object*) object1;
	struct tester_object* obj2 = (struct tester_object*) object2;
	if (obj1->dtype != obj2->dtype)
		return false;
	return yadsl_tester_compare_arguments(obj1->dtype, obj1->data, obj2->data) == 0;
}

void* yadsl_tester_object_copy(void* object)
{
	struct tester_object* new_object = malloc(sizeof *new_object);
	if (new_object) {
		struct tester_object *base = (struct tester_object *) object;
		size_t size;

		new_object->dtype = base->dtype;
		
		if (base->dtype == 's')
			size = (strlen((char*) base->data)+1)*sizeof(char);
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

////////////////////////////////////////////////////////////////////////////////
// STATIC FUNCTIONS DEFINITIONS
////////////////////////////////////////////////////////////////////////////////

static yadsl_TesterRet yadsl_tester_main_internal(int argc, char** argv)
{
	FILE* fp;
	yadsl_TesterRet ret;
	// First, load return value informations
	yadsl_tester_load_return_values_internal();
	if (argc == 1) {
		// If no arguments were passed, then print help strings
		yadsl_tester_print_help_strings();
		return YADSL_TESTER_RET_OK;
	}
	if (yadsl_tester_has_flag_internal(argc, argv, "/I")) {
		// Use standard input
		fp = stdin;
	} else {
		// Open file whose path was passed as argument
		fp = fopen(argv[1], "r");
		if (fp == NULL)
			return YADSL_TESTER_RET_FILE;
	}
	// Initialize tester
	if (ret = yadsl_tester_init())
		return ret;
	// Parse script
	return yadsl_tester_parse_file_internal(fp);
}

static yadsl_TesterRet yadsl_tester_parse_file_internal(FILE* fp)
{
	yadsl_TesterRet ret = YADSL_TESTER_RET_OK;
	size_t prev_line = line = 0;
	// Read a line from the file and store it in a buffer
	while (fgets(buffer, sizeof(buffer), fp)) {
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
static void yadsl_tester_load_return_values_internal()
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
		{YADSL_TESTER_RET_EXTERNAL, "external"},
	};
	for (i = 0; i < sizeof(nativeValues) / sizeof(nativeValues[0]); ++i) {
		struct returnValue retVal = nativeValues[i];
		tester_ret_infos[retVal.value] = retVal.info;
	}
}

static int yadsl_tester_parse_argument_internal(const char* format, void* arg, size_t* inc)
{
	if (arg == NULL) return -1;
	if (sscanf(cursor + *inc, "%[ \t\n]%[^ \t\n]", sep, temp) != 2)
		return -1;
	if (sscanf(temp, format, arg) != 1)
		return -1;
	*inc += strlen(sep) + strlen(temp);
	return 0;
}

static int yadsl_tester_parse_string_internal(char* arg, size_t* inc)
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

static yadsl_TesterRet yadsl_tester_parse_catch_command_internal(yadsl_TesterRet ret)
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
	fprintf(stderr, "ERROR: Could not catch \"%s\"\n", yadsl_tester_get_return_value_info(ret));
jump_print:
	return ret;
}

static void yadsl_tester_print_return_value_info_internal(yadsl_TesterRet ret)
{
	if (ret) {
		size_t spacing = fprintf(stderr, "ERROR: \"%s\" ", yadsl_tester_get_return_value_info(ret));
		yadsl_tester_print_cursor_position_internal(stderr, spacing);
	}
}

// Prints the buffer and the cursor current position with an arrow (^)
// spacing = how many characters have been printed out on the current line
// HINT: printf-like functions return the number of characters printed out
static void yadsl_tester_print_cursor_position_internal(FILE* fp, size_t spacing)
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

static int yadsl_tester_parse_dtype_internal(const char** format_t, void* arg, size_t* inc)
{
	const char* format;
	while (format = *(format_t++)) {
		if (!yadsl_tester_parse_argument_internal(format, arg, inc))
			return 0;
	}
	return 1;
}

int yadsl_tester_has_flag_internal(int argc, char** argv, const char* flag)
{
	while (argc--)
		if (strcmp(argv[argc], flag) == 0)
			return 1;
	return 0;
}
