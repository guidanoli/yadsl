#include <vector/vector.h>

#include <stdlib.h>

#include <tester/tester.h>
#include <testerutils/testerutils.h>

const char* TesterHelpStrings[] = {
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

TesterReturnValue TesterInitCallback()
{
	return TESTER_OK;
}

TesterReturnValue TesterParseCallback(const char* command)
{
	if matches(command, "new") {
		char format;
		size_t size;
		if (TesterParseArguments("cz", &format, &size) != 2)
			return TESTER_ARGUMENT;
		yadsl_VectorHandle* new_vector;
		size_t dtype_size = TesterGetDataTypeSize(format);
		if (dtype_size == 0) {
			TesterLog("Invalid format character %c.", format);
			return TESTER_ARGUMENT;
		}
		new_vector = yadsl_vector_create(dtype_size, size);
		if (!new_vector)
			return TESTER_MALLOC;
		if (vector)
			yadsl_vector_destroy(vector);
		yadsl_memdb_dump();
		vector = new_vector;
		dtype_format = format;
	} else if matches(command, "get") {
		size_t index;
		if (TesterParseArguments("z", &index) != 1)
			return TESTER_ARGUMENT;
		const char buf[2] = { dtype_format, '\0' };
		void* expected = malloc(TesterGetDataTypeSize(dtype_format));
		if (!expected)
			return TESTER_MALLOC;
		if (TesterParseArguments(buf, expected) != 1) {
			free(expected);
			return TESTER_ARGUMENT;
		}
		void* obtained = yadsl_vector_at(vector, index);
		int neq = TesterCompare(dtype_format, expected, obtained);
		free(expected);
		if (neq)
			return TESTER_ARGUMENT;
	} else if matches(command, "set") {
		size_t index;
		if (TesterParseArguments("z", &index) != 1)
			return TESTER_ARGUMENT;
		const char buf[2] = { dtype_format, '\0' };
		void* src = malloc(TesterGetDataTypeSize(dtype_format));
		if (!src)
			return TESTER_MALLOC;
		if (TesterParseArguments(buf, src) != 1) {
			free(src);
			return TESTER_ARGUMENT;
		}
		void* dest = yadsl_vector_at(vector, index);
		TesterCopy(dtype_format, src, dest);
		free(src);
	} else if matches(command, "resize") {
		yadsl_VectorHandle* new_vector;
		size_t new_size;
		if (TesterParseArguments("z", &new_size) != 1)
			return TESTER_ARGUMENT;
		new_vector = yadsl_vector_resize(vector, new_size);
		if (!new_vector)
			return TESTER_MALLOC;
		yadsl_memdb_dump();
		vector = new_vector;
	} else if matches(command, "size") {
		size_t expected, obtained;
		if (TesterParseArguments("z", &expected) != 1)
			return TESTER_ARGUMENT;
		obtained = yadsl_vector_size(vector);
		if (obtained != expected)
			return TESTER_ARGUMENT;
	} else if matches(command, "destroy") {
		yadsl_vector_destroy(vector);
		yadsl_memdb_dump();
		vector = NULL;
	} else {
		return TESTER_COUNT;
	}
	return TESTER_OK;
}

TesterReturnValue TesterExitCallback()
{
	if (vector) {
		yadsl_vector_destroy(vector);
		yadsl_memdb_dump();
		vector = NULL;
	}
	return TESTER_OK;
}