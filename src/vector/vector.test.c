#include <vector/vector.h>

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
char data[BUFSIZ];
char dtype_format;

yadsl_TesterRet yadsl_tester_init()
{
	return YADSL_TESTER_RET_OK;
}

yadsl_TesterRet yadsl_tester_parse(const char* command)
{
	if (yadsl_testerutils_match(command, "new")) {
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
#ifdef YADSL_DEBUG
		yadsl_memdb_dump();
#endif
		vector = new_vector;
		dtype_format = format;
	} else if (yadsl_testerutils_match(command, "get")) {
		size_t index;
		const char fmt[] = { 'z', dtype_format, '\0' };
		if (yadsl_tester_parse_arguments(fmt, &index, data) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		void* obtained = yadsl_vector_at(vector, index);
		if (yadsl_tester_compare_arguments(dtype_format, data, obtained))
			return YADSL_TESTER_RET_ARGUMENT;
	} else if (yadsl_testerutils_match(command, "set")) {
		size_t index;
		const char fmt[] = { 'z', dtype_format, '\0' };
		if (yadsl_tester_parse_arguments(fmt, &index, data) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		void* dest = yadsl_vector_at(vector, index);
		yadsl_tester_copy_argument(dtype_format, data, dest);
	} else if (yadsl_testerutils_match(command, "resize")) {
		bool ret;
		size_t new_size;
		if (yadsl_tester_parse_arguments("z", &new_size) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		ret = yadsl_vector_resize(&vector, new_size);
		if (!ret)
			return YADSL_TESTER_RET_MALLOC;
#ifdef YADSL_DEBUG
		yadsl_memdb_dump();
#endif
	} else if (yadsl_testerutils_match(command, "size")) {
		size_t expected, obtained;
		if (yadsl_tester_parse_arguments("z", &expected) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		obtained = yadsl_vector_size(vector);
		if (obtained != expected)
			return YADSL_TESTER_RET_ARGUMENT;
	} else if (yadsl_testerutils_match(command, "destroy")) {
		yadsl_vector_destroy(vector);
#ifdef YADSL_DEBUG
		yadsl_memdb_dump();
#endif
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
#ifdef YADSL_DEBUG
		yadsl_memdb_dump();
#endif
		vector = NULL;
	}
	return YADSL_TESTER_RET_OK;
}