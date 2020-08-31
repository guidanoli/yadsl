#include <diff/diff.h>

#include <yadsl/posixstring.h>
#include <stdio.h>

#include <tester/tester.h>

const char *yadsl_tester_help_strings[] = {
	"/diff X Y    saves the difference between X and Y in slot A",
	"/equ B       asserts last value computed is equal to B",
	"/neq B      asserts last value computed is different from B",
	"/gtr B      asserts last value computed is greater than B",
	"/lwr B      asserts last value computed is lower than B",
	"/geq B      asserts last value computed is greater or equal to B",
	"/leq B      asserts last value computed is lower or equal to B",
	NULL,
};

#define matches(a, b) (strcmp(a, b) == 0)

char X[BUFSIZ], Y[BUFSIZ];
double val, eax = -1.0; /* Last computed value */

yadsl_TesterRet yadsl_tester_init()
{
	return YADSL_TESTER_RET_OK;
}

yadsl_TesterRet yadsl_tester_parse(const char *command)
{
	if matches(command, "diff"){
		if (yadsl_tester_parse_arguments("ss", X, Y) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		eax = yadsl_utils_diff(X, Y);
		if (eax == -1.0)
			return YADSL_TESTER_RET_MALLOC;
		yadsl_tester_log("diff = %lf", eax);
	} else if matches(command, "equ") {
		if (yadsl_tester_parse_arguments("f", &val) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		if (!(eax == val))
			return YADSL_TESTER_RET_RETURN;
	} else if matches(command, "neq") {
		if (yadsl_tester_parse_arguments("f", &val) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		if (!(eax != val))
			return YADSL_TESTER_RET_RETURN;
	} else if matches(command, "gtr") {
		if (yadsl_tester_parse_arguments("f", &val) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		if (!(eax > val))
			return YADSL_TESTER_RET_RETURN;
	} else if matches(command, "lwr") {
		if (yadsl_tester_parse_arguments("f", &val) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		if (!(eax < val))
			return YADSL_TESTER_RET_RETURN;
	} else if matches(command, "geq") {
		if (yadsl_tester_parse_arguments("f", &val) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		if (!(eax >= val))
			return YADSL_TESTER_RET_RETURN;
	} else if matches(command, "leq") {
		if (yadsl_tester_parse_arguments("f", &val) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		if (!(eax <= val))
			return YADSL_TESTER_RET_RETURN;
	} else {
		return YADSL_TESTER_RET_COMMAND;
	}
	return YADSL_TESTER_RET_OK;
}

yadsl_TesterRet yadsl_tester_release()
{
	return YADSL_TESTER_RET_OK;
}
