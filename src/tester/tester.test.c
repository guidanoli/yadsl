#include <yadsl/posixstring.h>
#include <stddef.h>
#include <stdio.h>

#include <tester/tester.h>

#define matches(command, str) (strcmp(command, str) == 0)

char buffer[BUFSIZ];

const char *yadsl_tester_help_strings[] = {
	"/help                  Prints help strings, just like this one",
	"/print [s|i|f] <text>  Prints text formated as string, integer or float",
	"/throw                 Throws custom error",
	NULL,
};

yadsl_TesterRet yadsl_tester_init()
{
	puts("TesterInitCallback called");
	return YADSL_TESTER_RET_OK;
}

yadsl_TesterRet yadsl_tester_parse(const char *command)
{
	if matches(command, "help") {
		yadsl_tester_print_help_strings();
	} else if matches(command, "print") {
		if (yadsl_tester_parse_arguments("s", buffer) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		if (strlen(buffer) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		if matches(buffer, "s") {
			if (yadsl_tester_parse_arguments("s", buffer) != 1)
				return YADSL_TESTER_RET_ARGUMENT;
			yadsl_tester_log("String: '%s'", buffer);
		} else if matches(buffer, "i") {
			int i;
			if (yadsl_tester_parse_arguments("i", &i) != 1)
				return YADSL_TESTER_RET_ARGUMENT;
			yadsl_tester_log("Integer: '%d'", i);
		} else if matches(buffer, "l") {
			long l;
			if (yadsl_tester_parse_arguments("l", &l) != 1)
				return YADSL_TESTER_RET_RETURN;
			yadsl_tester_log("Long: '%ld'", l);
		} else if matches(buffer, "f") {
			float f;
			if (yadsl_tester_parse_arguments("f", &f) != 1)
				return YADSL_TESTER_RET_ARGUMENT;
			yadsl_tester_log("Float: '%f'", f);
		} else if matches(buffer, "z") {
			size_t z;
			if (yadsl_tester_parse_arguments("z", &z) != 1)
				return YADSL_TESTER_RET_ARGUMENT;
			yadsl_tester_log("Size Type: '%zu'", z);
		} else if matches(buffer, "c") {
			char c;
			if (yadsl_tester_parse_arguments("c", &c) != 1)
				return YADSL_TESTER_RET_ARGUMENT;
			yadsl_tester_log("Character: '%c'", c);
		} else {
			return YADSL_TESTER_RET_ARGUMENT;
		}
	} else if matches(command, "log") {
		if (yadsl_tester_parse_arguments("s", buffer) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		yadsl_tester_log(buffer);
	} else if matches(command, "throw") {
		if (yadsl_tester_parse_arguments("s", buffer) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		return yadsl_tester_return_external_value(buffer);
	} else {
		return YADSL_TESTER_RET_COMMAND;
	}
	return YADSL_TESTER_RET_OK;
}

yadsl_TesterRet yadsl_tester_release()
{
	puts("TesterExitCallback called");
	return YADSL_TESTER_RET_OK;
}
