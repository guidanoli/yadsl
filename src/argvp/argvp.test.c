#include <argvp/argvp.h>

#include <string.h>

#include <tester/tester.h>
#include <testerutils/testerutils.h>

#if defined(_MSC_VER)
# pragma warning(disable : 4996)
#endif

#define check_non_null(vec) do { \
	if (!vec) { \
		yadsl_tester_log("Unexpected NULL " #vec); \
		return YADSL_TESTER_RET_ARGUMENT; \
	} \
} while(0)

const char *yadsl_tester_help_strings[] = {
	"This is the module for testing the argvp library",
	"There is one available slot for storing a argv parser",
	"Any remaining argvp parser is deallocated at the end",
	"The available commands are:",
	"",
	"/create <args>",
	"/argc <expected-argc>",
	"/argv <idx-arg> <expected-argv>",
	"/add-kw-arg <arg-name> <value-cnt>",
	"/get-pos-arg-cnt <expected-cnt>",
	"/get-pos-arg <idx-arg> <expected-arg>",
	"/parse-pos-arg <idx-arg> <dtype> <expected-arg>",
	"/get-kw-arg-val <arg-name|NULL> <idx-val> <expected-arg>",
	"/parse-kw-arg-val <arg-name|NULL> <idx-val> <dtype> <expected-arg>",
	0,
};

yadsl_ArgvParserHandle* argvp;

char argvbuf[BUFSIZ]; /* "arg11\0arg12\0..." */
char* argv[BUFSIZ / 2]; /* [ &arg11, &arg12, ... ] */
int argc;

char buffer[BUFSIZ], buffer2[BUFSIZ];

static void tokenize_buffer(char* buf, char** argv, int* argc_ptr)
{
	int argi = 0;
	char* pch;
	pch = strtok(buf, " ");
	while (pch != NULL) {
		argv[argi++] = pch;
		pch = strtok(NULL, " ");
	}
	*argc_ptr = argi;
}

yadsl_TesterRet yadsl_tester_init()
{
	return YADSL_TESTER_RET_OK;
}

yadsl_TesterRet yadsl_tester_parse(const char *command)
{
	yadsl_TesterRet ret = YADSL_TESTER_RET_OK;
	if (yadsl_testerutils_match(command, "create")) {
		yadsl_ArgvParserHandle* temp;
		if (argvp) {
			yadsl_argvp_destroy(argvp);
			argvp = NULL;
		}
		if (yadsl_tester_parse_arguments("s", argvbuf) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		tokenize_buffer(argvbuf, argv, &argc);
		temp = yadsl_argvp_create(argc, argv);
		if (temp == NULL)
			return YADSL_TESTER_RET_MALLOC;
		argvp = temp;
	} else if (yadsl_testerutils_match(command, "argc")) {
		int expected;
		if (yadsl_tester_parse_arguments("i", &expected) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		if (argc != expected)
			return YADSL_TESTER_RET_RETURN;
	} else if (yadsl_testerutils_match(command, "argv")) {
		int argi;
		if (yadsl_tester_parse_arguments("is", &argi, buffer) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		if (argi >= argc) {
			yadsl_tester_log("Invalid index %zu (>=%zu)", argi, argc);
			return YADSL_TESTER_RET_ARGUMENT;
		}
		if (strcmp(argv[argi], buffer)) {
			yadsl_tester_log("%s != %s", argv[argi], buffer);
			return YADSL_TESTER_RET_RETURN;
		}
	} else if (yadsl_testerutils_match(command, "add-kw-arg")) {
		int valc;
		if (yadsl_tester_parse_arguments("si", buffer, &valc) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		check_non_null(argvp);
		yadsl_argvp_add_keyword_argument(argvp, buffer, valc);
	} else if (yadsl_testerutils_match(command, "get-pos-arg-cnt")) {
		int expected, obtained;
		if (yadsl_tester_parse_arguments("i", &expected) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		check_non_null(argvp);
		obtained = yadsl_argvp_get_positional_argument_count(argvp);
		if (expected != obtained) {
			yadsl_tester_log("%d != %d", expected, obtained);
			return YADSL_TESTER_RET_RETURN;
		}
	} else if (yadsl_testerutils_match(command, "get-pos-arg")) {
		int argi;
		const char* obtained;
		if (yadsl_tester_parse_arguments("is", &argi, buffer) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		check_non_null(argvp);
		obtained = yadsl_argvp_get_positional_argument(argvp, argi);
		if (obtained == NULL)
			obtained = "NULL";
		if (strcmp(obtained, buffer)) {
			yadsl_tester_log("%s != %s", obtained, buffer);
			return YADSL_TESTER_RET_RETURN;
		}
	} else if (yadsl_testerutils_match(command, "parse-pos-arg")) {
		int argi;
		int n;
		char const* fmt;
		char dtype;
		void* expected = NULL, *obtained = NULL;
		if (yadsl_tester_parse_arguments("i", &argi) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		expected = yadsl_tester_object_parse();
		if (expected == NULL) {
			ret = YADSL_TESTER_RET_MALLOC;
			goto fail_parse_pos_arg;
		}
		check_non_null(argvp);
		dtype = yadsl_tester_object_dtype(expected);
		fmt = yadsl_tester_get_dtype_fmt(dtype);
		n = yadsl_argvp_parse_positional_argument(argvp, argi, fmt, buffer);
		if (n == 0) {
			ret = YADSL_TESTER_RET_RETURN;
			goto fail_parse_pos_arg;
		}
		obtained = yadsl_tester_object_create(dtype, buffer);
		if (obtained == NULL) {
			ret = YADSL_TESTER_RET_MALLOC;
			goto fail_parse_pos_arg;
		}
		if (!yadsl_tester_object_equal(expected, obtained)) {
			sprintf(buffer, "%s != %s", fmt, fmt);
			yadsl_tester_log(buffer, yadsl_tester_object_data(expected),
			                         yadsl_tester_object_data(obtained));
			ret = YADSL_TESTER_RET_RETURN;
		}
	fail_parse_pos_arg:
		if (expected)
			yadsl_tester_object_free(expected);
		if (obtained)
			yadsl_tester_object_free(obtained);
	} else if (yadsl_testerutils_match(command, "get-kw-arg-val")) {
		int vali;
		const char* obtained, *kw = NULL;
		if (yadsl_tester_parse_arguments("sis", buffer, &vali, buffer2) != 3)
			return YADSL_TESTER_RET_ARGUMENT;
		check_non_null(argvp);
		if (!yadsl_testerutils_match(buffer, "NULL"))
			kw = buffer;
		obtained = yadsl_argvp_get_keyword_argument_value(argvp, kw, vali);
		if (obtained == NULL)
			obtained = "NULL";
		if (strcmp(obtained, buffer2)) {
			yadsl_tester_log("%s != %s", obtained, buffer2);
			return YADSL_TESTER_RET_RETURN;
		}
	} else if (yadsl_testerutils_match(command, "parse-kw-arg-val")) {
		int vali;
		int n;
		char const* fmt, *kw = NULL;
		char dtype;
		void* expected = NULL, * obtained = NULL;
		if (yadsl_tester_parse_arguments("si", buffer, &vali) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		check_non_null(argvp);
		if (!yadsl_testerutils_match(buffer, "NULL"))
			kw = buffer;
		expected = yadsl_tester_object_parse();
		if (expected == NULL) {
			ret = YADSL_TESTER_RET_MALLOC;
			goto fail_parse_kw_arg_val;
		}
		check_non_null(argvp);
		dtype = yadsl_tester_object_dtype(expected);
		fmt = yadsl_tester_get_dtype_fmt(dtype);
		n = yadsl_argvp_parse_keyword_argument_value(argvp, kw, vali, fmt, buffer2);
		if (n == 0) {
			ret = YADSL_TESTER_RET_RETURN;
			goto fail_parse_kw_arg_val;
		}
		obtained = yadsl_tester_object_create(dtype, buffer2);
		if (obtained == NULL) {
			ret = YADSL_TESTER_RET_MALLOC;
			goto fail_parse_kw_arg_val;
		}
		if (!yadsl_tester_object_equal(expected, obtained)) {
			sprintf(buffer, "%s != %s", fmt, fmt);
			yadsl_tester_log(buffer, yadsl_tester_object_data(expected),
				yadsl_tester_object_data(obtained));
			ret = YADSL_TESTER_RET_RETURN;
		}
	fail_parse_kw_arg_val:
		if (expected)
			yadsl_tester_object_free(expected);
		if (obtained)
			yadsl_tester_object_free(obtained);
	} else {
		return YADSL_TESTER_RET_COUNT;
	}
	return ret;
}

yadsl_TesterRet yadsl_tester_release()
{
	if (argvp)
		yadsl_argvp_destroy(argvp);
	return YADSL_TESTER_RET_OK;
}
