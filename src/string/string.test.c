#include <string/string.h>

#include <string.h>

#include <tester/tester.h>
#include <testerutils/testerutils.h>

#if defined(_MSC_VER)
# pragma warning(disable : 4996)
#endif

/* check if index is on range of C array
   (considers that index is non-negative) */
#define check_index(index, c_array) \
do { \
	if (index >= sizeof(c_array) / sizeof(*c_array)) { \
		yadsl_tester_log("Index %zu out of range (from 0 to %zu)", \
			index, sizeof(c_array) / sizeof(*c_array) - 1); \
		return YADSL_TESTER_RET_ARGUMENT; \
	} \
} while(0)

const char* yadsl_tester_help_strings[] = {
	"This is the interactive module of the string library\n"
	"You can interact with many string at the same time\n"
	"Strings are identified with #, which range from 0 to 9\n"
	"The registered actions are the following...",
	"/save <#> <string>",
	"/compare <#-first> <#-second> <eq?>",
	"/duplicate <#-src> <#-dest>",
	"/compare-ic <#-first> <#-second> <eq?>",
	0,
};

char buffer[BUFSIZ];
char strings[10][BUFSIZ];

yadsl_TesterRet yadsl_tester_init()
{
	for (size_t i = 0; i < sizeof(strings) / sizeof(*strings); ++i)
		strings[i][0] = '\0';
	return YADSL_TESTER_RET_OK;
}

yadsl_TesterRet yadsl_tester_parse(const char* command)
{
	if (yadsl_testerutils_match(command, "save")) {
		size_t index;
		if (yadsl_tester_parse_arguments("z", &index) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		check_index(index, strings);
		if (yadsl_tester_parse_arguments("s", strings[index]) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
	} else if (yadsl_testerutils_match(command, "compare")) {
		size_t indices[2];
		bool expected, obtained;
		if (yadsl_tester_parse_arguments("zzs", indices, indices+1, buffer) != 3)
			return YADSL_TESTER_RET_ARGUMENT;
		check_index(indices[0], strings);
		check_index(indices[1], strings);
		expected = yadsl_testerutils_str_to_bool(buffer);
		obtained = strcmp(strings[indices[0]], strings[indices[1]]) == 0;
		if (expected != obtained)
			return YADSL_TESTER_RET_RETURN;
	} else if (yadsl_testerutils_match(command, "duplicate")) {
		size_t indices[2];
		char* temp;
		if (yadsl_tester_parse_arguments("zz", indices, indices + 1) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		check_index(indices[0], strings);
		check_index(indices[1], strings);
		temp = yadsl_string_duplicate(strings[indices[0]]);
		if (temp == NULL)
			return YADSL_TESTER_RET_MALLOC;
		strcpy(strings[indices[1]], temp);
		free(temp);
	} else if (yadsl_testerutils_match(command, "compare-ic")) {
		size_t indices[2];
		bool expected, obtained;
		if (yadsl_tester_parse_arguments("zzs", indices, indices + 1, buffer) != 3)
			return YADSL_TESTER_RET_ARGUMENT;
		check_index(indices[0], strings);
		check_index(indices[1], strings);
		expected = yadsl_testerutils_str_to_bool(buffer);
		obtained = yadsl_string_compare_ic(strings[indices[0]], strings[indices[1]]) == 0;
		if (expected != obtained)
			return YADSL_TESTER_RET_RETURN;
	} else {
		return YADSL_TESTER_RET_COUNT;
	}
	return YADSL_TESTER_RET_OK;
}

yadsl_TesterRet yadsl_tester_release()
{
	return YADSL_TESTER_RET_OK;
}
