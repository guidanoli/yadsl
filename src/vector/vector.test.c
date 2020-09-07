#include <vector/vector.h>

#include <stdlib.h>

#include <tester/tester.h>
#include <testerutils/testerutils.h>

const char* yadsl_tester_help_strings[] = {
	"This is the vector test module",
	"",
	"Note: <format> is a format characters used by tester",
	"Check tester documentation for a list of all available formats",
	"Don't use strings as formats since it is not a primitive type",
	"",
	"/new <format> <size>                 create a new vector",
	"/size <expected>                     get vector size",
	"/get <index> <expected>              get element at index",
	"/set <index> <value>                 set element at index",
	"/resize <new-size>                   resizes vector",
	"/destroy                             destroys vector",
	NULL,
};

yadsl_VectorHandle* vector = NULL;
char dtype_format;

yadsl_TesterRet yadsl_tester_init()
{
	return YADSL_TESTER_RET_OK;
}

yadsl_TesterRet yadsl_tester_parse(const char* command)
{
	if yadsl_testerutils_match(command, "new") {
		char format;
		size_t size;
		if (yadsl_tester_parse_arguments("cz", &format, &size) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		yadsl_VectorHandle* new_vector;
		size_t dtype_size = yadsl_tester_get_dtype_size(format);
		if (dtype_size == 0) {
			yadsl_tester_log("Invalid format character %c.", format);
			return YADSL_TESTER_RET_ARGUMENT;
		}
		new_vector = yadsl_vector_create(dtype_size, size);
		if (!new_vector)
			return YADSL_TESTER_RET_MALLOC;
		if (vector)
			yadsl_vector_destroy(vector);
		yadsl_memdb_dump();
		vector = new_vector;
		dtype_format = format;
	} else if yadsl_testerutils_match(command, "get") {
		size_t index;
		if (yadsl_tester_parse_arguments("z", &index) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		const char buf[2] = { dtype_format, '\0' };
		void* expected = malloc(yadsl_tester_get_dtype_size(dtype_format));
		if (!expected)
			return YADSL_TESTER_RET_MALLOC;
		if (yadsl_tester_parse_arguments(buf, expected) != 1) {
			free(expected);
			return YADSL_TESTER_RET_ARGUMENT;
		}
		void* obtained = yadsl_vector_at(vector, index);
		int neq = yadsl_tester_compare_arguments(dtype_format, expected, obtained);
		free(expected);
		if (neq)
			return YADSL_TESTER_RET_ARGUMENT;
	} else if yadsl_testerutils_match(command, "set") {
		size_t index;
		if (yadsl_tester_parse_arguments("z", &index) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		const char buf[2] = { dtype_format, '\0' };
		void* src = malloc(yadsl_tester_get_dtype_size(dtype_format));
		if (!src)
			return YADSL_TESTER_RET_MALLOC;
		if (yadsl_tester_parse_arguments(buf, src) != 1) {
			free(src);
			return YADSL_TESTER_RET_ARGUMENT;
		}
		void* dest = yadsl_vector_at(vector, index);
		yadsl_tester_copy_argument(dtype_format, src, dest);
		free(src);
	} else if yadsl_testerutils_match(command, "resize") {
		yadsl_VectorHandle* new_vector;
		size_t new_size;
		if (yadsl_tester_parse_arguments("z", &new_size) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		new_vector = yadsl_vector_resize(vector, new_size);
		if (!new_vector)
			return YADSL_TESTER_RET_MALLOC;
		yadsl_memdb_dump();
		vector = new_vector;
	} else if yadsl_testerutils_match(command, "size") {
		size_t expected, obtained;
		if (yadsl_tester_parse_arguments("z", &expected) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		obtained = yadsl_vector_size(vector);
		if (obtained != expected)
			return YADSL_TESTER_RET_ARGUMENT;
	} else if yadsl_testerutils_match(command, "destroy") {
		yadsl_vector_destroy(vector);
		yadsl_memdb_dump();
		vector = NULL;
	} else {
		return YADSL_TESTER_RET_COUNT;
	}
	return YADSL_TESTER_RET_OK;
}

yadsl_TesterRet yadsl_tester_release()
{
	if (vector) {
		yadsl_vector_destroy(vector);
		yadsl_memdb_dump();
		vector = NULL;
	}
	return YADSL_TESTER_RET_OK;
}